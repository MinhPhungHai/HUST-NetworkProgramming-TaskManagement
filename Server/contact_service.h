#pragma once

#include "database_handler.h"
#include "../Common/protocol.h"
#include <sstream>

class ContactService {
private:
    DatabaseHandler& db;

public:
    ContactService(DatabaseHandler& database) : db(database) {}

    // Add contact (friend request)
    StatusCode addContact(const std::string& user_id, const std::string& contact_username_or_email,
                         std::string& contact_id_out) {
        try {
            // Find contact user
            std::string query = "SELECT user_id FROM users WHERE username = " +
                              db.escapeString(contact_username_or_email) + " OR email = " +
                              db.escapeString(contact_username_or_email);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_USER_NOT_FOUND;
            }

            std::string contact_user_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            // Can't add yourself
            if (contact_user_id == user_id) {
                return STATUS_INVALID_REQUEST;
            }

            // Check if contact already exists
            query = "SELECT contact_id, status FROM user_contacts WHERE "
                   "(user_id = " + db.escapeString(user_id) + " AND contact_user_id = " +
                   db.escapeString(contact_user_id) + ") OR "
                   "(user_id = " + db.escapeString(contact_user_id) + " AND contact_user_id = " +
                   db.escapeString(user_id) + ")";

            res = db.executeQuery(query);

            if (PQntuples(res) > 0) {
                std::string existing_status = PQgetvalue(res, 0, 1);
                PQclear(res);

                if (existing_status == "accepted") {
                    return STATUS_ERROR; // Already friends
                } else if (existing_status == "pending") {
                    return STATUS_ERROR; // Request already sent
                }
            }
            PQclear(res);

            // Create contact entry
            std::string contact_id = db.generateUUID();
            query = "INSERT INTO user_contacts (contact_id, user_id, contact_user_id, status, created_at) "
                   "VALUES (" + db.escapeString(contact_id) + ", " +
                   db.escapeString(user_id) + ", " +
                   db.escapeString(contact_user_id) + ", 'accepted', NOW())";

            db.executeQuery(query);

            // Create notification for contact
            std::string notif_id = db.generateUUID();
            query = "SELECT username FROM users WHERE user_id = " + db.escapeString(user_id);
            res = db.executeQuery(query);
            std::string username = PQgetvalue(res, 0, 0);
            PQclear(res);

            query = "INSERT INTO notifications (notification_id, user_id, notification_type, message, is_read, created_at) "
                   "VALUES (" + db.escapeString(notif_id) + ", " +
                   db.escapeString(contact_user_id) + ", 'contact_added', " +
                   db.escapeString(username + " added you as a contact") + ", false, NOW())";
            db.executeQuery(query);

            contact_id_out = contact_id;
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Add contact error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Get user's contacts
    std::string getContacts(const std::string& user_id) {
        try {
            std::string query =
                "SELECT c.contact_id, c.contact_user_id, u.username, u.email, c.status "
                "FROM user_contacts c "
                "JOIN users u ON c.contact_user_id = u.user_id "
                "WHERE c.user_id = " + db.escapeString(user_id) + " AND c.status = 'accepted' "
                "UNION "
                "SELECT c.contact_id, c.user_id as contact_user_id, u.username, u.email, c.status "
                "FROM user_contacts c "
                "JOIN users u ON c.user_id = u.user_id "
                "WHERE c.contact_user_id = " + db.escapeString(user_id) + " AND c.status = 'accepted' "
                "ORDER BY username";

            PGresult* res = db.executeQuery(query);
            int rows = PQntuples(res);

            std::ostringstream json;
            json << "[";

            for (int i = 0; i < rows; i++) {
                if (i > 0) json << ",";
                json << "{";
                json << "\"contact_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"user_id\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"username\":\"" << PQgetvalue(res, i, 2) << "\",";
                json << "\"email\":\"" << PQgetvalue(res, i, 3) << "\",";
                json << "\"status\":\"" << PQgetvalue(res, i, 4) << "\"";
                json << "}";
            }

            json << "]";
            PQclear(res);

            return json.str();
        } catch (const std::exception& e) {
            std::cerr << "Get contacts error: " << e.what() << std::endl;
            return "[]";
        }
    }

    // Search users (for adding contacts)
    std::string searchUsers(const std::string& search_term, const std::string& current_user_id, int limit = 20) {
        try {
            std::string search_pattern = "%" + search_term + "%";
            std::ostringstream query;
            query << "SELECT user_id, username, email FROM users "
                  << "WHERE (username ILIKE " << db.escapeString(search_pattern)
                  << " OR email ILIKE " << db.escapeString(search_pattern) << ") "
                  << "AND user_id != " << db.escapeString(current_user_id)
                  << " LIMIT " << limit;

            PGresult* res = db.executeQuery(query.str());
            int rows = PQntuples(res);

            std::ostringstream json;
            json << "[";

            for (int i = 0; i < rows; i++) {
                if (i > 0) json << ",";
                json << "{";
                json << "\"user_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"username\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"email\":\"" << PQgetvalue(res, i, 2) << "\"";
                json << "}";
            }

            json << "]";
            PQclear(res);

            return json.str();
        } catch (const std::exception& e) {
            std::cerr << "Search users error: " << e.what() << std::endl;
            return "[]";
        }
    }
};
