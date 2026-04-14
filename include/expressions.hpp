#ifndef EXPRESSIONS_HPP
#define EXPRESSIONS_HPP

#include <string>
#include <memory>

// Define the types of expressions we support based on our API Contract
enum class ExpressionType {
    COLUMN,
    CONSTANT,
    COMPARISON,
    LOGICAL_AND,
    LOGICAL_OR
};

// The recursive struct to hold our condition trees
struct Expression {
    ExpressionType type;
    
    // For Branch Nodes (COMPARISON, LOGICAL_AND, LOGICAL_OR)
    std::string op; // Stores operators like ">", "=", "AND"
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    // For Leaf Nodes (COLUMN, CONSTANT)
    std::string value; 
    
    // Constructor for Leaf Nodes (No children)
    Expression(ExpressionType t, const std::string& val) 
        : type(t), value(val) {}

    // Constructor for Branch Nodes (Has children)
    Expression(ExpressionType t, const std::string& o, 
               std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : type(t), op(o), left(std::move(l)), right(std::move(r)) {}
};

#endif // EXPRESSIONS_HPP