#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>
#include <unordered_map>

#include "ast_nodes.hpp" 
#include "catalog.hpp"   

class SemanticAnalyzer {
private:
    ICatalog& catalog;
    
    // Maps the alias (e.g., "u") to the actual table name (e.g., "users")
    // This allows us to resolve u.id to users.id in the catalog.
    std::unordered_map<std::string, std::string> alias_to_table; 

public:
    SemanticAnalyzer(ICatalog& cat) : catalog(cat) {}

    bool validateQuery(PlanNode* root) {
        alias_to_table.clear();
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

        // Walk bottom-up so tables are registered before their parent filters/projects
        for (const auto& child : node->children) {
            visitNode(child.get()); 
        }

        if (auto* get_node = dynamic_cast<LogicalGetNode*>(node)) {
            try {
                // Verify the physical table exists in the DB[cite: 111].
                catalog.getTableStats(get_node->table_name);
                
                // Register the alias-to-table mapping. 
                // If no alias was provided, parser.y sets alias = table_name[cite: 396].
                alias_to_table[get_node->alias] = get_node->table_name;
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
        if (column_name == "*") return; 

        size_t dot_pos = column_name.find('.');
        if (dot_pos != std::string::npos) {
            std::string alias_part = column_name.substr(0, dot_pos);
            std::string col_part = column_name.substr(dot_pos + 1);

            // 1. Check if the alias is in scope for this query.
            if (alias_to_table.find(alias_part) == alias_to_table.end()) {
                throw std::runtime_error("Table prefix '" + alias_part + "' is not in the FROM/JOIN clause.");
            }

            // 2. Resolve alias to physical table and check column existence.
            std::string physical_table = alias_to_table[alias_part];
            if (!catalog.columnExists(physical_table, col_part)) {
                throw std::runtime_error("Column '" + col_part + "' not found in table '" + physical_table + "'.");
            }
            return; 
        }

        // Handle unqualified columns (search all tables currently in scope).
        bool column_found = false;
        for (const auto& pair : alias_to_table) {
            if (catalog.columnExists(pair.second, column_name)) {
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