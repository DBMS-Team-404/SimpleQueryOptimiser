#ifndef AST_NODES_HPP
#define AST_NODES_HPP

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include "expressions.hpp" // We need the Expression struct we just built!

// ---------------------------------------------------------
// ENUMS FOR NODE AND JOIN TYPES
// ---------------------------------------------------------
enum class NodeType {
    LOGICAL_GET,  // FROM
    LOGICAL_FILTER, // WHERE, HAVING
    LOGICAL_PROJECT, // SELECT
    LOGICAL_JOIN, // JOIN
    LOGICAL_AGGREGATE, // GROUP BY
    LOGICAL_SORT, // ORDER BY
    LOGICAL_LIMIT, // LIMIT
    // PHYSICAL NODES
    PHYSICAL_HASH_JOIN,
    PHYSICAL_NESTED_LOOP_JOIN
};

enum class JoinType {
    INNER,
    LEFT,
    CROSS
};

// ---------------------------------------------------------
// BASE CLASS: PlanNode
// ---------------------------------------------------------
// Every operation in our tree will be a PlanNode.
class PlanNode {
public:
    NodeType type;
    
    // A node can have multiple children (e.g., JOIN has 2, GET has 0, FILTER has 1)
    std::vector<std::unique_ptr<PlanNode>> children;

    PlanNode(NodeType t) : type(t) {}
    virtual ~PlanNode() = default; // Essential for safe memory cleanup

    // A virtual function so every node can print itself to the terminal beautifully
    virtual void print(int indent = 0) const = 0; 
};

// ---------------------------------------------------------
// DERIVED CLASSES (The 7 SQL Operations)
// ---------------------------------------------------------

// 1. GET (FROM clause)
class LogicalGetNode : public PlanNode {
public:
    std::string table_name;
    std::string alias;

    LogicalGetNode(const std::string& table, const std::string& a) 
        : PlanNode(NodeType::LOGICAL_GET), table_name(table), alias(a) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> LOGICAL_GET (" << table_name << " AS " << alias << ")\n";
    }
};

// 2. FILTER (WHERE / HAVING clause)
class LogicalFilterNode : public PlanNode {
public:
    std::unique_ptr<Expression> predicate;

    LogicalFilterNode(std::unique_ptr<Expression> pred) 
        : PlanNode(NodeType::LOGICAL_FILTER), predicate(std::move(pred)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> LOGICAL_FILTER\n";
        for (const auto& child : children) child->print(indent + 4);
    }
};

// 3. PROJECT (SELECT clause)
class LogicalProjectNode : public PlanNode {
public:
    std::vector<std::string> columns;

    LogicalProjectNode(std::vector<std::string> cols) 
        : PlanNode(NodeType::LOGICAL_PROJECT), columns(std::move(cols)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> LOGICAL_PROJECT (";
        for (size_t i = 0; i < columns.size(); ++i) {
            std::cout << columns[i] << (i < columns.size() - 1 ? ", " : "");
        }
        std::cout << ")\n";
        for (const auto& child : children) child->print(indent + 4);
    }
};

// 4. JOIN (JOIN clause)
class LogicalJoinNode : public PlanNode {
public:
    JoinType join_type;
    std::unique_ptr<Expression> condition;

    LogicalJoinNode(JoinType jt, std::unique_ptr<Expression> cond) 
        : PlanNode(NodeType::LOGICAL_JOIN), join_type(jt), condition(std::move(cond)) {}

    void print(int indent = 0) const override {
        std::string j_type = (join_type == JoinType::INNER) ? "INNER" : (join_type == JoinType::LEFT ? "LEFT" : "CROSS");
        std::cout << std::string(indent, ' ') << "-> LOGICAL_JOIN (" << j_type << ")\n";
        for (const auto& child : children) child->print(indent + 4);
    }
};

// Structs for Aggregate and Sort configurations
struct AggregateConfig {
    std::string function_name; // e.g., "SUM"
    std::string column_name;   // e.g., "amount"
};

struct SortConfig {
    std::string column_name;
    std::string direction; // "ASC" or "DESC"
};

// 5. AGGREGATE (GROUP BY clause)
class LogicalAggregateNode : public PlanNode {
public:
    std::vector<std::string> group_by_columns;
    std::vector<AggregateConfig> aggregates;

    LogicalAggregateNode(std::vector<std::string> gb, std::vector<AggregateConfig> aggs)
        : PlanNode(NodeType::LOGICAL_AGGREGATE), group_by_columns(std::move(gb)), aggregates(std::move(aggs)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> LOGICAL_AGGREGATE\n";
        for (const auto& child : children) child->print(indent + 4);
    }
};

// 6. SORT (ORDER BY clause)
class LogicalSortNode : public PlanNode {
public:
    std::vector<SortConfig> sort_columns;

    LogicalSortNode(std::vector<SortConfig> sc) 
        : PlanNode(NodeType::LOGICAL_SORT), sort_columns(std::move(sc)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> LOGICAL_SORT\n";
        for (const auto& child : children) child->print(indent + 4);
    }
};

// 7. LIMIT (LIMIT clause)
class LogicalLimitNode : public PlanNode {
public:
    int limit_count;

    LogicalLimitNode(int count) 
        : PlanNode(NodeType::LOGICAL_LIMIT), limit_count(count) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "-> LOGICAL_LIMIT (" << limit_count << ")\n";
        for (const auto& child : children) child->print(indent + 4);
    }
};

// ---------------------------------------------------------
// PHYSICAL EXECUTION NODES (The Final Output)
// ---------------------------------------------------------

class PhysicalHashJoinNode : public PlanNode {
public:
    JoinType join_type;
    std::unique_ptr<Expression> condition;

    PhysicalHashJoinNode(JoinType jt, std::unique_ptr<Expression> cond) 
        : PlanNode(NodeType::PHYSICAL_HASH_JOIN), join_type(jt), condition(std::move(cond)) {}

    // In PhysicalHashJoinNode::print():
    void print(int indent = 0) const override {
        std::string jt = (join_type == JoinType::LEFT) ? "LEFT " : 
                        (join_type == JoinType::CROSS) ? "CROSS " : "";
        std::cout << std::string(indent, ' ') 
                << "=> [PHYSICAL_" << jt << "HASH_JOIN] (Memory Optimized)\n";
        for (const auto& child : children) child->print(indent + 4);
    }
};

class PhysicalNestedLoopJoinNode : public PlanNode {
public:
    JoinType join_type;
    std::unique_ptr<Expression> condition;

    PhysicalNestedLoopJoinNode(JoinType jt, std::unique_ptr<Expression> cond) 
        : PlanNode(NodeType::PHYSICAL_NESTED_LOOP_JOIN), join_type(jt), condition(std::move(cond)) {}

    void print(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "=> [PHYSICAL_NESTED_LOOP_JOIN] (CPU Intensive)\n";
        for (const auto& child : children) child->print(indent + 4);
    }
};

#endif // AST_NODES_HPP