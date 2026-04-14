#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstdio>
#include <bitset>
#include "../include/ast_nodes.hpp"
#include "semantic_analyzer.hpp"
#include "optimizer.hpp"
#include "cost_based_optimizer.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

// Our C++ AST parser
std::unique_ptr<PlanNode> parseNode(const json& j);

extern FILE* yyin; // Flex's input file pointer
extern json parse_sql_to_json();

// EXPRESSION PARSER
std::unique_ptr<Expression> parseExpression(const json& j) {
    std::string type_str = j.at("expression_type").get<std::string>();
    
    // Parse Leaf Nodes
    if (type_str == "COLUMN" || type_str == "CONSTANT") {
        ExpressionType t = (type_str == "COLUMN") ? ExpressionType::COLUMN : ExpressionType::CONSTANT;
        // We read 'value' as a string, whether it's "age" or "18"
        std::string val;
        if (j.at("value").is_number()) {
            val = std::to_string(j.at("value").get<int>());
        } else {
            val = j.at("value").get<std::string>();
        }
        return std::make_unique<Expression>(t, val);
    }
    
    // Parse Branch Nodes (Comparisons and AND/OR)
    ExpressionType t;
    if (type_str == "COMPARISON") t = ExpressionType::COMPARISON;
    else if (type_str == "LOGICAL_AND") t = ExpressionType::LOGICAL_AND;
    else t = ExpressionType::LOGICAL_OR;

    std::string op = j.contains("operator") ? j.at("operator").get<std::string>() : type_str;
    
    // Recursively parse the left and right sides of the condition
    auto left_expr = parseExpression(j.at("left"));
    auto right_expr = parseExpression(j.at("right"));

    return std::make_unique<Expression>(t, op, std::move(left_expr), std::move(right_expr));
}

// NODE PARSER
std::unique_ptr<PlanNode> parseNode(const json& j) {
    std::string type = j.at("node_type").get<std::string>();
    const auto& props = j.at("properties");
    std::unique_ptr<PlanNode> node_ptr = nullptr;

    // Instantiate the correct class based on the JSON string
    if (type == "LOGICAL_GET") {
        node_ptr = std::make_unique<LogicalGetNode>(
            props.at("table").get<std::string>(),
            props.at("alias").get<std::string>()
        );
    } 
    else if (type == "LOGICAL_FILTER") {
        node_ptr = std::make_unique<LogicalFilterNode>(
            parseExpression(props.at("predicate"))
        );
    }
    else if (type == "LOGICAL_PROJECT") {
        std::vector<std::string> cols;
        for (const auto& col : props.at("columns")) cols.push_back(col.get<std::string>());
        node_ptr = std::make_unique<LogicalProjectNode>(cols);
    }
    else if (type == "LOGICAL_JOIN") {
        JoinType jt = JoinType::INNER; // Default
        std::string jt_str = props.at("join_type").get<std::string>();
        if (jt_str == "LEFT") jt = JoinType::LEFT;
        else if (jt_str == "CROSS") jt = JoinType::CROSS;

        node_ptr = std::make_unique<LogicalJoinNode>(
            jt, parseExpression(props.at("condition"))
        );
    }
    else if (type == "LOGICAL_AGGREGATE") {
        std::vector<std::string> group_cols;
        // Extract the grouping columns from the JSON
        if (props.contains("group_columns")) {
            for (const auto& col : props.at("group_columns")) {
                group_cols.push_back(col.get<std::string>());
            }
        }
        // Create an empty aggregates vector for now (to satisfy the constructor)
        std::vector<AggregateConfig> aggs; 
        node_ptr = std::make_unique<LogicalAggregateNode>(group_cols, aggs);
    }
    else {
        std::cerr << "Unsupported node type: " << type << "\n";
        exit(1);
    }

    // Recursively parse all children
    if (j.contains("children")) {
        for (const auto& child_json : j.at("children")) {
            node_ptr->children.push_back(parseNode(child_json));
        }
    }

    return node_ptr;
}

// D3.js JSON GENERATOR
// Recursively turns our C++ PlanNode tree into a nested JSON object
json planToJson(const PlanNode* node) {
    if (!node) return nullptr;
    
    json j;
    // Map our C++ Node Types to Frontend Labels and CSS classes
    switch (node->type) {
        case NodeType::LOGICAL_GET: {
            auto* n = static_cast<const LogicalGetNode*>(node);
            j["name"] = "SCAN";
            j["details"] = "Table: " + n->table_name + " (" + n->alias + ")";
            j["type"] = "scan";
            break;
        }
        case NodeType::LOGICAL_FILTER: {
            j["name"] = "FILTER";
            j["details"] = "Predicate Applied"; 
            j["type"] = "filter";
            break;
        }
        case NodeType::LOGICAL_PROJECT: {
            auto* n = static_cast<const LogicalProjectNode*>(node);
            j["name"] = "PROJECT";
            std::string cols = "";
            for (size_t i = 0; i < n->columns.size(); ++i) {
                cols += n->columns[i];
                if (i < n->columns.size() - 1) cols += ", ";
            }
            j["details"] = "Cols: " + cols;
            j["type"] = "project";
            break;
        }
        case NodeType::LOGICAL_JOIN: {
            j["name"] = "LOGICAL JOIN";
            j["details"] = "Unoptimized Join";
            j["type"] = "join";
            break;
        }
        case NodeType::PHYSICAL_HASH_JOIN: {
            j["name"] = "HASH JOIN";
            j["details"] = "Physical Hash Join Algorithm";
            j["type"] = "join";
            break;
        }
        case NodeType::PHYSICAL_NESTED_LOOP_JOIN: {
            j["name"] = "NESTED LOOP";
            j["details"] = "Physical Nested Loop Algorithm";
            j["type"] = "join";
            break;
        }
        default:
            j["name"] = "NODE";
            j["details"] = "";
            j["type"] = "other";
            break;
    }

    // Recursively process children
    j["children"] = json::array();
    for (const auto& child : node->children) {
        j["children"].push_back(planToJson(child.get()));
    }
    
    return j;
}

// Now accepts json objects instead of strings
void writeResultJson(const std::string& out_path,
                     const json& logical_plan_json,
                     const json& rbo_plan_json,
                     const json& final_plan_json,
                     double final_cost)
{
    json out;
    out["logical_plan"]  = logical_plan_json;
    out["rbo_plan"]      = rbo_plan_json;
    out["final_plan"]    = final_plan_json;
    out["final_cost"]    = final_cost;
    out["status"]        = "success";

    std::ofstream f(out_path);
    f << out.dump(2);
    std::cout << "[UI] Result written to " << out_path << "\n";
}

// COMMAND LINE INTERFACE (CLI)
int main(int argc, char* argv[]) {
    std::cout << "=========================================\n";
    std::cout << "      DBMSTeam 404 Optimizer Engine      \n";
    std::cout << "=========================================\n";

    if (argc < 2) {
        std::cerr << "Error: No input file provided.\n";
        std::cerr << "Usage: ./optimizer_test <path_to_sql_file.sql>\n";
        return 1;
    }

    std::string file_path = argv[1];
    FILE* input_file = fopen(file_path.c_str(), "r");
    if (!input_file) {
        std::cerr << "Error: Could not open file " << file_path << "\n";
        return 1;
    }

    yyin = input_file;

    // Variables to hold our JSON snapshots for the UI
    json logical_json, rbo_json, final_json;
    double final_cost = 0.0;

    try {
        std::cout << "Parsing SQL file: " << file_path << "...\n\n";
        json query_json = parse_sql_to_json();

        if (query_json.empty() || query_json.is_null()) {
            std::cerr << "Failed to parse SQL into AST. Check syntax.\n";
            fclose(input_file);
            return 1;
        }

        std::unique_ptr<PlanNode> root = parseNode(query_json);

        std::cout << "--- LOGICAL EXECUTION PLAN ---\n";
        root->print(0);
        
        // Logical Plan: Converted to JSON for the frontend
        logical_json = planToJson(root.get());

        std::unique_ptr<ICatalog> catalog_ptr;
        if (argc >= 3 && std::string(argv[2]) == "--live") {
            std::cout << "[Catalog] Connecting to live PostgreSQL...\n";
            catalog_ptr = std::make_unique<PostgresCatalog>("localhost", "5432", "optimizer_db", "postgres", "postgres123");
        } else {
            std::cout << "[Catalog] Using MockCatalog from src/catalog.json\n";
            catalog_ptr = std::make_unique<MockCatalog>("src/catalog.json");
        }

        ICatalog& my_catalog = *catalog_ptr;
        SemanticAnalyzer analyzer(my_catalog);

        if (analyzer.validateQuery(root.get())) {
            std::cout << "\n[SUCCESS] Semantic check passed! Ready for optimization.\n";
            
            RuleBasedOptimizer rbo(my_catalog);
            root = rbo.optimize(std::move(root));
            
            std::cout << "\n--- OPTIMIZED EXECUTION PLAN ---\n";
            root->print(0);
            
            // RBO Plan: Converted to JSON
            rbo_json = planToJson(root.get());
            
            CostBasedOptimizer cbo(my_catalog);
            root = cbo.optimize(std::move(root));

            std::cout << "\n--- FINAL OPTIMIZED EXECUTION PLAN ---\n";
            root->print(0);
            
            // CBO Plan & Cost: Converted to JSON
            final_json = planToJson(root.get());
            final_cost = cbo.getLastCost();

        } else {
            std::cout << "\n[FAILED] Semantic check failed. Stopping execution.\n";
            fclose(input_file);
            return 1; 
        }
        
        std::cout << "\nStatus: Ingestion Complete!\n";

    } catch (const json::exception& e) {
        std::cerr << "AST Generation Error: " << e.what() << "\n";
        fclose(input_file);
        return 1;
    }

    fclose(input_file);

    // Write the JSON for the dashboard if the flag was passed
    if (argc >= 3 && std::string(argv[2]) == "--html-out") {
        writeResultJson("result.json", logical_json, rbo_json, final_json, final_cost);
    }
    
    return 0;
}