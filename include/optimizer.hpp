#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "ast_nodes.hpp"
#include "catalog.hpp"

class RuleBasedOptimizer {
private:
    ICatalog& catalog;

    // Safely searches the entire subtree to see if a table alias exists
    bool hasTable(PlanNode* node, const std::string& target_alias) {
        if (!node) return false;
        
        if (node->type == NodeType::LOGICAL_GET) {
            auto* get_node = static_cast<LogicalGetNode*>(node);
            std::string current = get_node->alias.empty() ? get_node->table_name : get_node->alias;
            return current == target_alias;
        }
        
        for (const auto& child : node->children) {
            if (hasTable(child.get(), target_alias)) return true;
        }
        return false;
    }

    // Checks if a subtree contains ALL the requested aliases
    bool hasAllTables(PlanNode* node, const std::vector<std::string>& aliases) {
        for (const auto& alias : aliases) {
            if (!hasTable(node, alias)) return false;
        }
        return true;
    }

    // Finds ALL explicitly referenced table aliases (e.g., "u1" from "u1.age")
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

    // Extracts the raw column name without the prefix (e.g., gets "age" from "u1.age" or "age")
    std::string getRawColumnName(Expression* expr) {
        if (!expr) return "";
        if (expr->type == ExpressionType::COLUMN) {
            std::string col = expr->value;
            size_t dot_pos = col.find('.');
            if (dot_pos != std::string::npos) {
                return col.substr(dot_pos + 1);
            }
            return col;
        }
        std::string left_col = getRawColumnName(expr->left.get());
        if (!left_col.empty()) return left_col;
        return getRawColumnName(expr->right.get());
    }

    // Searches the tree and asks the Catalog who owns this unqualified column
    std::string resolveTableForColumn(PlanNode* node, const std::string& col_name) {
        if (!node) return "";
        
        if (node->type == NodeType::LOGICAL_GET) {
            auto* get_node = static_cast<LogicalGetNode*>(node);
            if (catalog.columnExists(get_node->table_name, col_name)) {
                return get_node->alias.empty() ? get_node->table_name : get_node->alias;
            }
            return "";
        }
        
        for (const auto& child : node->children) {
            std::string found = resolveTableForColumn(child.get(), col_name);
            if (!found.empty()) return found;
        }
        return "";
    }

    // Recursively pushes a single-table filter down until it hits the specific leaf node
    std::unique_ptr<PlanNode> insertFilterBelowJoin(std::unique_ptr<PlanNode> tree_node, std::unique_ptr<PlanNode> filter_node, const std::string& target_table) {
        if (!tree_node) return filter_node;

        if (tree_node->type == NodeType::LOGICAL_GET) {
            filter_node->children.clear(); 
            filter_node->children.push_back(std::move(tree_node));
            return filter_node;
        }

        if (tree_node->type == NodeType::LOGICAL_FILTER) {
            tree_node->children[0] = insertFilterBelowJoin(std::move(tree_node->children[0]), std::move(filter_node), target_table);
            return tree_node;
        }

        if (tree_node->type == NodeType::LOGICAL_JOIN) {
            if (hasTable(tree_node->children[0].get(), target_table)) {
                tree_node->children[0] = insertFilterBelowJoin(std::move(tree_node->children[0]), std::move(filter_node), target_table);
            } else if (hasTable(tree_node->children[1].get(), target_table)) {
                tree_node->children[1] = insertFilterBelowJoin(std::move(tree_node->children[1]), std::move(filter_node), target_table);
            }
        }
        
        return tree_node;
    }

    // Pushes a multi-table filter down to its Lowest Common Ancestor (LCA)
    std::unique_ptr<PlanNode> pushMultiTableFilter(std::unique_ptr<PlanNode> tree_node, std::unique_ptr<PlanNode> filter_node, const std::vector<std::string>& aliases) {
        if (!tree_node) return filter_node;

        if (tree_node->type == NodeType::LOGICAL_FILTER) {
            tree_node->children[0] = pushMultiTableFilter(std::move(tree_node->children[0]), std::move(filter_node), aliases);
            return tree_node;
        }

        if (tree_node->type == NodeType::LOGICAL_JOIN) {
            if (hasAllTables(tree_node->children[0].get(), aliases)) {
                tree_node->children[0] = pushMultiTableFilter(std::move(tree_node->children[0]), std::move(filter_node), aliases);
                return tree_node;
            }
            else if (hasAllTables(tree_node->children[1].get(), aliases)) {
                tree_node->children[1] = pushMultiTableFilter(std::move(tree_node->children[1]), std::move(filter_node), aliases);
                return tree_node;
            }
            
            filter_node->children.clear();
            filter_node->children.push_back(std::move(tree_node));
            return filter_node;
        }

        filter_node->children.clear();
        filter_node->children.push_back(std::move(tree_node));
        return filter_node;
    }

public:
    RuleBasedOptimizer(ICatalog& cat) : catalog(cat) {}

    std::unique_ptr<PlanNode> optimize(std::unique_ptr<PlanNode> root) {
        std::cout << "\n--- STARTING RULE-BASED OPTIMIZATION ---\n";
        root = pushDownFilters(std::move(root));
        return root;
    }

private:
    std::unique_ptr<PlanNode> pushDownFilters(std::unique_ptr<PlanNode> node) {
        if (!node) return nullptr;

        for (auto& child : node->children) {
            child = pushDownFilters(std::move(child));
        }

        if (node->type == NodeType::LOGICAL_FILTER) {
            auto* filter_node = static_cast<LogicalFilterNode*>(node.get());

            if (filter_node->predicate->type == ExpressionType::LOGICAL_AND) {
                auto left_pred  = std::move(filter_node->predicate->left);
                auto right_pred = std::move(filter_node->predicate->right);

                auto left_filter = std::make_unique<LogicalFilterNode>(std::move(left_pred));
                left_filter->children = std::move(node->children);

                auto right_filter = std::make_unique<LogicalFilterNode>(std::move(right_pred));
                right_filter->children.push_back(std::move(left_filter));

                return pushDownFilters(std::move(right_filter));
            }

            if (!node->children.empty() && 
               (node->children[0]->type == NodeType::LOGICAL_JOIN || 
                node->children[0]->type == NodeType::LOGICAL_FILTER)) {
                
                auto* filter = static_cast<LogicalFilterNode*>(node.get());
                
                std::vector<std::string> referenced_aliases;
                getReferencedAliases(filter->predicate.get(), referenced_aliases);

                auto filter_owned = std::move(node);
                auto child_tree = std::move(filter_owned->children[0]); 
                
                bool pushed = false;

                // SCENARIO 1: Qualified Single Table Filter
                if (referenced_aliases.size() == 1) {
                    std::string target_alias = referenced_aliases[0];
                    if (hasTable(child_tree.get(), target_alias)) {
                        std::cout << "[RBO] Pushing Qualified Filter down to (" << target_alias << ")...\n";
                        child_tree = insertFilterBelowJoin(std::move(child_tree), std::move(filter_owned), target_alias);
                        pushed = true;
                    } 
                } 
                // SCENARIO 2: Cross-Table Filter (Theta-Join)
                else if (referenced_aliases.size() > 1) {
                    std::cout << "[RBO] Pushing Multi-Table Filter to Lowest Common Ancestor...\n";
                    child_tree = pushMultiTableFilter(std::move(child_tree), std::move(filter_owned), referenced_aliases);
                    pushed = true;
                }
                // SCENARIO 3: Unqualified column -> Ask the Catalog!
                else {
                    std::string raw_col = getRawColumnName(filter->predicate.get());
                    std::string target_alias = resolveTableForColumn(child_tree.get(), raw_col);

                    if (!target_alias.empty()) {
                        std::cout << "[RBO] Catalog resolved unqualified '" << raw_col 
                                  << "' to table (" << target_alias << "). Pushing down...\n";
                        child_tree = insertFilterBelowJoin(std::move(child_tree), std::move(filter_owned), target_alias); 
                        pushed = true;
                    } else {
                        std::cout << "[RBO] WARNING: Catalog could not find column '" << raw_col << "'. Deferring.\n";
                    }
                }

                if (!pushed) {
                    filter_owned->children.clear();
                    filter_owned->children.push_back(std::move(child_tree));
                    return filter_owned;
                }
                
                return child_tree;
            }
        }
        return node;
    }
};

#endif // OPTIMIZER_HPP