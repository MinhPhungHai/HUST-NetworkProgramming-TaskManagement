#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "protocol.h"

class MessageLogger {
private:
    std::ofstream log_file;
    std::mutex log_mutex;
    std::string log_path;
    bool is_server;

    std::string getCurrentTimestamp() {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

public:
    MessageLogger(const std::string& filename, bool server_mode = false)
        : log_path(filename), is_server(server_mode) {
        log_file.open(log_path, std::ios::app);
        if (!log_file.is_open()) {
            throw std::runtime_error("Failed to open log file: " + log_path);
        }

        std::lock_guard<std::mutex> lock(log_mutex);
        log_file << "\n=== " << (is_server ? "SERVER" : "CLIENT")
                 << " LOG STARTED at " << getCurrentTimestamp() << " ===\n" << std::endl;
        log_file.flush();
    }

    ~MessageLogger() {
        if (log_file.is_open()) {
            std::lock_guard<std::mutex> lock(log_mutex);
            log_file << "\n=== " << (is_server ? "SERVER" : "CLIENT")
                     << " LOG ENDED at " << getCurrentTimestamp() << " ===\n" << std::endl;
            log_file.close();
        }
    }

    void logMessage(const std::string& direction, MessageType type,
                   const std::string& payload, const std::string& peer_info = "") {
        std::lock_guard<std::mutex> lock(log_mutex);

        log_file << "[" << getCurrentTimestamp() << "] "
                 << direction << " " << getMessageTypeName(type);

        if (!peer_info.empty()) {
            log_file << " " << peer_info;
        }

        log_file << "\nPayload: " << payload << "\n"
                 << std::string(80, '-') << std::endl;
        log_file.flush();
    }

    void logSent(MessageType type, const std::string& payload, const std::string& to = "") {
        logMessage("SENT", type, payload, to.empty() ? "" : "to " + to);
    }

    void logReceived(MessageType type, const std::string& payload, const std::string& from = "") {
        logMessage("RECEIVED", type, payload, from.empty() ? "" : "from " + from);
    }

    void logError(const std::string& error_msg) {
        std::lock_guard<std::mutex> lock(log_mutex);
        log_file << "[" << getCurrentTimestamp() << "] ERROR: "
                 << error_msg << std::endl;
        log_file.flush();
    }

    void logInfo(const std::string& info_msg) {
        std::lock_guard<std::mutex> lock(log_mutex);
        log_file << "[" << getCurrentTimestamp() << "] INFO: "
                 << info_msg << std::endl;
        log_file.flush();
    }
};
