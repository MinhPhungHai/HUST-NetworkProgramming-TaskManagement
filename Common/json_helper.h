#pragma once

#include <string>
#include <sstream>
#include <map>
#include <vector>

// Simple JSON builder and parser for our protocol
// This is a lightweight implementation - for production, consider using nlohmann/json or similar

class JsonBuilder {
private:
    std::ostringstream json;
    bool first_field;

    void escapeString(const std::string& str, std::ostringstream& out) {
        for (char c : str) {
            switch (c) {
                case '"': out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\b': out << "\\b"; break;
                case '\f': out << "\\f"; break;
                case '\n': out << "\\n"; break;
                case '\r': out << "\\r"; break;
                case '\t': out << "\\t"; break;
                default: out << c; break;
            }
        }
    }

public:
    JsonBuilder() : first_field(true) {
        json << "{";
    }

    JsonBuilder& add(const std::string& key, const std::string& value) {
        if (!first_field) json << ",";
        json << "\"" << key << "\":\"";
        escapeString(value, json);
        json << "\"";
        first_field = false;
        return *this;
    }

    JsonBuilder& add(const std::string& key, int value) {
        if (!first_field) json << ",";
        json << "\"" << key << "\":" << value;
        first_field = false;
        return *this;
    }

    JsonBuilder& add(const std::string& key, bool value) {
        if (!first_field) json << ",";
        json << "\"" << key << "\":" << (value ? "true" : "false");
        first_field = false;
        return *this;
    }

    JsonBuilder& addArray(const std::string& key, const std::vector<std::string>& values) {
        if (!first_field) json << ",";
        json << "\"" << key << "\":[";
        for (size_t i = 0; i < values.size(); i++) {
            if (i > 0) json << ",";
            json << "\"";
            escapeString(values[i], json);
            json << "\"";
        }
        json << "]";
        first_field = false;
        return *this;
    }

    JsonBuilder& addRaw(const std::string& key, const std::string& raw_json) {
        if (!first_field) json << ",";
        json << "\"" << key << "\":" << raw_json;
        first_field = false;
        return *this;
    }

    std::string build() {
        return json.str() + "}";
    }
};

// Simple JSON parser - extracts values by key
class JsonParser {
private:
    std::string json_str;

    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    std::string unescapeString(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == '\\' && i + 1 < str.length()) {
                switch (str[i + 1]) {
                    case '"': result += '"'; i++; break;
                    case '\\': result += '\\'; i++; break;
                    case 'b': result += '\b'; i++; break;
                    case 'f': result += '\f'; i++; break;
                    case 'n': result += '\n'; i++; break;
                    case 'r': result += '\r'; i++; break;
                    case 't': result += '\t'; i++; break;
                    default: result += str[i]; break;
                }
            } else {
                result += str[i];
            }
        }
        return result;
    }

public:
    JsonParser(const std::string& json) : json_str(json) {}

    std::string getString(const std::string& key, const std::string& default_value = "") {
        std::string search = "\"" + key + "\"";
        size_t pos = json_str.find(search);
        if (pos == std::string::npos) return default_value;

        size_t colon = json_str.find(':', pos);
        if (colon == std::string::npos) return default_value;

        size_t quote_start = json_str.find('"', colon);
        if (quote_start == std::string::npos) return default_value;

        size_t quote_end = quote_start + 1;
        while (quote_end < json_str.length()) {
            if (json_str[quote_end] == '"' && json_str[quote_end - 1] != '\\') {
                break;
            }
            quote_end++;
        }

        if (quote_end >= json_str.length()) return default_value;

        std::string value = json_str.substr(quote_start + 1, quote_end - quote_start - 1);
        return unescapeString(value);
    }

    int getInt(const std::string& key, int default_value = 0) {
        std::string search = "\"" + key + "\"";
        size_t pos = json_str.find(search);
        if (pos == std::string::npos) return default_value;

        size_t colon = json_str.find(':', pos);
        if (colon == std::string::npos) return default_value;

        size_t num_start = colon + 1;
        while (num_start < json_str.length() && isspace(json_str[num_start])) {
            num_start++;
        }

        size_t num_end = num_start;
        while (num_end < json_str.length() &&
               (isdigit(json_str[num_end]) || json_str[num_end] == '-')) {
            num_end++;
        }

        if (num_end == num_start) return default_value;

        std::string num_str = json_str.substr(num_start, num_end - num_start);
        try {
            return std::stoi(num_str);
        } catch (...) {
            return default_value;
        }
    }

    bool getBool(const std::string& key, bool default_value = false) {
        std::string search = "\"" + key + "\"";
        size_t pos = json_str.find(search);
        if (pos == std::string::npos) return default_value;

        size_t colon = json_str.find(':', pos);
        if (colon == std::string::npos) return default_value;

        size_t true_pos = json_str.find("true", colon);
        size_t false_pos = json_str.find("false", colon);

        if (true_pos != std::string::npos &&
            (false_pos == std::string::npos || true_pos < false_pos)) {
            return true;
        }
        if (false_pos != std::string::npos) {
            return false;
        }

        return default_value;
    }

    bool has(const std::string& key) {
        std::string search = "\"" + key + "\"";
        return json_str.find(search) != std::string::npos;
    }
};
