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
    // Replace pushDownFilters with this version that splits AND predicates

    std::unique_ptr<PlanNode> pushDownFilters(std::unique_ptr<PlanNode> node) {
        if (!node) return nullptr;

        for (auto& child : node->children)
            child = pushDownFilters(std::move(child));

        if (node->type == NodeType::LOGICAL_FILTER) {
            auto* filter_node = static_cast<LogicalFilterNode*>(node.get());

            // If predicate is AND, split it and push each half independently
            if (filter_node->predicate->type == ExpressionType::LOGICAL_AND) {
                auto left_pred  = std::move(filter_node->predicate->left);
                auto right_pred = std::move(filter_node->predicate->right);

                // Build two separate FILTER nodes and push each
                auto left_filter = std::make_unique<LogicalFilterNode>(std::move(left_pred));
                left_filter->children = std::move(node->children);

                auto right_filter = std::make_unique<LogicalFilterNode>(std::move(right_pred));
                right_filter->children.push_back(std::move(left_filter));

                return pushDownFilters(std::move(right_filter));
            }

            // Single predicate — existing logic handles this correctly
            if (!node->children.empty() && 
                node->children[0]->type == NodeType::LOGICAL_JOIN) {

                auto* filter = static_cast<LogicalFilterNode*>(node.get());
                std::string col_name = getColumnName(filter->predicate.get());

                size_t dot_pos = col_name.find('.');
                if (dot_pos != std::string::npos) 
                    col_name = col_name.substr(dot_pos + 1);

                auto filter_owned = std::move(node);
                auto join = std::move(filter_owned->children[0]);

                std::string left_table  = getTableName(join->children[0].get());
                std::string right_table = getTableName(join->children[1].get());

                if (catalog.columnExists(left_table, col_name)) {
                    std::cout << "[RBO] Pushing Filter onto Left Table (" << left_table << ")...\n";
                    auto join_left = std::move(join->children[0]);
                    filter_owned->children[0] = std::move(join_left);
                    join->children[0] = std::move(filter_owned);
                }
                else if (catalog.columnExists(right_table, col_name)) {
                    std::cout << "[RBO] Pushing Filter onto Right Table (" << right_table << ")...\n";
                    auto join_right = std::move(join->children[1]);
                    filter_owned->children[0] = std::move(join_right);
                    join->children[1] = std::move(filter_owned);
                }
                else {
                    filter_owned->children[0] = std::move(join);
                    return filter_owned;
                }
                return join;
            }
        }
        return node;
    }
};

#endif