#pragma once

#include <unordered_map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <memory>
#include <sstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

struct TableStats {
    size_t num_tuples;
    size_t num_blocks;
};

// INTERFACE
class ICatalog {
public:
    virtual ~ICatalog() = default;
    virtual TableStats getTableStats(const std::string& table_name) const = 0;
    virtual bool columnExists(const std::string& table_name, const std::string& column_name) const = 0;
};

// MOCK CATALOG
// Supports three modes:
//   1. MockCatalog()            — hardcoded defaults (users/orders/roles)
//   2. MockCatalog("file.json") — load schema from a JSON file
//   3. addTable(...)            — add tables programmatically at runtime
class MockCatalog : public ICatalog {
private:
    std::unordered_map<std::string, TableStats>              table_statistics;
    std::unordered_map<std::string, std::vector<std::string>> table_columns;

    void loadDefaults() {
        table_statistics["users"]  = {10000,   100};
        table_statistics["orders"] = {5000000, 25000};
        table_statistics["roles"]  = {50,      1};

        table_columns["users"]  = {"id", "name", "age", "role_id"};
        table_columns["orders"] = {"id", "user_id", "amount", "order_date"};
        table_columns["roles"]  = {"id", "role_name"};
    }

    void loadFromJson(const std::string& file_path) {
        std::ifstream f(file_path);
        if (!f.is_open()) {
            std::cerr << "[MockCatalog] WARNING: Could not open '" << file_path
                      << "'. Falling back to hardcoded defaults.\n";
            loadDefaults();
            return;
        }

        json data;
        try {
            f >> data;
        } catch (const json::parse_error& e) {
            std::cerr << "[MockCatalog] WARNING: JSON parse error in '" << file_path
                      << "': " << e.what() << ". Falling back to hardcoded defaults.\n";
            loadDefaults();
            return;
        }

        if (!data.contains("tables")) {
            std::cerr << "[MockCatalog] WARNING: JSON missing 'tables' key. "
                      << "Falling back to hardcoded defaults.\n";
            loadDefaults();
            return;
        }

        for (auto& [table_name, info] : data["tables"].items()) {
            // stats
            size_t tuples = info.value("num_tuples", 1000);
            size_t blocks = info.value("num_blocks", 10);
            table_statistics[table_name] = {tuples, blocks};

            // columns
            std::vector<std::string> cols;
            if (info.contains("columns") && info["columns"].is_array()) {
                for (const auto& col : info["columns"]) {
                    cols.push_back(col.get<std::string>());
                }
            }
            table_columns[table_name] = cols;
        }

        std::cout << "[MockCatalog] Loaded " << table_statistics.size() << " table(s) from '" << file_path << "'.\n";
    }

public:
    // hardcoded defaults
    MockCatalog() {
        loadDefaults();
    }

    // load from JSON file
    explicit MockCatalog(const std::string& json_file_path) {
        loadFromJson(json_file_path);
    }

    // add a table at runtime (useful in tests / main.cpp)
    void addTable(const std::string& table_name,
                  size_t num_tuples,
                  size_t num_blocks,
                  const std::vector<std::string>& columns)
    {
        table_statistics[table_name] = {num_tuples, num_blocks};
        table_columns[table_name]    = columns;
        std::cout << "[MockCatalog] Added table '" << table_name << "' ("
                  << num_tuples << " tuples, " << num_blocks << " blocks).\n";
    }

    void printSchema() const {
        std::cout << "\n[MockCatalog] Current schema:\n";
        for (const auto& [name, stats] : table_statistics) {
            std::cout << "  " << name
                      << " | tuples: " << stats.num_tuples
                      << " | blocks: " << stats.num_blocks
                      << " | columns: [";
            auto it = table_columns.find(name);
            if (it != table_columns.end()) {
                for (size_t i = 0; i < it->second.size(); ++i) {
                    std::cout << it->second[i];
                    if (i + 1 < it->second.size()) std::cout << ", ";
                }
            }
            std::cout << "]\n";
        }
        std::cout << "\n";
    }

    // ICatalog interface
    TableStats getTableStats(const std::string& table_name) const override {
        auto it = table_statistics.find(table_name);
        if (it != table_statistics.end()) return it->second;
        throw std::invalid_argument("Table not found in Mock Catalog: " + table_name);
    }

    bool columnExists(const std::string& table_name, const std::string& column_name) const override {
        auto it = table_columns.find(table_name);
        if (it != table_columns.end()) {
            for (const auto& col : it->second) {
                if (col == column_name) return true;
            }
        }
        return false;
    }
};

// POSTGRES CATALOG 
class PostgresCatalog : public ICatalog {
private:
    std::string connection_uri;

    // Run a psql query and return trimmed stdout, or "" on failure
    std::string runPsql(const std::string& sql) const {
        // -q  : quiet (suppresses "ANALYZE" confirmation lines)
        // -t  : tuples-only (no headers, no row-count footer)
        // -A  : unaligned output (fields separated by | not spaces)
        std::string command =
            "psql \"" + connection_uri + "\" -q -t -A -c \"" + sql + "\" 2>/dev/null";

        std::string result;
        std::array<char, 256> buffer;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) return "";
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();
        return result;
    }

public:
    PostgresCatalog(const std::string& host, const std::string& port,
                    const std::string& db_name, const std::string& user,
                    const std::string& password)
    {
        connection_uri = "postgresql://" + user + ":" + password +
                         "@" + host + ":" + port + "/" + db_name;
        std::cout << "[PostgresCatalog] URI: " << connection_uri << "\n";
    }

    TableStats getTableStats(const std::string& table_name) const override {
        // ANALYZE separately (its "ANALYZE" echo goes to stderr with 2>/dev/null)
        runPsql("ANALYZE " + table_name + ";");

        // Cast to bigint so we get clean integers, not "10000.0"
        std::string sql =
            "SELECT reltuples::bigint, relpages "
            "FROM pg_class "
            "WHERE relname='" + table_name + "' AND relkind='r';";

        std::string result = runPsql(sql);

        // With -A, output looks like: "10000|64\n"
        TableStats stats = {0, 0};
        char sep;
        std::stringstream ss(result);
        if (ss >> stats.num_tuples >> sep >> stats.num_blocks) {
            return stats;
        }

        // Fallback: try whitespace-separated (in case -A wasn't respected)
        std::stringstream ss2(result);
        if (ss2 >> stats.num_tuples >> stats.num_blocks) {
            return stats;
        }

        throw std::invalid_argument(
            "[PostgresCatalog] Table not found or parse failed for: " + table_name +
            "\n  Raw output was: '" + result + "'");
    }

    bool columnExists(const std::string& table_name, const std::string& column_name) const override {
        // pg_attribute is faster and doesn't need information_schema privileges
        std::string sql =
            "SELECT 1 FROM pg_attribute a "
            "JOIN pg_class c ON c.oid = a.attrelid "
            "WHERE c.relname='" + table_name + "' "
            "AND a.attname='" + column_name + "' "
            "AND a.attnum > 0 AND NOT a.attisdropped;";

        std::string result = runPsql(sql);
        return result.find("1") != std::string::npos;
    }
};