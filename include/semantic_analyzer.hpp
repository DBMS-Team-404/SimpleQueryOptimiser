#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

#include "ast_nodes.hpp" 
#include "catalog.hpp"   

class SemanticAnalyzer {
private:
    // IMPORTANT: Changed to use the ICatalog interface from your catalog.hpp
    ICatalog& catalog;
    std::vector<std::string> active_tables; 

public:
    SemanticAnalyzer(ICatalog& cat) : catalog(cat) {}

    bool validateQuery(PlanNode* root) {
        active_tables.clear();
        try {
            visitNode(root);
            return true; 
        } catch (const std::runtime_error& e) {
            std::cerr << "Semantic Error: " << e.what() << std::endl;
            return false;
        }
    }

private:
    void visitNode(PlanNode* node) {
        if (node == nullptr) return;

        for (const auto& child : node->children) {
            visitNode(child.get()); 
        }

        if (auto* get_node = dynamic_cast<LogicalGetNode*>(node)) {
            // Your catalog throws an invalid_argument if a table is missing.
            // We catch it and convert it to a semantic error.
            try {
                catalog.getTableStats(get_node->table_name);
                active_tables.push_back(get_node->table_name);
            } catch (const std::invalid_argument& e) {
                throw std::runtime_error("Table does not exist in catalog: " + get_node->table_name);
            }
        }
        else if (auto* filter_node = dynamic_cast<LogicalFilterNode*>(node)) {
            validateExpression(filter_node->predicate.get());
        }
        else if (auto* project_node = dynamic_cast<LogicalProjectNode*>(node)) {
            for (const auto& col_name : project_node->columns) {
                validateColumn(col_name);
            }
        }
        else if (auto* join_node = dynamic_cast<LogicalJoinNode*>(node)) {
            validateExpression(join_node->condition.get());
        }
    }

    void validateColumn(const std::string& column_name) {
        if (column_name == "*") return; // Allow select star

        // NEW: Check if the column is fully qualified (e.g., "orders.user_id")
        size_t dot_pos = column_name.find('.');
        if (dot_pos != std::string::npos) {
            std::string table_part = column_name.substr(0, dot_pos);
            std::string col_part = column_name.substr(dot_pos + 1);

            // 1. Verify the table is actually in the query
            bool table_active = false;
            for (const auto& active_table : active_tables) {
                if (active_table == table_part) {
                    table_active = true;
                    break;
                }
            }
            if (!table_active) {
                throw std::runtime_error("Table prefix '" + table_part + "' is not in the FROM/JOIN clause.");
            }

            // 2. Ask the catalog if the specific column exists in that specific table
            if (!catalog.columnExists(table_part, col_part)) {
                throw std::runtime_error("Column '" + col_part + "' not found in table '" + table_part + "'.");
            }
            return; // Validated successfully!
        }

        // ORIGINAL: If no dot exists, just search all active tables
        bool column_found = false;
        for (const auto& table : active_tables) {
            if (catalog.columnExists(table, column_name)) {
                column_found = true;
                break;
            }
        }

        if (!column_found) {
            throw std::runtime_error("Column not found in active tables: " + column_name);
        }
    }

    void validateExpression(Expression* expr) {
        if (expr == nullptr) return;

        if (expr->type == ExpressionType::COLUMN) {
            validateColumn(expr->value);
        }
        else if (expr->type == ExpressionType::COMPARISON || 
                 expr->type == ExpressionType::LOGICAL_AND || 
                 expr->type == ExpressionType::LOGICAL_OR) {
            
            validateExpression(expr->left.get());
            validateExpression(expr->right.get());
        }
    }
};

#endif // SEMANTIC_ANALYZER_HPP