# How to Build and Run — WSL Guide

## Prerequisites (run once)

```bash
sudo apt update
sudo apt install -y flex bison g++ make
```

---

## Build

```bash
# From the project root (where makefile lives)
make

# If you hit errors from a previous broken build, clean first
make rebuild
```

A successful build prints nothing and produces the `optimizer_test` binary in the project root.

---

## Run a single query

```bash
./optimizer_test test_queries/test1_simple_scan.sql
./optimizer_test test_queries/test2_filter_pushdown.sql
./optimizer_test test_queries/test3_three_table_join.sql
```

---

## Run all tests in one shot

```bash
for f in test_queries/*.sql; do
    echo ""
    echo "=============================="
    echo "Running: $f"
    echo "=============================="
    ./optimizer_test "$f"
done
```

---

## Run with your catalog.json (if MockCatalog loads from file)

Make sure `catalog.json` is in `src/` as shown in your file tree.
The binary picks it up at runtime — no recompile needed when you
change table stats.

---

## What each test should print

| Test | Expected outcome |
|------|-----------------|
| test1 | PROJECT -> GET(users). No join, no filter. |
| test2 | RBO pushes FILTER(age>18) below JOIN onto GET(users). CBO picks Hash Join. |
| test3 | DP explores 7 subsets. Best order should join roles(50 tuples) first. |
| test4 | AGGREGATE node wraps the join core in the final plan. |
| test5 | Physical node says LEFT not INNER. |
| test6 | "[FAILED] Semantic Error: Table does not exist: invoices" |
| test7 | "[FAILED] Semantic Error: Column not found: salary" |
| test8 | Both filters pushed down independently to correct tables. |

---

## Reading the DP output (test3 is the most interesting)

```
[DP] Found 3 table(s) to enumerate.
[DP] Subset 0b00000011 new best cost: ...   <- users+orders
[DP] Subset 0b00000101 new best cost: ...   <- users+roles
[DP] Subset 0b00000110 new best cost: ...   <- orders+roles
[DP] Subset 0b00000111 new best cost: ...   <- all three
[DP] Best join plan cost: XXXX
```

The subset with the lowest cost at `0b00000111` (all 3 bits set) is your winner.
The final plan tree printed below it shows the physical join order the DP chose.

---

## Common errors and fixes

**`parser.tab.h: No such file`**
Bison didn't run. Check that `bison` is installed: `which bison`

**`'json.hpp' file not found`**
The nlohmann header must be at `include/nlohmann/json.hpp`.
Download it: https://github.com/nlohmann/json/releases/latest → single-header

**`Unexpected character` during parse**
Your SQL has lowercase keywords and the lexer is case-sensitive.
Add `%option case-insensitive` in `lexer.l` under `%option yylineno`.

**`Table not found in Mock Catalog`**
The table name in your SQL doesn't match what's in `catalog.json` or
the hardcoded defaults. Check spelling — it's case-sensitive.
