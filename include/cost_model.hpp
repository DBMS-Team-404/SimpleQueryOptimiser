#pragma once

#include "catalog.hpp"

// =============================================================
// COST CONSTANTS
// =============================================================

// Disk I/O
const double SEQ_PAGE_COST    = 1.0;   // sequential block fetch (fastest)
const double RANDOM_PAGE_COST = 4.0;   // random block fetch (4x slower: disk seek)

// CPU
const double CPU_TUPLE_COST    = 0.01;   // cost to read one tuple into memory
const double CPU_OPERATOR_COST = 0.0025; // cost to evaluate one predicate / comparison

// Memory
const double MEM_WRITE_COST = 0.005;   // cost to insert one tuple into a hash table in RAM

// =============================================================
// COST MODEL
// All formulas are static — just pass in TableStats.
// =============================================================
class CostModel {
public:

    // -----------------------------------------------------------
    // Sequential Scan
    // Read every block once, then CPU processes every tuple.
    // Best case for full table reads.
    // -----------------------------------------------------------
    static double costSequentialScan(const TableStats& stats) {
        double io_cost  = stats.num_blocks * SEQ_PAGE_COST;
        double cpu_cost = stats.num_tuples * CPU_TUPLE_COST;
        return io_cost + cpu_cost;
    }

    // -----------------------------------------------------------
    // Index Scan (B-Tree assumed)
    // Random I/O to traverse tree_height levels, then one random
    // fetch for the matching block.
    // -----------------------------------------------------------
    static double costIndexScan(const TableStats& stats, int tree_height = 3) {
        double io_cost  = (tree_height + 1) * RANDOM_PAGE_COST;
        double cpu_cost = CPU_TUPLE_COST; // just the one matching tuple
        return io_cost + cpu_cost;
        (void)stats; // stats used for cardinality estimation elsewhere
    }

    // -----------------------------------------------------------
    // Nested Loop Join
    // Read outer once. For EVERY outer tuple, re-read ALL of inner.
    // O(outer * inner) — only good when outer is tiny.
    // -----------------------------------------------------------
    static double costNestedLoopJoin(const TableStats& outer, const TableStats& inner) {
        // I/O: outer read once + inner read once per outer tuple
        double io_cost  = (outer.num_blocks * SEQ_PAGE_COST) +
                          (outer.num_tuples * inner.num_blocks * SEQ_PAGE_COST);
        // CPU: every outer tuple compared against every inner tuple
        double cpu_cost = outer.num_tuples * inner.num_tuples * CPU_OPERATOR_COST;
        return io_cost + cpu_cost;
    }

    // -----------------------------------------------------------
    // Hash Join
    // Read both tables exactly once.
    // Build a hash table from the smaller (inner) side in RAM.
    // Probe with the outer side.
    // O(outer + inner) — almost always better than NLJ for large tables.
    // -----------------------------------------------------------
    static double costHashJoin(const TableStats& outer, const TableStats& inner) {
        // I/O: both tables read exactly once
        double io_cost  = (outer.num_blocks + inner.num_blocks) * SEQ_PAGE_COST;
        // Memory: write inner into hash table
        double mem_cost = inner.num_tuples * MEM_WRITE_COST;
        // CPU: read all tuples + hash/probe cost on outer
        double cpu_cost = (outer.num_tuples + inner.num_tuples) * CPU_TUPLE_COST +
                          (outer.num_tuples * CPU_OPERATOR_COST);
        return io_cost + mem_cost + cpu_cost;
    }

    // -----------------------------------------------------------
    // costJoin — convenience wrapper used by the DP enumerator.
    // Returns the cost of the CHEAPER of Hash Join and NLJ,
    // automatically putting the smaller table on the correct side.
    //
    // Also handles the "smaller table goes to inner" rule:
    //   Hash Join  → inner = smaller table (less RAM usage)
    //   NLJ        → outer = smaller table (fewer inner-table re-reads)
    // -----------------------------------------------------------
    static double costJoin(const TableStats& left, const TableStats& right) {
        // For Hash Join: put the smaller table as inner (build side)
        const TableStats& hj_outer = (left.num_tuples <= right.num_tuples) ? right : left;
        const TableStats& hj_inner = (left.num_tuples <= right.num_tuples) ? left  : right;

        // For NLJ: put the smaller table as outer (loop driver)
        const TableStats& nlj_outer = (left.num_tuples <= right.num_tuples) ? left  : right;
        const TableStats& nlj_inner = (left.num_tuples <= right.num_tuples) ? right : left;

        double hash_cost = costHashJoin(hj_outer, hj_inner);
        double nlj_cost  = costNestedLoopJoin(nlj_outer, nlj_inner);

        return std::min(hash_cost, nlj_cost);
    }
};