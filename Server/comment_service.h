#pragma once

#include "database_handler.h"
#include "../Common/protocol.h"
#include <sstream>

class CommentService {
private:
    DatabaseHandler& db;

    StatusCode checkTaskAccess(const std::string& task_id, const std::string& user_id, std::string& project_id_out) {
        std::string query = "SELECT project_id FROM tasks WHERE task_id = " + db.escapeString(task_id);
        PGresult* res = db.executeQuery(query);

        if (PQntuples(res) == 0) {
            PQclear(res);
            return STATUS_NOT_FOUND;
        }

        project_id_out = PQgetvalue(res, 0, 0);
        PQclear(res);

        query = "SELECT member_id FROM project_members WHERE project_id = " +
               db.escapeString(project_id_out) + " AND user_id = " + db.escapeString(user_id);
        res = db.executeQuery(query);

        if (PQntuples(res) == 0) {
            PQclear(res);
            return STATUS_UNAUTHORIZED;
        }
        PQclear(res);

        return STATUS_SUCCESS;
    }

public:
    CommentService(DatabaseHandler& database) : db(database) {}

    StatusCode getTaskComments(const std::string& task_id, const std::string& user_id, std::string& comments_json_out) {
        try {
            std::string project_id;
            StatusCode access = checkTaskAccess(task_id, user_id, project_id);
            if (access != STATUS_SUCCESS) {
                return access;
            }

            std::ostringstream query;
            query << "SELECT c.comment_id, c.user_id, u.username, c.comment_content, c.created_at "
                  << "FROM task_comments c "
                  << "JOIN users u ON c.user_id = u.user_id "
                  << "WHERE c.task_id = " << db.escapeString(task_id)
                  << " ORDER BY c.created_at ASC";

            PGresult* res = db.executeQuery(query.str());
            int rows = PQntuples(res);

            std::ostringstream json;
            json << "[";

            for (int i = 0; i < rows; i++) {
                if (i > 0) json << ",";
                json << "{";
                json << "\"comment_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"user_id\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"username\":\"" << PQgetvalue(res, i, 2) << "\",";
                json << "\"content\":\"" << PQgetvalue(res, i, 3) << "\",";
                json << "\"created_at\":\"" << PQgetvalue(res, i, 4) << "\"";
                json << "}";
            }

            json << "]";
            PQclear(res);

            comments_json_out = json.str();
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Get task comments error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    StatusCode addTaskComment(const std::string& task_id, const std::string& user_id,
                              const std::string& content, std::string& comment_id_out) {
        try {
            std::string project_id;
            StatusCode access = checkTaskAccess(task_id, user_id, project_id);
            if (access != STATUS_SUCCESS) {
                return access;
            }

            std::string comment_id = db.generateUUID();
            std::string query =
                "INSERT INTO task_comments (comment_id, task_id, user_id, comment_content, created_at) "
                "VALUES (" + db.escapeString(comment_id) + ", " +
                db.escapeString(task_id) + ", " +
                db.escapeString(user_id) + ", " +
                db.escapeString(content) + ", NOW())";

            db.executeQuery(query);
            comment_id_out = comment_id;
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Add task comment error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }
};
