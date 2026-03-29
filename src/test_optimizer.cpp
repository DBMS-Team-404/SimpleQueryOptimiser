#include <iostream>
#include <iomanip>
#include "../include/catalog.hpp"
#include "../include/cost_model.hpp"

int main() {
    std::cout << "==========================================\n";
    std::cout << "   DBMS TEAM 404 - OPTIMIZER TEST SUITE   \n";
    std::cout << "==========================================\n\n";

    // 1. Boot up the mock catalog
    MockCatalog catalog;

    try {
        // 2. Fetch our mock tables
        TableStats users = catalog.getTableStats("users");
        TableStats orders = catalog.getTableStats("orders");

        std::cout << "[SYSTEM] Loaded statistics for 'users' and 'orders' tables.\n\n";

        // 3. Test the Scan Costs
        std::cout << "--- SCAN Node Costs ---\n";
        double seq_cost = CostModel::costSequentialScan(users);
        double idx_cost = CostModel::costIndexScan(users, 3); // Assuming B-Tree height of 3

        std::cout << "Sequential Scan Cost: " << std::fixed << std::setprecision(2) << seq_cost << "\n";
        std::cout << "Index Scan Cost:      " << idx_cost << "\n";
        
        if (idx_cost < seq_cost) {
            std::cout << "Result: Optimizer correctly prefers Index Scan.\n\n";
        }

        // 4. Test the Join Costs
        std::cout << "--- JOIN Node Costs (Users x Orders) ---\n";
        double nlj_cost = CostModel::costNestedLoopJoin(users, orders);
        double hash_cost = CostModel::costHashJoin(users, orders);

        std::cout << "Nested Loop Join Cost: " << nlj_cost << "\n";
        std::cout << "Hash Join Cost:        " << hash_cost << "\n";

        if (hash_cost < nlj_cost) {
            std::cout << "Result: Optimizer correctly prefers Hash Join for massive tables.\n\n";
        } else {
            std::cout << "Result: Optimizer prefers Nested Loop Join.\n\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << '\n';
    }

    std::cout << "==========================================\n";
    std::cout << "             TESTS COMPLETED              \n";
    std::cout << "==========================================\n";

    return 0;
}