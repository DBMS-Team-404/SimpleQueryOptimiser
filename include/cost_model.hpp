#pragma once

#include "catalog.hpp"

// 1. Disk I/O Costs
const double SEQ_PAGE_COST    = 1.0;  // Fetching a block sequentially (fastest disk operation)
const double RANDOM_PAGE_COST = 4.0;  // Fetching a block randomly (4x slower due to disk seek time)

// 2. CPU Processing Costs
const double CPU_TUPLE_COST    = 0.01;   // CPU cost to just read/fetch a single row into memory
const double CPU_OPERATOR_COST = 0.0025; // CPU cost to execute a WHERE clause or Join comparison

// 3. Memory Costs
const double MEM_WRITE_COST    = 0.005;  // Cost to write a tuple into a Hash Table in RAM


class CostModel {
public:
    // -------------------------------------------------------------------
    // Sequential Scan
    // Reads every block sequentially, then CPU processes every tuple.
    // -------------------------------------------------------------------
    static double costSequentialScan(const TableStats& stats) {
        double io_cost = stats.num_blocks * SEQ_PAGE_COST;
        double cpu_cost = stats.num_tuples * CPU_TUPLE_COST;
        
        return io_cost + cpu_cost;
    }

    // -------------------------------------------------------------------
    // Index Scan (Assuming B-Tree)
    // Jumps around the disk (random I/O) to traverse the tree layers, 
    // then does one final random read to fetch the target row.
    // -------------------------------------------------------------------
    static double costIndexScan(const TableStats& stats, int tree_height = 3) {
        // Tree traversal (random disk reads) + fetching the actual tuple block
        double io_cost = (tree_height * RANDOM_PAGE_COST) + (1.0 * RANDOM_PAGE_COST);
        double cpu_cost = 1.0 * CPU_TUPLE_COST;     
        
        return io_cost + cpu_cost;
    }

    // -------------------------------------------------------------------
    // Nested Loop Join
    // Outer table is read sequentially. For EVERY tuple in outer, 
    // the ENTIRE inner table is read sequentially again.
    // -------------------------------------------------------------------
    static double costNestedLoopJoin(const TableStats& outer, const TableStats& inner) {
        // I/O: Read outer once + (read inner completely for every outer row)
        double io_cost = (outer.num_blocks * SEQ_PAGE_COST) + 
                         (outer.num_tuples * inner.num_blocks * SEQ_PAGE_COST);
                         
        // CPU: Compare every outer row against every inner row
        double cpu_cost = (outer.num_tuples * inner.num_tuples * CPU_OPERATOR_COST);
        
        return io_cost + cpu_cost;
    }

    // -------------------------------------------------------------------
    // Hash Join
    // Both tables read sequentially EXACTLY ONCE. 
    // Inner table is written to RAM. CPU compares hashes.
    // -------------------------------------------------------------------
    static double costHashJoin(const TableStats& outer, const TableStats& inner) {
        // I/O: Read both tables exactly once sequentially
        double io_cost = (outer.num_blocks + inner.num_blocks) * SEQ_PAGE_COST;
        
        // Memory: Write the smaller (inner) table into a Hash Table in RAM
        double mem_cost = inner.num_tuples * MEM_WRITE_COST; 
        
        // CPU: Read all tuples, plus the cost to hash and probe them
        double cpu_cost = (outer.num_tuples + inner.num_tuples) * CPU_TUPLE_COST + 
                          (outer.num_tuples * CPU_OPERATOR_COST);
        
        return io_cost + cpu_cost + mem_cost;
    }
};