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

struct JoinEdge {
    int                         left_idx;
    int                         right_idx;
    JoinType                    join_type;
    std::shared_ptr<Expression> condition;
};

struct CrossFilter {
    uint32_t required_mask;
    std::shared_ptr<Expression> condition;
};

class CostBasedOptimizer {
private:
    ICatalog& catalog;
    std::unordered_map<uint32_t, DPEntry> memo;
    double last_best_cost = 0.0;

    struct TableInfo {
        std::string table_name;
        std::string alias;
        TableStats  stats;
        PlanNode* base_plan;  
    };
    std::vector<TableInfo> tables;
    std::vector<JoinEdge>  edges;
    
    std::vector<PlanNode*>   pending_cross_filters;
    std::vector<CrossFilter> cross_filters;

    // Finds all aliases in an expression so we can map the filter to a bitmask
    void getReferencedAliases(Expression* expr, std::vector<std::string>& aliases) {
        if (!expr) return;
        if (expr->type == ExpressionType::COLUMN) {
            std::string col = expr->value;
            size_t dot_pos = col.find('.');
            if (dot_pos != std::string::npos) {
                std::string alias = col.substr(0, dot_pos);
                if (std::find(aliases.begin(), aliases.end(), alias) == aliases.end()) {
                    aliases.push_back(alias);
                }
            }
        }
        getReferencedAliases(expr->left.get(), aliases);
        getReferencedAliases(expr->right.get(), aliases);
    }

    // Maps an alias string to the integer index used in the DP bitmask
    int getTableIndex(const std::string& alias) {
        for (int i = 0; i < (int)tables.size(); ++i) {
            std::string current = tables[i].alias.empty() ? tables[i].table_name : tables[i].alias;
            if (current == alias) return i;
        }
        return -1;
    }

    // Converts the safely stashed filters into DP-ready bitmasks
    void resolveCrossFilters() {
        for (PlanNode* node : pending_cross_filters) {
            auto* filter = static_cast<LogicalFilterNode*>(node);
            std::vector<std::string> aliases;
            getReferencedAliases(filter->predicate.get(), aliases);
            
            uint32_t mask = 0;
            for (const auto& alias : aliases) {
                int idx = getTableIndex(alias);
                if (idx != -1) mask |= (1u << idx);
            }
            if (mask != 0) {
                cross_filters.push_back({mask, std::shared_ptr<Expression>(filter->predicate.get(), [](Expression*){})});
            }
        }
    }

    void extractJoinTree(PlanNode* node) {
        if (!node) return;

        if (node->type == NodeType::LOGICAL_GET) {
            auto* get = static_cast<LogicalGetNode*>(node);
            TableStats stats = catalog.getTableStats(get->table_name);
            tables.push_back({get->table_name, get->alias, stats, node});
            return;
        }

        if (node->type == NodeType::LOGICAL_FILTER) {
            // Leaf filter: attach to the table
            if (!node->children.empty() && node->children[0]->type == NodeType::LOGICAL_GET) {
                auto* get = static_cast<LogicalGetNode*>(node->children[0].get());
                TableStats stats = catalog.getTableStats(get->table_name);
                tables.push_back({get->table_name, get->alias, stats, node});
                return;
            } 
            // Intermediate filter (LCA): Stash it safely!
            else {
                pending_cross_filters.push_back(node);
                extractJoinTree(node->children[0].get());
                return;
            }
        }

        if (node->type == NodeType::LOGICAL_JOIN) {
            auto* join = static_cast<LogicalJoinNode*>(node);

            int left_start = (int)tables.size();
            extractJoinTree(node->children[0].get());
            int left_end   = (int)tables.size();

            int right_start = (int)tables.size();
            extractJoinTree(node->children[1].get());

            edges.push_back({
                left_end   - 1,   
                right_start,      
                join->join_type,
                std::shared_ptr<Expression>(join->condition.get(), [](Expression*){})
            });
            return;
        }

        for (auto& child : node->children)
            extractJoinTree(child.get());
    }

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

    std::unique_ptr<PlanNode> clonePlan(const PlanNode* node) {
        if (!node) return nullptr;

        std::unique_ptr<PlanNode> copy;   

        switch (node->type) {
            case NodeType::LOGICAL_GET: {
                auto* n = static_cast<const LogicalGetNode*>(node);
                return std::make_unique<LogicalGetNode>(n->table_name, n->alias);
            }
            case NodeType::LOGICAL_FILTER: {
                auto* n = static_cast<const LogicalFilterNode*>(node);
                copy = std::make_unique<LogicalFilterNode>(cloneExpr(n->predicate.get()));
                break;  
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

        for (const auto& child : node->children)
            copy->children.push_back(clonePlan(child.get()));

        return copy;
    }

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

    void runDP() {
        int N = (int)tables.size();
        if (N == 0) return;

        for (int i = 0; i < N; i++) {
            uint32_t mask      = 1u << i;
            double   scan_cost = CostModel::costSequentialScan(tables[i].stats);
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
                    if (base_cost >= PRUNE_BUDGET) continue;

                    const JoinEdge* edge = findEdge(left_mask, right_mask);
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

                    for (const auto& cf : cross_filters) {
                        // Condition: This new join subset satisfies ALL the tables the filter needs, 
                        // AND neither the left branch nor the right branch could satisfy it alone.
                        if ((cf.required_mask & subset) == cf.required_mask &&
                            (cf.required_mask & left_mask) != cf.required_mask &&
                            (cf.required_mask & right_mask) != cf.required_mask) {
                            
                            auto filter_node = std::make_unique<LogicalFilterNode>(cloneExpr(cf.condition.get()));
                            filter_node->children.push_back(std::move(decision.node));
                            decision.node = std::move(filter_node);
                        }
                    }

                    double total_cost = base_cost + decision.cost;

                    if (!memo.count(subset) || total_cost < memo[subset].cost) {
                        memo[subset] = DPEntry(total_cost, out_stats, std::move(decision.node));
                    }
                }

                uint32_t c = subset & -subset;
                uint32_t r = subset + c;
                subset = (((r ^ subset) >> 2) / c) | r;
            }
        }
    }

    void collectWrappers(PlanNode* node, std::vector<PlanNode*>& out) {
        if (!node) return;

        if (node->type == NodeType::LOGICAL_JOIN ||
            node->type == NodeType::LOGICAL_GET)
            return;

        if (node->type == NodeType::LOGICAL_FILTER &&
            !node->children.empty() &&
            (node->children[0]->type == NodeType::LOGICAL_GET || node->children[0]->type == NodeType::LOGICAL_JOIN))
            return; 

        out.push_back(node);
        if (!node->children.empty())
            collectWrappers(node->children[0].get(), out);
    }

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

    double getLastCost() const { return last_best_cost; }

    std::unique_ptr<PlanNode> optimize(std::unique_ptr<PlanNode> root) {
        std::cout << "\n--- STARTING COST-BASED OPTIMIZATION (DP JOIN ENUMERATION) ---\n";

        std::vector<PlanNode*> wrappers;
        collectWrappers(root.get(), wrappers);

        // Make sure we start extracting EXACTLY where the wrappers stopped
        PlanNode* join_tree_root = root.get();
        if (!wrappers.empty() && !wrappers.back()->children.empty()) {
            join_tree_root = wrappers.back()->children[0].get();
        }

        extractJoinTree(join_tree_root);
        resolveCrossFilters(); // Converts stashed nodes into DP masks!

        int N = (int)tables.size();
        std::cout << "[DP] Found " << N << " table(s) to enumerate.\n";

        if (N == 0) return root;

        if (N == 1) {
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
        last_best_cost = best.cost;
        return wrapNonJoinNodes(std::move(best.plan), wrappers);
    }
};

#endif 