#pragma once

#include "database_handler.h"
#include "../Common/protocol.h"
#include <sstream>

class FileService {
private:
    DatabaseHandler& db;

public:
    FileService(DatabaseHandler& database) : db(database) {}

    // Upload file attachment
    StatusCode uploadFile(const std::string& task_id, const std::string& uploader_id,
                         const std::string& file_name, const std::string& file_path,
                         const std::string& file_type, int file_size,
                         std::string& file_id_out) {
        try {
            // Check if user has access to task
            std::string query = "SELECT t.project_id FROM tasks t WHERE task_id = " +
                              db.escapeString(task_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string project_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            // Check if user is project member
            query = "SELECT member_id FROM project_members WHERE project_id = " +
                   db.escapeString(project_id) + " AND user_id = " +
                   db.escapeString(uploader_id);
            res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_UNAUTHORIZED;
            }
            PQclear(res);

            // Generate file ID
            std::string file_id = db.generateUUID();

            // Insert file record
            std::ostringstream insert_query;
            insert_query << "INSERT INTO file_attachments (file_id, task_id, uploaded_by, "
                        << "file_name, file_path, file_type, file_size, uploaded_at) "
                        << "VALUES (" << db.escapeString(file_id) << ", "
                        << db.escapeString(task_id) << ", "
                        << db.escapeString(uploader_id) << ", "
                        << db.escapeString(file_name) << ", "
                        << db.escapeString(file_path) << ", "
                        << db.escapeString(file_type) << ", "
                        << file_size << ", NOW())";

            db.executeQuery(insert_query.str());

            file_id_out = file_id;
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Upload file error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Get files for a task
    std::string getTaskFiles(const std::string& task_id) {
        try {
            std::string query =
                "SELECT f.file_id, f.file_name, f.file_path, f.file_type, f.file_size, "
                "f.uploaded_by, u.username as uploader_name, f.uploaded_at "
                "FROM file_attachments f "
                "JOIN users u ON f.uploaded_by = u.user_id "
                "WHERE f.task_id = " + db.escapeString(task_id) +
                " ORDER BY f.uploaded_at DESC";

            PGresult* res = db.executeQuery(query);
            int rows = PQntuples(res);

            std::ostringstream json;
            json << "[";

            for (int i = 0; i < rows; i++) {
                if (i > 0) json << ",";
                json << "{";
                json << "\"file_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"file_name\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"file_path\":\"" << PQgetvalue(res, i, 2) << "\",";
                json << "\"file_type\":\"" << PQgetvalue(res, i, 3) << "\",";
                json << "\"file_size\":" << PQgetvalue(res, i, 4) << ",";
                json << "\"uploaded_by\":\"" << PQgetvalue(res, i, 5) << "\",";
                json << "\"uploader_name\":\"" << PQgetvalue(res, i, 6) << "\",";
                json << "\"uploaded_at\":\"" << PQgetvalue(res, i, 7) << "\"";
                json << "}";
            }

            json << "]";
            PQclear(res);

            return json.str();
        } catch (const std::exception& e) {
            std::cerr << "Get files error: " << e.what() << std::endl;
            return "[]";
        }
    }

    // Delete file
    StatusCode deleteFile(const std::string& file_id, const std::string& user_id) {
        try {
            // Check if user is the uploader or project owner
            std::string query = "SELECT f.uploaded_by, t.project_id FROM file_attachments f "
                              "JOIN tasks t ON f.task_id = t.task_id "
                              "WHERE f.file_id = " + db.escapeString(file_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string uploader_id = PQgetvalue(res, 0, 0);
            std::string project_id = PQgetvalue(res, 0, 1);
            PQclear(res);

            // Check if user is uploader or project owner
            query = "SELECT owner_id FROM projects WHERE project_id = " +
                   db.escapeString(project_id);
            res = db.executeQuery(query);
            std::string owner_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            if (uploader_id != user_id && owner_id != user_id) {
                return STATUS_UNAUTHORIZED;
            }

            // Delete file record
            query = "DELETE FROM file_attachments WHERE file_id = " + db.escapeString(file_id);
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Delete file error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }
};
