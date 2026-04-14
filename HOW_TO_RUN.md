🚀 DBMSTeam 404 — Query Optimizer

A C++-based SQL Query Optimizer with support for:

Logical plan generation
Rule-Based Optimization (RBO)
Cost-Based Optimization (CBO) using Dynamic Programming
Interactive Web Dashboard (Flask + D3.js)

📦 Features
🔍 SQL Parsing (Flex + Bison)
🌳 Logical Query Plan Generation
⚙️ Rule-Based Optimization (Filter Pushdown, etc.)
📊 Cost-Based Optimization (Join Ordering via DP)
🌐 Web UI with interactive tree visualization
💻 CLI support for batch testing

🛠️ 1. Prerequisites

Run this once in WSL:

# Update package list
sudo apt update

# Install C++ build tools
sudo apt install -y flex bison g++ make

# Install Python and Flask (for dashboard)
sudo apt install -y python3 python3-flask

⚠️ Using apt for Flask avoids the "externally managed environment" pip issue.

⚙️ 2. Build the C++ Engine

From the project root (where the Makefile exists):

make

If you encounter errors or change code:

make rebuild

✅ Successful build:

No output (silent success)
Generates: optimizer_test binary

🌐 3. Run the Web Dashboard (Recommended)

Start the Flask server:

python3 app.py

Open browser:

http://localhost:8080
Usage:
Enter SQL query in editor
Click Run Optimizer
View:
Logical Plan
RBO Plan
Final CBO Plan (with join strategies)
💻 4. Run via Terminal (CLI)
Run individual test:
./optimizer_test test_queries/test1_simple_scan.sql
./optimizer_test test_queries/test2_filter_pushdown.sql
./optimizer_test test_queries/test3_three_table_join.sql
Run all tests:
for f in test_queries/*.sql; do
    echo ""
    echo "=============================="
    echo "Running: $f"
    echo "=============================="
    ./optimizer_test "$f"
done
📊 5. Understanding DP Output (CBO)

Example:

[DP] Found 3 table(s) to enumerate.
[DP] Subset 0b00000011 new best cost: ...
[DP] Subset 0b00000101 new best cost: ...
[DP] Subset 0b00000110 new best cost: ...
[DP] Subset 0b00000111 new best cost: ...
[DP] Best join plan cost: XXXX
Key Idea:
Each subset = combination of tables
Final subset (0b111) = all tables
Lowest cost = optimal join order

🧪 6. Expected Test Results
| Test  | Expected Behavior                                  |
| ----- | -------------------------------------------------- |
| test1 | Simple scan: `PROJECT → GET(users)`                |
| test2 | Filter pushed below join; Hash Join selected       |
| test3 | DP explores 7 subsets; smallest table joined first |
| test4 | Aggregate node wraps join                          |
| test5 | LEFT join appears in physical plan                 |
| test6 | ❌ Table does not exist                             |
| test7 | ❌ Column not found                                 |
| test8 | Filters pushed independently                       |

⚠️ 7. Common Errors & Fixes
❌ Port 8080 already in use

Fix:

Ctrl + C

Stop existing server before restarting.
❌ parser.tab.h not found

Cause: Bison not installed
Fix:

which bison
❌ 'json.hpp' file not found

Fix:

Ensure file exists at:
include/nlohmann/json.hpp
Download from official repo if missing
❌ SQL parsing errors

Fix:

Ensure lexer has:
%option case-insensitive
❌ Table not found

Fix:

Check:
src/catalog.json
Query table names (case-sensitive)
📁 Project Structure (Suggested)
.
├── src/
├── include/
│   └── nlohmann/json.hpp
├── test_queries/
├── app.py
├── Makefile
├── optimizer_test
└── README.md
🤝 Contributing
Fork the repo
Create a feature branch
Submit a PR
🧠 Notes
RBO improves logical plan using heuristics
CBO uses DP for optimal join ordering
Visualization helps debug execution strategies
📜 License
