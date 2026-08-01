# SimpleQueryOptimiser

A SQL query optimizer built from scratch in C++17 as a DBMS course project at IIT Kharagpur. Takes raw SQL, parses it into a logical plan, applies rule-based and cost-based optimizations, and outputs a physical execution plan with concrete join algorithm selections — mirroring the optimizer pipeline inside production databases like PostgreSQL.

## Project Objective

Implement the query optimization layer of a relational DBMS: given a SQL SELECT query, produce an efficient physical execution plan by applying heuristic rewrites (predicate pushdown) and cost-based join ordering (dynamic programming over join subsets with a disk/CPU/memory cost model).

## What We Built

### Parsing — SQL to Structured AST

The query string is tokenized with Flex and parsed with Bison into a JSON-based AST using nlohmann/json. The grammar supports SELECT with projections, multi-table JOINs (INNER, LEFT, CROSS, FULL), WHERE with nested AND/OR predicates, GROUP BY with HAVING, table aliases, and qualified column references (e.g., `u.age`). Using JSON as the intermediate representation decouples the C-based Flex/Bison layer from the C++17 optimizer pipeline, and also serves as the wire format for the web dashboard.

### Semantic Validation — Catalog-Backed Type Checking

Before optimization, a semantic analyzer walks the plan tree bottom-up and validates every table and column reference against a catalog. Tables are checked for existence, columns are checked against their owning table, and aliases are resolved (so `u.id` correctly maps to `users.id`). The catalog is accessed through an `ICatalog` interface with two backends: a `MockCatalog` that loads schema and statistics from `src/catalog.json`, and a `PostgresCatalog` that queries a live Postgres instance via `pg_class` and `pg_attribute` for real statistics.

### Rule-Based Optimization — Predicate Pushdown

The RBO pass pushes WHERE predicates as close to leaf scans as possible, reducing intermediate result sizes before expensive joins. Conjunctive predicates (`AND`) are first split into independent filters that can be pushed separately. Three cases are handled:

- **Qualified single-table predicates** (`u.age > 18`): pushed directly above the referenced table's scan using the alias.
- **Unqualified predicates** (`age > 18`): the catalog is queried to resolve which table owns the column, then pushed accordingly.
- **Cross-table predicates** (`u1.age > u2.age`): pushed to the lowest common ancestor (LCA) of all referenced tables in the join tree — the earliest point where all required columns are available.

### Cost-Based Optimization — DP Join Enumeration

The CBO uses Selinger-style bottom-up dynamic programming over bitmask-encoded table subsets to find the optimal join order. For n tables, it evaluates all 2^n − 1 non-empty subsets, trying every binary partition of each subset and selecting the cheapest join. A prune budget skips partitions whose sub-plan costs already exceed a threshold.

At each join candidate, the optimizer picks between Hash Join and Nested Loop Join using a cost model with five tunable constants: sequential and random I/O page costs, CPU tuple processing cost, CPU predicate evaluation cost, and hash table memory write cost. The formulas mirror PostgreSQL's approach — NLJ costs O(outer × inner) in I/O (re-reading the inner table for each outer tuple), while Hash Join costs O(outer + inner) with an additional memory cost for hash table construction. Cross-table filters from the RBO pass are re-injected at the exact DP subset where all their required tables first converge.

### Web Dashboard

A Flask server bridges the C++ engine and a D3.js frontend. Users enter a SQL query, and the dashboard renders three plan trees side by side — the raw logical plan, the RBO-optimized plan, and the final CBO physical plan — along with the computed cost.

## How to Run

### Prerequisites

```bash
sudo apt update
sudo apt install -y flex bison g++ make python3 python3-flask
```

### Build

```bash
make            # builds the optimizer_test binary
make rebuild    # clean + rebuild
```

### Run via CLI

```bash
./optimizer_test test_queries/test1_simple_scan.sql

# run all tests
for f in test_queries/*.sql; do
    echo "=============================="
    echo "Running: $f"
    echo "=============================="
    ./optimizer_test "$f"
done
```

### Run via Dashboard

```bash
python3 app.py
# open http://localhost:8080
```

### Live Postgres Mode

```bash
./optimizer_test test_queries/test3_three_table_join.sql --live
```

This connects to a local PostgreSQL instance and pulls real tuple/block statistics from `pg_class` instead of using the mock catalog.

## Test Cases

| Test | What It Exercises |
|------|-------------------|
| test1 | Simple full-table scan — no joins, no filters. Plan is just PROJECT → GET. |
| test2 | Single filter pushdown — `age > 18` is pushed below the join, Hash Join selected. |
| test3 | Three-table join — DP explores 7 subsets, joins the smallest table (roles, 50 rows) first. |
| test4 | GROUP BY with HAVING — aggregate node wraps the join subtree. |
| test5 | LEFT JOIN — preserved in the physical plan (Hash Join respects join type). |
| test6 | Bad table name — semantic analyzer catches `invoices` not in catalog. |
| test7 | Bad column — semantic analyzer catches `salary` not in any active table. |
| test8 | Conjunctive filter split — `age > 18 AND amount > 500` split and pushed to different tables. |
| test9 | Four-table join with alias — DP explores 15 subsets, filter on `role_name` pushed to roles scan. |
| test10 | Four tables with self-join + cross-table filter — `u1.age > u2.age` placed at LCA during DP. |
| test11 | Five tables, self-join on both users and orders — stress test for DP enumeration and cross-filter injection. |

## Project Structure
```
├── include/
│ ├── ast_nodes.hpp # Plan node class hierarchy
│ ├── expressions.hpp # Expression tree for predicates
│ ├── catalog.hpp # ICatalog interface, MockCatalog, PostgresCatalog
│ ├── semantic_analyzer.hpp # Pre-optimization validation
│ ├── optimizer.hpp # Rule-based optimizer (filter pushdown)
│ ├── cost_model.hpp # I/O + CPU + memory cost formulas
│ ├── cost_based_optimizer.hpp # DP join enumerator
│ └── nlohmann/json.hpp # Third-party JSON library (vendored)
├── src/
│ ├── lexer.l # Flex tokenizer
│ ├── parser.y # Bison grammar → JSON AST
│ ├── main.cpp # CLI driver + JSON serializer for dashboard
│ └── catalog.json # Mock schema with table statistics
├── templates/
│ └── dashboard.html # D3.js visualization frontend
├── test_queries/ # SQL test files
├── app.py # Flask server
└── makefile
```

## Team

Built by DBMS Team 404, IIT Kharagpur (Spring 2026).