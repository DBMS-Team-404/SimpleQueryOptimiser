#ifndef COST_BASED_OPTIMIZER_HPP
#define COST_BASED_OPTIMIZER_HPP

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <limits>
#include <cmath>
#include <bitset>
#include "ast_nodes.hpp"
#include "catalog.hpp"
#include "cost_model.hpp"

static constexpr double PRUNE_BUDGET    = 10'000'000.0;
static constexpr double JOIN_SELECTIVITY = 0.1;

// =============================================================
// DP TABLE ENTRY
// =============================================================
struct DPEntry {
    double                   cost;
    TableStats               out_stats;
    std::unique_ptr<PlanNode> plan;

    DPEntry() : cost(std::numeric_limits<double>::infinity()), out_stats({0,0}) {}
    DPEntry(double c, TableStats s, std::unique_ptr<PlanNode> p)
        : cost(c), out_stats(s), plan(std::move(p)) {}
    DPEntry(DPEntry&&) = default;
    DPEntry& operator=(DPEntry&&) = default;
};

// =============================================================
// JOIN EDGE
// =============================================================
struct JoinEdge {
    int                         left_idx;
    int                         right_idx;
    JoinType                    join_type;
    std::shared_ptr<Expression> condition;
};

// =============================================================
// COST-BASED OPTIMIZER
// =============================================================
class CostBasedOptimizer {
private:
    ICatalog& catalog;
    std::unordered_map<uint32_t, DPEntry> memo;

    // -------------------------------------------------------
    // TableInfo: one entry per leaf table discovered.
    //
    // base_plan = raw pointer to the node that forms the base
    // of this table in the original tree. Two cases:
    //   (a) LOGICAL_GET          — plain scan, no filter
    //   (b) LOGICAL_FILTER       — RBO pushed a filter here;
    //       its child[0] is the GET
    //
    // We store a raw pointer because the original tree owns
    // the memory. clonePlan() copies it before putting it in
    // the memo table so the DP owns independent copies.
    // -------------------------------------------------------
    struct TableInfo {
        std::string table_name;
        std::string alias;
        TableStats  stats;
        PlanNode*   base_plan;  // GET or FILTER->GET
    };
    std::vector<TableInfo> tables;
    std::vector<JoinEdge>  edges;

    // -------------------------------------------------------
    // STEP 1 — walk the logical tree, collect tables and edges
    // -------------------------------------------------------
    void extractJoinTree(PlanNode* node) {
        if (!node) return;

        // Case (a): plain table scan
        if (node->type == NodeType::LOGICAL_GET) {
            auto* get = static_cast<LogicalGetNode*>(node);
            TableStats stats = catalog.getTableStats(get->table_name);
            tables.push_back({get->table_name, get->alias, stats, node});
            return;
        }

        // Case (b): filter pushed directly onto a GET by the RBO
        if (node->type == NodeType::LOGICAL_FILTER &&
            !node->children.empty() &&
            node->children[0]->type == NodeType::LOGICAL_GET)
        {
            auto* get = static_cast<LogicalGetNode*>(node->children[0].get());
            TableStats stats = catalog.getTableStats(get->table_name);
            // base_plan is the FILTER node (owns the GET as its child)
            tables.push_back({get->table_name, get->alias, stats, node});
            return;
        }

        // JOIN: record the edge between the two subtrees, then recurse
        if (node->type == NodeType::LOGICAL_JOIN) {
            auto* join = static_cast<LogicalJoinNode*>(node);

            int left_start = (int)tables.size();
            extractJoinTree(node->children[0].get());
            int left_end   = (int)tables.size();

            int right_start = (int)tables.size();
            extractJoinTree(node->children[1].get());

            edges.push_back({
                left_end   - 1,   // rightmost table on the left side
                right_start,      // leftmost table on the right side
                join->join_type,
                // non-owning shared_ptr; cloned when building physical nodes
                std::shared_ptr<Expression>(join->condition.get(), [](Expression*){})
            });
            (void)left_start;
            return;
        }

        // PROJECT / AGGREGATE / SORT / LIMIT above the join tree — recurse
        for (auto& child : node->children)
            extractJoinTree(child.get());
    }

    // -------------------------------------------------------
    // STEP 2 — deep-copy an Expression tree
    // -------------------------------------------------------
    static std::unique_ptr<Expression> cloneExpr(const Expression* e) {
        if (!e) return nullptr;
        if (e->type == ExpressionType::COLUMN || e->type == ExpressionType::CONSTANT)
            return std::make_unique<Expression>(e->type, e->value);
        return std::make_unique<Expression>(
            e->type, e->op,
            cloneExpr(e->left.get()),
            cloneExpr(e->right.get())
        );
    }

    // -------------------------------------------------------
    // STEP 3 — deep-clone a PlanNode subtree
    //
    // BUG THAT WAS HERE: the LOGICAL_FILTER case was declaring
    // its own local `copy` (shadowing the outer one declared at
    // the top of the function), building the filter into it,
    // then hitting `break` — the outer `copy` was still null,
    // so the child loop below wrote children into null, and the
    // local copy was immediately destroyed. Result: filter gone.
    //
    // FIX: every case now assigns into the single outer `copy`,
    // and LOGICAL_GET returns early (no children to clone).
    // -------------------------------------------------------
    std::unique_ptr<PlanNode> clonePlan(const PlanNode* node) {
        if (!node) return nullptr;

        std::unique_ptr<PlanNode> copy;   // ONE copy variable for all cases

        switch (node->type) {

            case NodeType::LOGICAL_GET: {
                auto* n = static_cast<const LogicalGetNode*>(node);
                // GET has no children — return directly, skip child loop
                return std::make_unique<LogicalGetNode>(n->table_name, n->alias);
            }

            case NodeType::LOGICAL_FILTER: {
                auto* n = static_cast<const LogicalFilterNode*>(node);
                copy = std::make_unique<LogicalFilterNode>(cloneExpr(n->predicate.get()));
                break;  // fall through to child loop
            }

            case NodeType::PHYSICAL_HASH_JOIN: {
                auto* n = static_cast<const PhysicalHashJoinNode*>(node);
                copy = std::make_unique<PhysicalHashJoinNode>(
                    n->join_type, cloneExpr(n->condition.get()));
                break;
            }

            case NodeType::PHYSICAL_NESTED_LOOP_JOIN: {
                auto* n = static_cast<const PhysicalNestedLoopJoinNode*>(node);
                copy = std::make_unique<PhysicalNestedLoopJoinNode>(
                    n->join_type, cloneExpr(n->condition.get()));
                break;
            }

            default:
                std::cerr << "[CBO] clonePlan: unexpected node type\n";
                return nullptr;
        }

        // Clone all children into the copy
        for (const auto& child : node->children)
            copy->children.push_back(clonePlan(child.get()));

        return copy;
    }

    // -------------------------------------------------------
    // STEP 4 — cardinality estimator for join output
    // -------------------------------------------------------
    static TableStats estimateJoinOutput(const TableStats& left, const TableStats& right) {
        double out_tuples = std::max(1.0,
            (double)left.num_tuples * right.num_tuples * JOIN_SELECTIVITY);
        double density = std::max(
            (double)left.num_tuples  / std::max(left.num_blocks,  (size_t)1),
            (double)right.num_tuples / std::max(right.num_blocks, (size_t)1)
        );
        size_t out_blocks = (size_t)std::ceil(out_tuples / density);
        return { (size_t)out_tuples, std::max(out_blocks, (size_t)1) };
    }

    // -------------------------------------------------------
    // STEP 5 — pick Hash Join vs NLJ, build the physical node
    // -------------------------------------------------------
    struct JoinDecision {
        double                    cost;
        std::unique_ptr<PlanNode> node;
    };

    JoinDecision chooseBestJoin(
        JoinType jt,
        const Expression* cond,
        std::unique_ptr<PlanNode> left_plan,
        std::unique_ptr<PlanNode> right_plan,
        const TableStats& left_stats,
        const TableStats& right_stats)
    {
        double hash_cost = CostModel::costHashJoin(left_stats, right_stats);
        double nlj_cost  = CostModel::costNestedLoopJoin(left_stats, right_stats);

        std::unique_ptr<PlanNode> physical;
        double chosen_cost;

        if (hash_cost <= nlj_cost) {
            physical    = std::make_unique<PhysicalHashJoinNode>(jt, cloneExpr(cond));
            chosen_cost = hash_cost;
        } else {
            physical    = std::make_unique<PhysicalNestedLoopJoinNode>(jt, cloneExpr(cond));
            chosen_cost = nlj_cost;
        }

        physical->children.push_back(std::move(left_plan));
        physical->children.push_back(std::move(right_plan));
        return { chosen_cost, std::move(physical) };
    }

    // -------------------------------------------------------
    // STEP 6 — find the join edge connecting two subsets
    // -------------------------------------------------------
    const JoinEdge* findEdge(uint32_t left_mask, uint32_t right_mask) const {
        for (const auto& e : edges) {
            bool l_in_left  = (left_mask  >> e.left_idx)  & 1;
            bool r_in_right = (right_mask >> e.right_idx) & 1;
            bool l_in_right = (right_mask >> e.left_idx)  & 1;
            bool r_in_left  = (left_mask  >> e.right_idx) & 1;
            if ((l_in_left && r_in_right) || (l_in_right && r_in_left))
                return &e;
        }
        return nullptr;
    }

    // -------------------------------------------------------
    // STEP 7 — DP enumerator (left-deep + bushy, with pruning)
    //
    // BUG THAT WAS HERE: the base case always built a fresh
    // LogicalGetNode, ignoring any FILTER the RBO had placed
    // on the leaf. Even if extractJoinTree correctly recorded
    // the FILTER as base_plan, the DP threw it away here.
    //
    // FIX: clone tables[i].base_plan instead of making a GET.
    // -------------------------------------------------------
    void runDP() {
        int N = (int)tables.size();
        if (N == 0) return;

        // Base case: each table alone
        for (int i = 0; i < N; i++) {
            uint32_t mask      = 1u << i;
            double   scan_cost = CostModel::costSequentialScan(tables[i].stats);
            // Clone the full leaf plan — preserves any pushed-down filter
            auto base_clone = clonePlan(tables[i].base_plan);
            memo[mask] = DPEntry(scan_cost, tables[i].stats, std::move(base_clone));
        }

        int full_mask = (1 << N) - 1;

        for (int size = 2; size <= N; size++) {
            uint32_t subset = (1u << size) - 1;
            while ((int)subset <= full_mask) {

                for (uint32_t left_mask = (subset - 1) & subset;
                     left_mask > 0;
                     left_mask = (left_mask - 1) & subset)
                {
                    uint32_t right_mask = subset ^ left_mask;
                    if (!memo.count(left_mask) || !memo.count(right_mask)) continue;

                    DPEntry& left_entry  = memo[left_mask];
                    DPEntry& right_entry = memo[right_mask];

                    double base_cost = left_entry.cost + right_entry.cost;
                    if (base_cost >= PRUNE_BUDGET) {
                        std::cout << "[DP] Pruning subset 0b" << std::bitset<8>(subset)
                                  << " — base cost " << base_cost
                                  << " exceeds budget " << PRUNE_BUDGET << "\n";
                        continue;
                    }

                    const JoinEdge*   edge = findEdge(left_mask, right_mask);
                    JoinType          jt   = edge ? edge->join_type : JoinType::CROSS;
                    const Expression* cond = edge ? edge->condition.get() : nullptr;

                    TableStats out_stats = estimateJoinOutput(
                        left_entry.out_stats, right_entry.out_stats);

                    auto left_clone  = clonePlan(left_entry.plan.get());
                    auto right_clone = clonePlan(right_entry.plan.get());
                    if (!left_clone || !right_clone) continue;

                    auto decision = chooseBestJoin(
                        jt, cond,
                        std::move(left_clone), std::move(right_clone),
                        left_entry.out_stats,  right_entry.out_stats
                    );

                    double total_cost = base_cost + decision.cost;

                    if (!memo.count(subset) || total_cost < memo[subset].cost) {
                        memo[subset] = DPEntry(total_cost, out_stats,
                                               std::move(decision.node));
                        std::cout << "[DP] Subset 0b" << std::bitset<8>(subset)
                                  << " new best cost: " << total_cost << "\n";
                    }
                }

                // Gosper's hack — next subset with same popcount
                uint32_t c = subset & -subset;
                uint32_t r = subset + c;
                subset = (((r ^ subset) >> 2) / c) | r;
            }
        }
    }

    // -------------------------------------------------------
    // STEP 8 — collect wrapper nodes above the join subtree
    //
    // Stops at JOIN or GET nodes (those are the join tree).
    // A FILTER directly on a GET is a leaf filter — already
    // baked into base_plan, so we skip it here too.
    // -------------------------------------------------------
    void collectWrappers(PlanNode* node, std::vector<PlanNode*>& out) {
        if (!node) return;

        if (node->type == NodeType::LOGICAL_JOIN ||
            node->type == NodeType::LOGICAL_GET)
            return;

        if (node->type == NodeType::LOGICAL_FILTER &&
            !node->children.empty() &&
            node->children[0]->type == NodeType::LOGICAL_GET)
            return;  // leaf filter — handled via base_plan, not a wrapper

        out.push_back(node);
        if (!node->children.empty())
            collectWrappers(node->children[0].get(), out);
    }

    // -------------------------------------------------------
    // STEP 9 — re-wrap the optimized join core with
    // PROJECT / AGGREGATE / SORT / LIMIT (applied inside-out)
    // -------------------------------------------------------
    std::unique_ptr<PlanNode> wrapNonJoinNodes(
        std::unique_ptr<PlanNode> join_core,
        std::vector<PlanNode*>&   wrappers)
    {
        for (int i = (int)wrappers.size() - 1; i >= 0; i--) {
            PlanNode* w = wrappers[i];
            std::unique_ptr<PlanNode> new_node;

            if (w->type == NodeType::LOGICAL_FILTER) {
                auto* f = static_cast<LogicalFilterNode*>(w);
                auto wrapper = std::make_unique<LogicalFilterNode>(
                    cloneExpr(f->predicate.get()));
                wrapper->children.push_back(std::move(join_core));
                new_node = std::move(wrapper);
            }
            else if (w->type == NodeType::LOGICAL_PROJECT) {
                auto* p = static_cast<LogicalProjectNode*>(w);
                auto wrapper = std::make_unique<LogicalProjectNode>(p->columns);
                wrapper->children.push_back(std::move(join_core));
                new_node = std::move(wrapper);
            }
            else if (w->type == NodeType::LOGICAL_AGGREGATE) {
                auto* a = static_cast<LogicalAggregateNode*>(w);
                auto wrapper = std::make_unique<LogicalAggregateNode>(
                    a->group_by_columns, a->aggregates);
                wrapper->children.push_back(std::move(join_core));
                new_node = std::move(wrapper);
            }
            else if (w->type == NodeType::LOGICAL_SORT) {
                auto* s = static_cast<LogicalSortNode*>(w);
                auto wrapper = std::make_unique<LogicalSortNode>(s->sort_columns);
                wrapper->children.push_back(std::move(join_core));
                new_node = std::move(wrapper);
            }
            else if (w->type == NodeType::LOGICAL_LIMIT) {
                auto* l = static_cast<LogicalLimitNode*>(w);
                auto wrapper = std::make_unique<LogicalLimitNode>(l->limit_count);
                wrapper->children.push_back(std::move(join_core));
                new_node = std::move(wrapper);
            }
            else {
                new_node = std::move(join_core);
            }

            join_core = std::move(new_node);
        }
        return join_core;
    }

public:
    CostBasedOptimizer(ICatalog& cat) : catalog(cat) {}

    std::unique_ptr<PlanNode> optimize(std::unique_ptr<PlanNode> root) {
        std::cout << "\n--- STARTING COST-BASED OPTIMIZATION (DP JOIN ENUMERATION) ---\n";

        std::vector<PlanNode*> wrappers;
        collectWrappers(root.get(), wrappers);

        extractJoinTree(root.get());

        int N = (int)tables.size();
        std::cout << "[DP] Found " << N << " table(s) to enumerate.\n";

        if (N == 0) {
            std::cout << "[DP] No tables found — returning tree unchanged.\n";
            return root;
        }

        if (N == 1) {
            std::cout << "[DP] Single table query — no join enumeration needed.\n";
            auto leaf = clonePlan(tables[0].base_plan);
            return wrapNonJoinNodes(std::move(leaf), wrappers);
        }

        runDP();

        uint32_t full_mask = (1u << N) - 1;
        if (!memo.count(full_mask)) {
            std::cerr << "[DP] ERROR: No plan found for full table set!\n";
            return root;
        }

        DPEntry& best = memo[full_mask];
        std::cout << "\n[DP] Best join plan cost: " << best.cost << "\n";

        return wrapNonJoinNodes(std::move(best.plan), wrappers);
    }
};

#endif // COST_BASED_OPTIMIZER_HPP