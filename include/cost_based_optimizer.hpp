#ifndef COST_BASED_OPTIMIZER_HPP
#define COST_BASED_OPTIMIZER_HPP

#include <memory>
#include <iostream>
#include <string>
#include <utility> 
#include "ast_nodes.hpp"
#include "catalog.hpp"
#include "cost_model.hpp" // WEEK 2 MATH INCLUDED!

class CostBasedOptimizer {
private:
    ICatalog& catalog;

    // Helper to find the underlying table size of a branch
    TableStats getStatsForBranch(PlanNode* node) {
        if (!node) return {1, 1};
        if (node->type == NodeType::LOGICAL_GET) {
            return catalog.getTableStats(static_cast<LogicalGetNode*>(node)->table_name);
        }
        if (!node->children.empty()) {
            return getStatsForBranch(node->children[0].get()); // Recursively dig down
        }
        return {1, 1}; // Fallback
    }

public:
    CostBasedOptimizer(ICatalog& cat) : catalog(cat) {}

    std::unique_ptr<PlanNode> optimize(std::unique_ptr<PlanNode> root) {
        std::cout << "\n--- STARTING COST-BASED OPTIMIZATION (LOGICAL TO PHYSICAL) ---\n";
        root = translateToPhysical(std::move(root));
        return root;
    }

private:
    std::unique_ptr<PlanNode> translateToPhysical(std::unique_ptr<PlanNode> node) {
        if (!node) return nullptr;

        // 1. Process children first (Bottom-up traversal)
        for (auto& child : node->children) {
            child = translateToPhysical(std::move(child));
        }

        // 2. THE MUTATION: If we find a Logical Join, turn it into a Physical Join
        if (node->type == NodeType::LOGICAL_JOIN && node->children.size() == 2) {
            auto logical_join = static_cast<LogicalJoinNode*>(node.get());

            // Get statistics for the left and right branches
            TableStats left_stats = getStatsForBranch(node->children[0].get());
            TableStats right_stats = getStatsForBranch(node->children[1].get());

            // REORDERING RULE: Always put the smaller table on the left
            if (right_stats.num_tuples < left_stats.num_tuples) {
                std::cout << "[CBO] Reordering: Swapping so smaller table is on the left (Outer Loop).\n";
                std::swap(node->children[0], node->children[1]);
                std::swap(left_stats, right_stats); // Swap the stats variables too
            }

            // PHYSICAL ALGORITHM SELECTION (Using Week 2 Math!)
            double hash_cost = CostModel::costHashJoin(left_stats, right_stats);
            double nlj_cost = CostModel::costNestedLoopJoin(left_stats, right_stats);

            std::unique_ptr<PlanNode> physical_node;

            if (hash_cost <= nlj_cost) {
                std::cout << "[CBO] Math Engine Selected: HASH JOIN (Cost: " << hash_cost << " vs NLJ Cost: " << nlj_cost << ")\n";
                // Create the physical node
                physical_node = std::make_unique<PhysicalHashJoinNode>(logical_join->join_type, std::move(logical_join->condition));
            } else {
                std::cout << "[CBO] Math Engine Selected: NESTED LOOP JOIN (Cost: " << nlj_cost << " vs Hash Cost: " << hash_cost << ")\n";
                physical_node = std::make_unique<PhysicalNestedLoopJoinNode>(logical_join->join_type, std::move(logical_join->condition));
            }

            // Move the children from the old Logical Node to the new Physical Node
            physical_node->children = std::move(node->children);
            
            // Return the new Physical Node (destroying the old Logical one)
            return physical_node;
        }

        return node;
    }
};

#endif