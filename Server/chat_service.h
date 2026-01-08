#pragma once

#include "database_handler.h"
#include "../Common/protocol.h"
#include <sstream>

class ChatService {
private:
    DatabaseHandler& db;

public:
    ChatService(DatabaseHandler& database) : db(database) {}

    // Send chat message
    StatusCode sendMessage(const std::string& project_id, const std::string& sender_id,
                          const std::string& message_content, std::string& chat_id_out) {
        try {
            // Check if user is member of project
            std::string query = "SELECT member_id FROM project_members WHERE project_id = " +
                              db.escapeString(project_id) + " AND user_id = " +
                              db.escapeString(sender_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_UNAUTHORIZED;
            }
            PQclear(res);

            // Generate chat ID
            std::string chat_id = db.generateUUID();

            // Insert chat message
            query = "INSERT INTO project_chat_log (chat_id, project_id, sender_id, message_content, created_at) "
                   "VALUES (" + db.escapeString(chat_id) + ", " +
                   db.escapeString(project_id) + ", " +
                   db.escapeString(sender_id) + ", " +
                   db.escapeString(message_content) + ", NOW())";

            db.executeQuery(query);

            chat_id_out = chat_id;
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Send message error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Get chat history
    std::string getChatHistory(const std::string& project_id, int limit = 100, int offset = 0) {
        try {
            std::ostringstream query;
            query << "SELECT c.chat_id, c.sender_id, u.username, c.message_content, c.created_at "
                  << "FROM project_chat_log c "
                  << "JOIN users u ON c.sender_id = u.user_id "
                  << "WHERE c.project_id = " << db.escapeString(project_id)
                  << " ORDER BY c.created_at DESC "
                  << "LIMIT " << limit << " OFFSET " << offset;

            PGresult* res = db.executeQuery(query.str());
            int rows = PQntuples(res);

            std::ostringstream json;
            json << "[";

            for (int i = rows - 1; i >= 0; i--) { // Reverse order for chronological
                if (i < rows - 1) json << ",";
                json << "{";
                json << "\"chat_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"sender_id\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"sender_name\":\"" << PQgetvalue(res, i, 2) << "\",";
                json << "\"message\":\"" << PQgetvalue(res, i, 3) << "\",";
                json << "\"timestamp\":\"" << PQgetvalue(res, i, 4) << "\"";
                json << "}";
            }

            json << "]";
            PQclear(res);

            return json.str();
        } catch (const std::exception& e) {
            std::cerr << "Get chat history error: " << e.what() << std::endl;
            return "[]";
        }
    }

    // Get recent chat messages (for real-time updates)
    std::string getRecentMessages(const std::string& project_id, const std::string& since_timestamp) {
        try {
            std::string query =
                "SELECT c.chat_id, c.sender_id, u.username, c.message_content, c.created_at "
                "FROM project_chat_log c "
                "JOIN users u ON c.sender_id = u.user_id "
                "WHERE c.project_id = " + db.escapeString(project_id) +
                " AND c.created_at > " + db.escapeString(since_timestamp) +
                " ORDER BY c.created_at ASC";

            PGresult* res = db.executeQuery(query);
            int rows = PQntuples(res);

            std::ostringstream json;
            json << "[";

            for (int i = 0; i < rows; i++) {
                if (i > 0) json << ",";
                json << "{";
                json << "\"chat_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"sender_id\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"sender_name\":\"" << PQgetvalue(res, i, 2) << "\",";
                json << "\"message\":\"" << PQgetvalue(res, i, 3) << "\",";
                json << "\"timestamp\":\"" << PQgetvalue(res, i, 4) << "\"";
                json << "}";
            }

            json << "]";
            PQclear(res);

            return json.str();
        } catch (const std::exception& e) {
            std::cerr << "Get recent messages error: " << e.what() << std::endl;
            return "[]";
        }
    }
};
