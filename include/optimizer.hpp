#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <memory>
#include <iostream>
#include <string>
#include "ast_nodes.hpp"
#include "catalog.hpp"

class RuleBasedOptimizer {
private:
    ICatalog& catalog;

    // Helper: Digs down a branch to find the name of the table sitting at the bottom
    std::string getTableName(PlanNode* node) {
        if (!node) return "";
        if (node->type == NodeType::LOGICAL_GET) {
            return static_cast<LogicalGetNode*>(node)->table_name;
        }
        if (!node->children.empty()) {
            return getTableName(node->children[0].get());
        }
        return "";
    }

    // Helper: Extracts the actual column name from an Expression like "age > 18"
    std::string getColumnName(Expression* expr) {
        if (!expr) return "";
        if (expr->type == ExpressionType::COLUMN) return expr->value;
        
        std::string left_col = getColumnName(expr->left.get());
        if (!left_col.empty()) return left_col;
        
        return getColumnName(expr->right.get());
    }

public:
    // NOW REQUIRES THE CATALOG!
    RuleBasedOptimizer(ICatalog& cat) : catalog(cat) {}

    std::unique_ptr<PlanNode> optimize(std::unique_ptr<PlanNode> root) {
        std::cout << "\n--- STARTING RULE-BASED OPTIMIZATION ---\n";
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

        // 2. The Smart Filter Pushdown
        if (node->type == NodeType::LOGICAL_FILTER) {
            if (!node->children.empty() && node->children[0]->type == NodeType::LOGICAL_JOIN) {
                
                // Extract the column name we are filtering on (e.g., "age" or "users.age")
                auto filter_node = static_cast<LogicalFilterNode*>(node.get());
                std::string col_name = getColumnName(filter_node->predicate.get());
                
                // If it's a fully qualified name (users.age), strip it down to just "age" for the catalog lookup
                size_t dot_pos = col_name.find('.');
                if (dot_pos != std::string::npos) col_name = col_name.substr(dot_pos + 1);

                // Grab our pointers
                auto filter = std::move(node);
                auto join = std::move(filter->children[0]);
                
                // Figure out which tables are on the left and right of the join
                std::string left_table = getTableName(join->children[0].get());
                std::string right_table = getTableName(join->children[1].get());

                // ASK THE CATALOG: Who owns this column?
                if (catalog.columnExists(left_table, col_name)) {
                    std::cout << "[RBO] Pushing Filter below Join onto Left Table (" << left_table << ")...\n";
                    auto join_left_child = std::move(join->children[0]);
                    filter->children[0] = std::move(join_left_child);
                    join->children[0] = std::move(filter);
                } 
                else if (catalog.columnExists(right_table, col_name)) {
                    std::cout << "[RBO] Pushing Filter below Join onto Right Table (" << right_table << ")...\n";
                    auto join_right_child = std::move(join->children[1]);
                    filter->children[0] = std::move(join_right_child);
                    join->children[1] = std::move(filter);
                }
                else {
                    // Fallback: If we can't figure it out, just leave it on top of the join
                    filter->children[0] = std::move(join);
                    return filter;
                }

                return join; 
            }
        }

        return node;
    }
};

#endif