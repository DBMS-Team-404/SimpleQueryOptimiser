#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <memory>
#include <iostream>
#include "ast_nodes.hpp"

class RuleBasedOptimizer {
public:
    // The main entry point: takes the root of the tree and returns an optimized version
    std::unique_ptr<PlanNode> optimize(std::unique_ptr<PlanNode> root) {
        std::cout << "\n--- STARTING RULE-BASED OPTIMIZATION ---\n";
        
        // Rule 1: Predicate Pushdown
        root = pushDownFilters(std::move(root));
        
        return root;
    }

private:
    std::unique_ptr<PlanNode> pushDownFilters(std::unique_ptr<PlanNode> node) {
        if (!node) return nullptr;

        // 1. Process children first (Bottom-up)
        for (auto& child : node->children) {
            child = pushDownFilters(std::move(child));
        }

        // 2. Check for Filter -> Join pattern
        if (node->type == NodeType::LOGICAL_FILTER) {
            if (!node->children.empty() && node->children[0]->type == NodeType::LOGICAL_JOIN) {
                
                std::cout << "[RBO] Pushing Filter below Join..." << std::endl;

                // Grab pointers to the pieces
                auto filter = std::move(node);
                auto join = std::move(filter->children[0]);
                
                // Logic: Move the join to the top, and put the filter under the join
                // For now, we'll just put it on the Left Child (child[0]) of the join
                auto join_left_child = std::move(join->children[0]);
                
                filter->children[0] = std::move(join_left_child); // Filter now sits on Table A
                join->children[0] = std::move(filter);            // Join now sits on top of Filter

                return join; // The Join is now the new "root" of this subtree
            }
        }

        return node;
    }
};

#endif