#pragma once

#include "database_handler.h"
#include "../Common/protocol.h"
#include <sstream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

class FileService {
private:
    DatabaseHandler& db;
    std::string storage_path;

    // Create storage directory if it doesn't exist
    void ensureStorageDirectory() {
        struct stat st = {0};
        if (stat(storage_path.c_str(), &st) == -1) {
            mkdir(storage_path.c_str(), 0755);
        }
    }

    // Get full file path on server
    std::string getServerFilePath(const std::string& file_id) {
        return storage_path + "/" + file_id;
    }

public:
    FileService(DatabaseHandler& database, const std::string& storage_dir = "./uploads")
        : db(database), storage_path(storage_dir) {
        ensureStorageDirectory();
    }

    // Upload file with binary content
    StatusCode uploadFileContent(const std::string& task_id, const std::string& uploader_id,
                                 const std::string& file_name, const std::string& file_type,
                                 const char* file_data, size_t file_size,
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

            // Save file to disk
            std::string server_path = getServerFilePath(file_id);
            std::ofstream file(server_path, std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "Failed to create file: " << server_path << std::endl;
                return STATUS_DATABASE_ERROR;
            }

            file.write(file_data, file_size);
            file.close();

            // Insert file record
            std::ostringstream insert_query;
            insert_query << "INSERT INTO file_attachments (file_id, task_id, uploaded_by, "
                        << "file_name, file_path, file_type, file_size, created_at) "
                        << "VALUES (" << db.escapeString(file_id) << ", "
                        << db.escapeString(task_id) << ", "
                        << db.escapeString(uploader_id) << ", "
                        << db.escapeString(file_name) << ", "
                        << db.escapeString(server_path) << ", "
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

    // Legacy upload (metadata only) - keeping for compatibility
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
                        << "file_name, file_path, file_type, file_size, created_at) "
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
                "f.uploaded_by, u.username as uploader_name, f.created_at "
                "FROM file_attachments f "
                "JOIN users u ON f.uploaded_by = u.user_id "
                "WHERE f.task_id = " + db.escapeString(task_id) +
                " ORDER BY f.created_at DESC";

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

    // Download file content
    StatusCode downloadFileContent(const std::string& file_id, const std::string& user_id,
                                   std::vector<char>& file_data_out,
                                   std::string& file_name_out,
                                   std::string& file_type_out) {
        try {
            // Get file metadata and check access
            std::string query = "SELECT f.file_name, f.file_path, f.file_type, t.project_id "
                              "FROM file_attachments f "
                              "JOIN tasks t ON f.task_id = t.task_id "
                              "WHERE f.file_id = " + db.escapeString(file_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            file_name_out = PQgetvalue(res, 0, 0);
            std::string file_path = PQgetvalue(res, 0, 1);
            file_type_out = PQgetvalue(res, 0, 2);
            std::string project_id = PQgetvalue(res, 0, 3);
            PQclear(res);

            // Check if user is project member
            query = "SELECT member_id FROM project_members WHERE project_id = " +
                   db.escapeString(project_id) + " AND user_id = " +
                   db.escapeString(user_id);
            res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_UNAUTHORIZED;
            }
            PQclear(res);

            // Read file from disk
            std::ifstream file(file_path, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << file_path << std::endl;
                return STATUS_NOT_FOUND;
            }

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            file_data_out.resize(size);
            if (!file.read(file_data_out.data(), size)) {
                std::cerr << "Failed to read file: " << file_path << std::endl;
                return STATUS_DATABASE_ERROR;
            }

            file.close();
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Download file error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Delete file
    StatusCode deleteFile(const std::string& file_id, const std::string& user_id) {
        try {
            // Check if user is the uploader or project owner
            std::string query = "SELECT f.uploaded_by, f.file_path, t.project_id FROM file_attachments f "
                              "JOIN tasks t ON f.task_id = t.task_id "
                              "WHERE f.file_id = " + db.escapeString(file_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string uploader_id = PQgetvalue(res, 0, 0);
            std::string file_path = PQgetvalue(res, 0, 1);
            std::string project_id = PQgetvalue(res, 0, 2);
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

            // Delete file from disk
            if (unlink(file_path.c_str()) != 0 && errno != ENOENT) {
                std::cerr << "Failed to delete file from disk: " << file_path << std::endl;
            }

            // Delete file record from database
            query = "DELETE FROM file_attachments WHERE file_id = " + db.escapeString(file_id);
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Delete file error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }
};
