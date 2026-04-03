#include <iostream>
#include <exception>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern json parse_sql_to_json();

int main() {
    try {
        json ast = parse_sql_to_json();
        
        if (!ast.is_null() && !ast.empty()) {
            std::cout << "\n[SUCCESS] --- Generated Logical Plan (AST) ---\n";
            std::cout << ast.dump(4) << std::endl;
        } else {
            std::cerr << "\n[ERROR] AST is empty or parsing failed." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "\n[RUNTIME ERROR]: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}