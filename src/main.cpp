#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "../include/ast_nodes.hpp"
#include "../include/nlohmann/json.hpp" // Make sure this path matches your folder structure!

using json = nlohmann::json;

// Forward declaration so nodes can call it recursively
std::unique_ptr<PlanNode> parseNode(const json& j);

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
        std::cerr << "Usage: ./optimizer <path_to_json_file>\n";
        return 1;
    }

    std::string file_path = argv[1];
    std::ifstream input_file(file_path);
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open file " << file_path << "\n";
        return 1;
    }

    try {
        // Read the file into a JSON object
        json query_json;
        input_file >> query_json;

        // The Moment of Truth: Parse the JSON into C++ Objects
        std::cout << "Parsing file: " << file_path << "...\n\n";
        std::unique_ptr<PlanNode> root = parseNode(query_json);

        // Print the resulting tree to the terminal
        std::cout << "--- LOGICAL EXECUTION PLAN ---\n";
        root->print(0);
        std::cout << "\nStatus: Ingestion Complete!\n";

    } catch (const json::exception& e) {
        std::cerr << "JSON Parsing Error: " << e.what() << "\n";
        std::cerr << "Make sure your JSON strictly follows the API Contract!\n";
        return 1;
    }

    return 0;
}