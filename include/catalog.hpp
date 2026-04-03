#pragma once

#include <unordered_map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <array>
#include <memory>
#include <sstream>

struct TableStats {
    size_t num_tuples;
    size_t num_blocks;
};

// 2. The Interface (Abstract Base Class)
class ICatalog {
public:
    virtual ~ICatalog() = default;
    virtual TableStats getTableStats(const std::string& table_name) const = 0;
};

// 3. The Fake Catalog (For fast, offline team testing)
class MockCatalog : public ICatalog {
private:
    std::unordered_map<std::string, TableStats> table_statistics;

public:
    MockCatalog() {
        table_statistics["users"] = {10000, 100};       
        table_statistics["orders"] = {5000000, 25000};  
    }

    TableStats getTableStats(const std::string& table_name) const override {
        auto it = table_statistics.find(table_name);
        if (it != table_statistics.end()) {
            return it->second;
        }
        throw std::invalid_argument("Table not found in Mock Catalog: " + table_name);
    }
};



// 4. The Real PostgreSQL Catalog (Uses the terminal to query live data)
class PostgresCatalog : public ICatalog {
private:
    std::string connection_uri;

public:
    // 1.Constructor: Feed it all your PSQL details
    PostgresCatalog(const std::string& host, const std::string& port, const std::string& db_name, 
                    const std::string& user, const std::string& password)
    {
        // Build the official Postgres Connection URI
        connection_uri = "postgresql://" + user + ":" + password + "@" + host + ":" + port + "/" + db_name;
    }

    TableStats getTableStats(const std::string& table_name) const override {
        // 2. Use the URI in the terminal command
        // Notice we wrap the URI in quotes so the terminal doesn't misinterpret special characters in your password
        std::string command = "psql \"" + connection_uri + "\" -q -t -c \"ANALYZE " + table_name + "; SELECT reltuples, relpages FROM pg_class WHERE relname='" + table_name + "';\"";        
        std::string result = "";
        std::array<char, 128> buffer;
        
        // Open the invisible terminal and run it
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed! Cannot run psql.");
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        TableStats stats = {0, 0};
        std::stringstream ss(result);
        if (ss >> stats.num_tuples >> stats.num_blocks) {
            if (stats.num_tuples == static_cast<size_t>(-1)) {
                 std::cerr << "[WARNING] Table '" << table_name << "' has not been ANALYZED in Postgres.\n";
            }
            return stats;
        }

        throw std::invalid_argument("Table not found or authentication failed for: " + table_name);
    }
};