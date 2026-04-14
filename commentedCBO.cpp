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
#include "ast_nodes.hpp"
#include "catalog.hpp"
#include "cost_model.hpp"

// =============================================================
// PRUNING BUDGET
// If a partial plan's cost exceeds this, we stop exploring it.
// =============================================================
static constexpr double PRUNE_BUDGET = 10'000'000.0;

// =============================================================
// JOIN SELECTIVITY
// We don't have column statistics, so we assume a join reduces
// the output to (left * right) / max(left, right) tuples.
// This is a standard "10% selectivity" heuristic used by
// textbook optimizers when no histogram data is available.
// =============================================================
static constexpr double JOIN_SELECTIVITY = 0.1;

// =============================================================
// INTERNAL STRUCTURES FOR THE DP TABLE
// =============================================================

// Represents the best plan found so far for a given subset of tables.
struct DPEntry {
    double              cost;         // Total accumulated cost for this sub-plan
    TableStats          out_stats;    // Estimated output cardinality (feeds parent joins)
    std::unique_ptr<PlanNode> plan;   // The actual physical plan tree

    DPEntry() : cost(std::numeric_limits<double>::infinity()), out_stats({0,0}) {}

    DPEntry(double c, TableStats s, std::unique_ptr<PlanNode> p)
        : cost(c), out_stats(s), plan(std::move(p)) {}

    // Move-only (plan is a unique_ptr)
    DPEntry(DPEntry&&) = default;
    DPEntry& operator=(DPEntry&&) = default;
};

// Holds one extracted join edge from the original logical tree
struct JoinEdge {
    int                            left_idx;   // index into the tables[] vector
    int                            right_idx;
    JoinType                       join_type;
    std::shared_ptr<Expression>    condition;  // shared so we can clone into multiple candidate plans
};

// =============================================================
// COST-BASED OPTIMIZER
// =============================================================
class CostBasedOptimizer {
private:
    ICatalog& catalog;

    // DP memo table: bitmask -> best DPEntry for that subset of tables
    // We use uint32_t so we can handle up to 32 tables (plenty for this project)
    std::unordered_map<uint32_t, DPEntry> memo;

    // All leaf tables extracted from the logical join tree, in discovery order
    struct TableInfo {
        std::string           table_name;
        std::string           alias;
        TableStats            stats;
    };
    std::vector<TableInfo> tables;

    // All join edges found in the logical join tree
    std::vector<JoinEdge> edges;

    // -------------------------------------------------------
    // STEP 1: Extract all leaves and edges from the logical tree
    // -------------------------------------------------------
    void extractJoinTree(PlanNode* node) {
        if (!node) return;

        if (node->type == NodeType::LOGICAL_GET) {
            auto* get = static_cast<LogicalGetNode*>(node);
            TableStats stats = catalog.getTableStats(get->table_name);
            tables.push_back({get->table_name, get->alias, stats});
            return;
        }

        if (node->type == NodeType::LOGICAL_JOIN) {
            auto* join = static_cast<LogicalJoinNode*>(node);

            // Record position before recursing left, so we know left_idx
            int left_start = (int)tables.size();
            extractJoinTree(node->children[0].get());
            int left_end = (int)tables.size(); // tables[left_start..left_end) are the left subtree

            int right_start = (int)tables.size();
            extractJoinTree(node->children[1].get());
            int right_end = (int)tables.size();

            // For simple 2-table joins this is straightforward.
            // For multi-way joins extracted from a chain, we record the
            // direct children indices (the outermost leaf on each side).
            // The DP will figure out all orderings regardless.
            // We store the indices of the leftmost leaf on each side as
            // a representative — the DP enumerator explores all orderings anyway.
            edges.push_back({
                left_end  - 1,   // rightmost table added from left branch
                right_start,     // leftmost table added from right branch
                join->join_type,
                std::shared_ptr<Expression>(join->condition.get(), [](Expression*){})
                // ^^^ non-owning shared_ptr: the logical tree still owns the condition.
                // We will deep-copy it when building physical nodes below.
            });
            (void)right_end; // suppress unused warning
            return;
        }

        // For FILTER, PROJECT, AGGREGATE etc. — just recurse into children
        if (node->type == NodeType::LOGICAL_FILTER) {
            // Check if the child is a GET — if so, register this filter+get as the leaf
            if (!node->children.empty() && 
                node->children[0]->type == NodeType::LOGICAL_GET) {
                auto* get = static_cast<LogicalGetNode*>(node->children[0].get());
                TableStats stats = catalog.getTableStats(get->table_name);
                tables.push_back({get->table_name, get->alias, stats});
                // Store the full filter node as the base plan for this table
                // (not just the GET)
                leaf_plans.push_back(node); // need a parallel vector for this
                return;
            }
            // Otherwise recurse normally
            for (auto& child : node->children)
                extractJoinTree(child.get());
        }
    }

    // -------------------------------------------------------
    // STEP 2: Deep-copy an Expression tree
    // (needed because multiple DP candidates may want the same condition)
    // -------------------------------------------------------
    static std::unique_ptr<Expression> cloneExpr(const Expression* e) {
        if (!e) return nullptr;
        if (e->type == ExpressionType::COLUMN || e->type == ExpressionType::CONSTANT) {
            return std::make_unique<Expression>(e->type, e->value);
        }
        return std::make_unique<Expression>(
            e->type, e->op,
            cloneExpr(e->left.get()),
            cloneExpr(e->right.get())
        );
    }

    // -------------------------------------------------------
    // STEP 3: Cardinality estimator for join output
    // Uses simple selectivity heuristic.
    // -------------------------------------------------------
    static TableStats estimateJoinOutput(const TableStats& left, const TableStats& right) {
        double out_tuples = std::max(1.0,
            (double)left.num_tuples * right.num_tuples * JOIN_SELECTIVITY);
        // Estimate blocks: assume same density as the larger input
        double density = std::max(
            (double)left.num_tuples  / std::max(left.num_blocks,  (size_t)1),
            (double)right.num_tuples / std::max(right.num_blocks, (size_t)1)
        );
        size_t out_blocks = (size_t)std::ceil(out_tuples / density);
        return { (size_t)out_tuples, std::max(out_blocks, (size_t)1) };
    }

    // -------------------------------------------------------
    // STEP 4: Pick the cheaper physical join algorithm,
    // build the node, and return the cost.
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
            physical = std::make_unique<PhysicalHashJoinNode>(jt, cloneExpr(cond));
            chosen_cost = hash_cost;
        } else {
            physical = std::make_unique<PhysicalNestedLoopJoinNode>(jt, cloneExpr(cond));
            chosen_cost = nlj_cost;
        }

        physical->children.push_back(std::move(left_plan));
        physical->children.push_back(std::move(right_plan));

        return { chosen_cost, std::move(physical) };
    }

    // -------------------------------------------------------
    // STEP 5: Find the join edge connecting two subsets, if any.
    // Returns nullptr condition if no direct edge (cross join fallback).
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
        return nullptr; // No direct edge — will produce a cross join
    }

    // -------------------------------------------------------
    // STEP 6: Deep-clone a PlanNode tree
    // (needed when a sub-plan is reused across multiple candidates)
    // -------------------------------------------------------
    std::unique_ptr<PlanNode> clonePlan(const PlanNode* node) {
        if (!node) return nullptr;

        std::unique_ptr<PlanNode> copy;

        switch (node->type) {
            case NodeType::LOGICAL_GET: {
                auto* n = static_cast<const LogicalGetNode*>(node);
                copy = std::make_unique<LogicalGetNode>(n->table_name, n->alias);
                break;
            }
            case NodeType::PHYSICAL_HASH_JOIN: {
                auto* n = static_cast<const PhysicalHashJoinNode*>(node);
                copy = std::make_unique<PhysicalHashJoinNode>(n->join_type, cloneExpr(n->condition.get()));
                break;
            }
            case NodeType::PHYSICAL_NESTED_LOOP_JOIN: {
                auto* n = static_cast<const PhysicalNestedLoopJoinNode*>(node);
                copy = std::make_unique<PhysicalNestedLoopJoinNode>(n->join_type, cloneExpr(n->condition.get()));
                break;
            }
            case NodeType::LOGICAL_FILTER: {
                auto* n = static_cast<const LogicalFilterNode*>(node);
                auto copy = std::make_unique<LogicalFilterNode>(cloneExpr(n->predicate.get()));
                for (const auto& child : node->children)
                    copy->children.push_back(clonePlan(child.get()));
                break;
            }
            default:
                // For other node types (FILTER etc.) a shallow recreation is fine
                // since we only clone join sub-trees here
                return nullptr;
        }

        for (const auto& child : node->children)
            copy->children.push_back(clonePlan(child.get()));

        return copy;
    }

    // -------------------------------------------------------
    // STEP 7: THE DP ENUMERATOR
    //
    // Explores BOTH left-deep and bushy trees.
    //
    // Left-deep:  ((A ⋈ B) ⋈ C) ⋈ D  — only single tables on the right
    // Bushy:      (A ⋈ B) ⋈ (C ⋈ D)  — both sides can be sub-joins
    //
    // For N tables we have 2^N subsets (bitmasks).
    // For each subset S we try every way to split it into (left | right).
    // We memoize the best plan for each subset.
    // -------------------------------------------------------
    void runDP() {
        int N = (int)tables.size();
        if (N == 0) return;

        // --- Base case: single tables ---
        for (int i = 0; i < N; i++) {
            uint32_t mask = 1u << i;
            double scan_cost = CostModel::costSequentialScan(tables[i].stats);
            auto get_node = std::make_unique<LogicalGetNode>(
                tables[i].table_name, tables[i].alias);
            memo[mask] = DPEntry(scan_cost, tables[i].stats, std::move(get_node));
        }

        // --- Fill subsets of increasing size ---
        // Enumerate all non-empty subsets of {0..N-1}
        int full_mask = (1 << N) - 1;

        for (int size = 2; size <= N; size++) {
            // Generate all subsets of `size` bits set in [0, full_mask]
            // Using Gosper's hack to iterate subsets of a given popcount
            uint32_t subset = (1u << size) - 1; // smallest subset with `size` bits
            while ((int)subset <= full_mask) {

                // Try every way to partition `subset` into two non-empty parts
                // Enumerate all proper non-empty subsets of `subset`
                for (uint32_t left_mask = (subset - 1) & subset;
                     left_mask > 0;
                     left_mask = (left_mask - 1) & subset)
                {
                    uint32_t right_mask = subset ^ left_mask;

                    // Skip if either side hasn't been solved yet
                    if (!memo.count(left_mask) || !memo.count(right_mask)) continue;

                    DPEntry& left_entry  = memo[left_mask];
                    DPEntry& right_entry = memo[right_mask];

                    // --- PRUNING: kill branch early if already too expensive ---
                    double base_cost = left_entry.cost + right_entry.cost;
                    if (base_cost >= PRUNE_BUDGET) {
                        std::cout << "[DP] Pruning subset 0b" << std::bitset<8>(subset)
                                  << " — base cost " << base_cost
                                  << " exceeds budget " << PRUNE_BUDGET << "\n";
                        continue;
                    }

                    // Find the join condition linking left and right
                    const JoinEdge* edge = findEdge(left_mask, right_mask);
                    JoinType jt = edge ? edge->join_type : JoinType::CROSS;
                    const Expression* cond = edge ? edge->condition.get() : nullptr;

                    // Estimate output cardinality
                    TableStats out_stats = estimateJoinOutput(
                        left_entry.out_stats, right_entry.out_stats);

                    // Build candidate join node and compute cost
                    // We need clones of the child plans because the same sub-plan
                    // may be the left child of multiple candidates
                    auto left_clone  = clonePlan(left_entry.plan.get());
                    auto right_clone = clonePlan(right_entry.plan.get());

                    if (!left_clone || !right_clone) continue;

                    auto decision = chooseBestJoin(
                        jt, cond,
                        std::move(left_clone),
                        std::move(right_clone),
                        left_entry.out_stats,
                        right_entry.out_stats
                    );

                    double total_cost = base_cost + decision.cost;

                    // Update memo if this is the best plan for this subset
                    if (!memo.count(subset) || total_cost < memo[subset].cost) {
                        memo[subset] = DPEntry(total_cost, out_stats, std::move(decision.node));

                        std::cout << "[DP] Subset 0b" << std::bitset<8>(subset)
                                  << " new best cost: " << total_cost << "\n";
                    }
                }

                // Gosper's hack: next subset with same popcount
                uint32_t c = subset & -subset;
                uint32_t r = subset + c;
                subset = (((r ^ subset) >> 2) / c) | r;
            }
        }
    }

    // -------------------------------------------------------
    // STEP 8: Wrap non-join nodes (FILTER, PROJECT, AGGREGATE)
    // back around the optimized join core.
    // -------------------------------------------------------
    std::unique_ptr<PlanNode> wrapNonJoinNodes(
        std::unique_ptr<PlanNode> join_core,
        std::vector<PlanNode*>& wrappers)   // collected in top-down order
    {
        // Wrappers were collected top-down, so we apply them inside-out
        // (last collected = innermost wrapper, closest to the join core)
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
                // Unknown node type — skip wrapping
                new_node = std::move(join_core);
            }

            join_core = std::move(new_node);
        }
        return join_core;
    }

    // Collect all non-join wrapper nodes sitting above the join tree (top-down)
    void collectWrappers(PlanNode* node, std::vector<PlanNode*>& out) {
        if (!node) return;
        bool is_join_or_get = (node->type == NodeType::LOGICAL_JOIN ||
                               node->type == NodeType::LOGICAL_GET);
        if (!is_join_or_get) {
            out.push_back(node);
            if (!node->children.empty())
                collectWrappers(node->children[0].get(), out);
        }
    }

public:
    CostBasedOptimizer(ICatalog& cat) : catalog(cat) {}

    std::unique_ptr<PlanNode> optimize(std::unique_ptr<PlanNode> root) {
        std::cout << "\n--- STARTING COST-BASED OPTIMIZATION (DP JOIN ENUMERATION) ---\n";

        // 1. Collect wrapper nodes sitting above the join subtree
        std::vector<PlanNode*> wrappers;
        collectWrappers(root.get(), wrappers);

        // 2. Extract leaf tables and join edges from the full tree
        extractJoinTree(root.get());

        int N = (int)tables.size();
        std::cout << "[DP] Found " << N << " table(s) to enumerate.\n";

        if (N == 0) {
            std::cout << "[DP] No tables found — returning tree unchanged.\n";
            return root;
        }

        if (N == 1) {
            // Nothing to join — just do a scan
            std::cout << "[DP] Single table query — no join enumeration needed.\n";
            auto get = std::make_unique<LogicalGetNode>(
                tables[0].table_name, tables[0].alias);
            return wrapNonJoinNodes(std::move(get), wrappers);
        }

        // 3. Run the DP enumerator
        runDP();

        // 4. Extract the best full plan from the memo table
        uint32_t full_mask = (1u << N) - 1;

        if (!memo.count(full_mask)) {
            std::cerr << "[DP] ERROR: No plan found for full table set! "
                      << "Check that all join conditions reference valid tables.\n";
            return root; // return original unchanged
        }

        DPEntry& best = memo[full_mask];
        std::cout << "\n[DP] Best join plan cost: " << best.cost << "\n";

        // 5. Re-wrap with PROJECT / FILTER / AGGREGATE etc.
        auto final_plan = wrapNonJoinNodes(std::move(best.plan), wrappers);

        return final_plan;
    }
};

// needed for the bitset printing in the DP loop
#include <bitset>

#endif // COST_BASED_OPTIMIZER_HPP