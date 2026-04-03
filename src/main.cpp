#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <cstdio> // Required for FILE* to interface with Flex/Bison
#include "../include/ast_nodes.hpp"
#include "json.hpp"
#include "semantic_analyzer.hpp"
#include "optimizer.hpp"
#include "cost_based_optimizer.hpp"

using json = nlohmann::json;

// ---------------------------------------------------------
// FORWARD DECLARATIONS
// ---------------------------------------------------------
// 1. Our C++ AST parser
std::unique_ptr<PlanNode> parseNode(const json& j);

// 2. The Flex/Bison hooks (Bridging C to C++)
extern FILE* yyin; // Flex's input file pointer
extern json parse_sql_to_json(); // The helper function from your teammate's parser.y

// ---------------------------------------------------------
// 1. EXPRESSION PARSER
// ---------------------------------------------------------
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

// ---------------------------------------------------------
// 2. NODE PARSER
// ---------------------------------------------------------
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

// ---------------------------------------------------------
// 3. COMMAND LINE INTERFACE (CLI)
// ---------------------------------------------------------
int main(int argc, char* argv[]) {
    std::cout << "=========================================\n";
    std::cout << "      DBMSTeam 404 Optimizer Engine      \n";
    std::cout << "=========================================\n";

    if (argc < 2) {
        std::cerr << "Error: No input file provided.\n";
        std::cerr << "Usage: ./optimizer <path_to_sql_file.sql>\n";
        return 1;
    }

    std::string file_path = argv[1];
    
    // 1. Open the SQL file using standard C FILE* (because Flex needs it)
    FILE* input_file = fopen(file_path.c_str(), "r");
    if (!input_file) {
        std::cerr << "Error: Could not open file " << file_path << "\n";
        return 1;
    }

    // 2. Tell the Flex lexer to read from this file instead of the keyboard
    yyin = input_file;

    try {
        std::cout << "Parsing SQL file: " << file_path << "...\n\n";

        // 3. The Magic: Call Bison to parse SQL and return our JSON Contract!
        json query_json = parse_sql_to_json();

        if (query_json.empty() || query_json.is_null()) {
            std::cerr << "Failed to parse SQL into AST. Check syntax.\n";
            fclose(input_file);
            return 1;
        }

        // 4. Our original magic: Convert JSON to C++ Memory Structures
        std::unique_ptr<PlanNode> root = parseNode(query_json);

        // Print the resulting tree to the terminal
        std::cout << "--- LOGICAL EXECUTION PLAN ---\n";
        root->print(0);

        MockCatalog my_catalog; 
        
        // 2. Initialize your analyzer with your catalog interface
        SemanticAnalyzer analyzer(my_catalog);

        // 3. Pass the parsed tree (root) into the validate function.
        if (analyzer.validateQuery(root.get())) {
            std::cout << "\n[SUCCESS] Semantic check passed! Ready for optimization.\n";
            // --- NEW OPTIMIZATION STEP ---
            RuleBasedOptimizer rbo(my_catalog);
            root = rbo.optimize(std::move(root));
            
            std::cout << "\n--- OPTIMIZED EXECUTION PLAN ---\n";
            root->print(0);
            // -----------------------------
            CostBasedOptimizer cbo(my_catalog); // Pass the catalog so it can read stats!
            root = cbo.optimize(std::move(root));

            std::cout << "\n--- FINAL OPTIMIZED EXECUTION PLAN ---\n";
            root->print(0);
        } else {
            std::cout << "\n[FAILED] Semantic check failed. Stopping execution.\n";
            return 1; 
        }
        
        std::cout << "\nStatus: Ingestion Complete!\n";

    } catch (const json::exception& e) {
        std::cerr << "AST Generation Error: " << e.what() << "\n";
        std::cerr << "Make sure your Bison grammar outputs the correct API Contract!\n";
        fclose(input_file);
        return 1;
    }

    fclose(input_file);
    return 0;
}