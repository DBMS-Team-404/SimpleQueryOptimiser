#ifndef COST_BASED_OPTIMIZER_HPP
#define COST_BASED_OPTIMIZER_HPP

#include <memory>
#include <iostream>
#include <string>
#include <utility> // For std::swap
#include "ast_nodes.hpp"
#include "catalog.hpp"

class CostBasedOptimizer {
private:
    ICatalog& catalog;

public:
    CostBasedOptimizer(ICatalog& cat) : catalog(cat) {}

    std::unique_ptr<PlanNode> optimize(std::unique_ptr<PlanNode> root) {
        std::cout << "\n--- STARTING COST-BASED OPTIMIZATION ---\n";
        
        // 1. Evaluate the initial cost
        double initial_cost = estimateCost(root.get());
        std::cout << "\n[CBO] Initial Estimated Execution Cost: " << initial_cost << "\n";
        
        // 2. Shuffle the tree to find a cheaper path!
        root = reorderJoins(std::move(root));

        // 3. Evaluate the final optimized cost
        double final_cost = estimateCost(root.get());
        std::cout << "\n[CBO] Final Optimized Execution Cost: " << final_cost << "\n";

        return root;
    }

private:
    std::unique_ptr<PlanNode> reorderJoins(std::unique_ptr<PlanNode> node) {
        if (!node) return nullptr;

        // Recursively go down the tree first
        for (auto& child : node->children) {
            child = reorderJoins(std::move(child));
        }

        // If we find a JOIN, let's see if we can optimize it!
        if (node->type == NodeType::LOGICAL_JOIN && node->children.size() == 2) {
            double left_cost = estimateCost(node->children[0].get());
            double right_cost = estimateCost(node->children[1].get());

            // THE MAGIC: Always put the smaller/cheaper table on the LEFT (Outer Loop)
            if (right_cost < left_cost) {
                std::cout << "\n[CBO] Optimization found! Right side is cheaper (" 
                          << right_cost << " < " << left_cost << "). Swapping Join Order...\n";
                
                // C++ feature to instantly swap the two children branches in memory
                std::swap(node->children[0], node->children[1]);
            }
        }

        return node;
    }

    double estimateCost(PlanNode* node) {
        if (!node) return 0.0;

        double cost = 0.0;
        double left_cost = 0.0;
        double right_cost = 0.0;

        if (node->children.size() > 0) left_cost = estimateCost(node->children[0].get());
        if (node->children.size() > 1) right_cost = estimateCost(node->children[1].get());

        if (node->type == NodeType::LOGICAL_GET) {
            auto* get_node = static_cast<LogicalGetNode*>(node);
            TableStats stats = catalog.getTableStats(get_node->table_name);
            cost = stats.num_blocks + (stats.num_tuples * 0.1);
        }
        else if (node->type == NodeType::LOGICAL_JOIN) {
            // Formula: Cost of Left + Cost of Right + Penalty (Left is Outer Loop)
            cost = left_cost + right_cost + (left_cost * 1.5); 
        }
        else {
            cost = left_cost + right_cost; 
        }

        return cost;
    }
};

#endif