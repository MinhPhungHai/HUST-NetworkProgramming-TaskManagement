#pragma once

#include <string>
#include <vector>
#include <memory>
#include <libpq-fe.h>
#include <stdexcept>
#include <iostream>

class DatabaseHandler {
private:
    PGconn* conn;
    std::string connection_string;

    void checkConnection() {
        if (PQstatus(conn) != CONNECTION_OK) {
            throw std::runtime_error("Database connection lost: " + std::string(PQerrorMessage(conn)));
        }
    }

public:
    DatabaseHandler(const std::string& dbname = "netprog_db",
                   const std::string& user = "netprog_user",
                   const std::string& password = "netprog_password",
                   const std::string& host = "localhost",
                   const std::string& port = "5432") {

        connection_string = "host=" + host +
                          " port=" + port +
                          " dbname=" + dbname +
                          " user=" + user +
                          " password=" + password;

        conn = PQconnectdb(connection_string.c_str());

        if (PQstatus(conn) != CONNECTION_OK) {
            std::string error = PQerrorMessage(conn);
            PQfinish(conn);
            throw std::runtime_error("Failed to connect to database: " + error);
        }

        // Set schema to netprog
        PGresult* res = PQexec(conn, "SET search_path TO netprog;");
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::string error = PQerrorMessage(conn);
            PQclear(res);
            throw std::runtime_error("Failed to set schema: " + error);
        }
        PQclear(res);

        std::cout << "Database connected successfully" << std::endl;
    }

    ~DatabaseHandler() {
        if (conn) {
            PQfinish(conn);
        }
    }

    // Execute query and return result
    PGresult* executeQuery(const std::string& query) {
        checkConnection();
        PGresult* res = PQexec(conn, query.c_str());

        if (!res) {
            throw std::runtime_error("Query execution failed: null result");
        }

        ExecStatusType status = PQresultStatus(res);
        if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
            std::string error = PQerrorMessage(conn);
            PQclear(res);
            throw std::runtime_error("Query failed: " + error);
        }

        return res;
    }

    // Execute parameterized query
    PGresult* executeParams(const std::string& query,
                           const std::vector<std::string>& params) {
        checkConnection();

        std::vector<const char*> param_values;
        for (const auto& param : params) {
            param_values.push_back(param.c_str());
        }

        PGresult* res = PQexecParams(conn,
                                    query.c_str(),
                                    params.size(),
                                    nullptr,
                                    param_values.data(),
                                    nullptr,
                                    nullptr,
                                    0);

        if (!res) {
            throw std::runtime_error("Query execution failed: null result");
        }

        ExecStatusType status = PQresultStatus(res);
        if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
            std::string error = PQerrorMessage(conn);
            PQclear(res);
            throw std::runtime_error("Query failed: " + error);
        }

        return res;
    }

    // Helper to escape strings for SQL
    std::string escapeString(const std::string& input) {
        char* escaped = PQescapeLiteral(conn, input.c_str(), input.length());
        if (!escaped) {
            throw std::runtime_error("Failed to escape string");
        }
        std::string result(escaped);
        PQfreemem(escaped);
        return result;
    }

    // Generate UUID
    std::string generateUUID() {
        PGresult* res = executeQuery("SELECT gen_random_uuid()::text");
        std::string uuid = PQgetvalue(res, 0, 0);
        PQclear(res);
        return uuid;
    }
};
