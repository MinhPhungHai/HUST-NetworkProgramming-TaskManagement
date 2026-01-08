#pragma once

#include "database_handler.h"
#include "../Common/protocol.h"
#include <vector>
#include <sstream>

class TaskService {
private:
    DatabaseHandler& db;

public:
    TaskService(DatabaseHandler& database) : db(database) {}

    // Get all tasks for a project
    std::string getProjectTasks(const std::string& project_id) {
        try {
            std::string query =
                "SELECT t.task_id, t.task_name, t.description, t.assigned_to, "
                "u1.username as assigned_name, t.created_by, u2.username as creator_name, "
                "t.status, t.priority, t.start_date, t.due_date, t.completed_at, t.created_at "
                "FROM tasks t "
                "LEFT JOIN users u1 ON t.assigned_to = u1.user_id "
                "JOIN users u2 ON t.created_by = u2.user_id "
                "WHERE t.project_id = " + db.escapeString(project_id) +
                " ORDER BY t.created_at DESC";

            PGresult* res = db.executeQuery(query);
            int rows = PQntuples(res);

            std::ostringstream json;
            json << "[";

            for (int i = 0; i < rows; i++) {
                if (i > 0) json << ",";
                json << "{";
                json << "\"task_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"task_name\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"description\":\"" << (PQgetisnull(res, i, 2) ? "" : PQgetvalue(res, i, 2)) << "\",";
                json << "\"assigned_to\":\"" << (PQgetisnull(res, i, 3) ? "" : PQgetvalue(res, i, 3)) << "\",";
                json << "\"assigned_name\":\"" << (PQgetisnull(res, i, 4) ? "" : PQgetvalue(res, i, 4)) << "\",";
                json << "\"created_by\":\"" << PQgetvalue(res, i, 5) << "\",";
                json << "\"creator_name\":\"" << PQgetvalue(res, i, 6) << "\",";
                json << "\"status\":\"" << PQgetvalue(res, i, 7) << "\",";
                json << "\"priority\":\"" << PQgetvalue(res, i, 8) << "\",";
                json << "\"start_date\":\"" << (PQgetisnull(res, i, 9) ? "" : PQgetvalue(res, i, 9)) << "\",";
                json << "\"due_date\":\"" << (PQgetisnull(res, i, 10) ? "" : PQgetvalue(res, i, 10)) << "\",";
                json << "\"completed_at\":\"" << (PQgetisnull(res, i, 11) ? "" : PQgetvalue(res, i, 11)) << "\",";
                json << "\"created_at\":\"" << PQgetvalue(res, i, 12) << "\"";
                json << "}";
            }

            json << "]";
            PQclear(res);

            return json.str();
        } catch (const std::exception& e) {
            std::cerr << "Get tasks error: " << e.what() << std::endl;
            return "[]";
        }
    }

    // Create new task
    StatusCode createTask(const std::string& project_id, const std::string& creator_id,
                         const std::string& task_name, const std::string& description,
                         const std::string& assigned_to, const std::string& priority,
                         const std::string& start_date, const std::string& due_date,
                         std::string& task_id_out) {
        try {
            // Check if user is member of project
            std::string query = "SELECT member_id FROM project_members WHERE project_id = " +
                              db.escapeString(project_id) + " AND user_id = " +
                              db.escapeString(creator_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_UNAUTHORIZED;
            }
            PQclear(res);

            // Generate task ID
            std::string task_id = db.generateUUID();

            // Insert task
            std::ostringstream insert_query;
            insert_query << "INSERT INTO tasks (task_id, project_id, task_name, description, "
                        << "created_by, assigned_to, status, priority, start_date, due_date, created_at) "
                        << "VALUES (" << db.escapeString(task_id) << ", "
                        << db.escapeString(project_id) << ", "
                        << db.escapeString(task_name) << ", ";

            if (description.empty()) {
                insert_query << "NULL, ";
            } else {
                insert_query << db.escapeString(description) << ", ";
            }

            insert_query << db.escapeString(creator_id) << ", ";

            if (assigned_to.empty()) {
                insert_query << "NULL, ";
            } else {
                insert_query << db.escapeString(assigned_to) << ", ";
            }

            insert_query << "'todo', ";

            if (priority.empty()) {
                insert_query << "'medium', ";
            } else {
                insert_query << db.escapeString(priority) << ", ";
            }

            if (start_date.empty()) {
                insert_query << "NULL, ";
            } else {
                insert_query << db.escapeString(start_date) << ", ";
            }

            if (due_date.empty()) {
                insert_query << "NULL, ";
            } else {
                insert_query << db.escapeString(due_date) << ", ";
            }

            insert_query << "NOW())";

            db.executeQuery(insert_query.str());

            // If task is assigned, create notification
            if (!assigned_to.empty() && assigned_to != creator_id) {
                std::string notif_id = db.generateUUID();
                query = "INSERT INTO notifications (notification_id, user_id, notification_type, message, is_read, created_at) "
                       "VALUES (" + db.escapeString(notif_id) + ", " +
                       db.escapeString(assigned_to) + ", 'task_assigned', " +
                       db.escapeString("You have been assigned to task: " + task_name) + ", false, NOW())";
                db.executeQuery(query);
            }

            task_id_out = task_id;
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Create task error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Update task
    StatusCode updateTask(const std::string& task_id, const std::string& user_id,
                         const std::string& task_name, const std::string& description,
                         const std::string& assigned_to, const std::string& status,
                         const std::string& priority, const std::string& start_date,
                         const std::string& due_date) {
        try {
            // Check if user is member of the project
            std::string query = "SELECT project_id FROM tasks WHERE task_id = " +
                              db.escapeString(task_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string project_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            query = "SELECT member_id FROM project_members WHERE project_id = " +
                   db.escapeString(project_id) + " AND user_id = " + db.escapeString(user_id);
            res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_UNAUTHORIZED;
            }
            PQclear(res);

            // Build update query
            std::ostringstream update_query;
            update_query << "UPDATE tasks SET ";

            bool first = true;
            if (!task_name.empty()) {
                update_query << "task_name = " << db.escapeString(task_name);
                first = false;
            }
            if (!description.empty()) {
                if (!first) update_query << ", ";
                update_query << "description = " << db.escapeString(description);
                first = false;
            }
            if (!assigned_to.empty()) {
                if (!first) update_query << ", ";
                update_query << "assigned_to = " << db.escapeString(assigned_to);
                first = false;
            }
            if (!status.empty()) {
                if (!first) update_query << ", ";
                update_query << "status = " << db.escapeString(status);

                // If status is completed, set completed_at
                if (status == "completed") {
                    update_query << ", completed_at = NOW()";
                }
                first = false;
            }
            if (!priority.empty()) {
                if (!first) update_query << ", ";
                update_query << "priority = " << db.escapeString(priority);
                first = false;
            }
            if (!start_date.empty()) {
                if (!first) update_query << ", ";
                update_query << "start_date = " << db.escapeString(start_date);
                first = false;
            }
            if (!due_date.empty()) {
                if (!first) update_query << ", ";
                update_query << "due_date = " << db.escapeString(due_date);
                first = false;
            }

            if (first) {
                return STATUS_INVALID_REQUEST; // Nothing to update
            }

            update_query << ", updated_at = NOW() WHERE task_id = " << db.escapeString(task_id);

            db.executeQuery(update_query.str());

            // If task is reassigned, create notification
            if (!assigned_to.empty() && assigned_to != user_id) {
                query = "SELECT task_name FROM tasks WHERE task_id = " + db.escapeString(task_id);
                res = db.executeQuery(query);
                std::string tname = PQgetvalue(res, 0, 0);
                PQclear(res);

                std::string notif_id = db.generateUUID();
                query = "INSERT INTO notifications (notification_id, user_id, notification_type, message, is_read, created_at) "
                       "VALUES (" + db.escapeString(notif_id) + ", " +
                       db.escapeString(assigned_to) + ", 'task_assigned', " +
                       db.escapeString("You have been assigned to task: " + tname) + ", false, NOW())";
                db.executeQuery(query);
            }

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Update task error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Delete task
    StatusCode deleteTask(const std::string& task_id, const std::string& user_id) {
        try {
            // Check if user is member or creator of task
            std::string query = "SELECT t.project_id, t.created_by FROM tasks t WHERE task_id = " +
                              db.escapeString(task_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string project_id = PQgetvalue(res, 0, 0);
            std::string creator_id = PQgetvalue(res, 0, 1);
            PQclear(res);

            // Check if user is project member
            query = "SELECT member_id FROM project_members WHERE project_id = " +
                   db.escapeString(project_id) + " AND user_id = " + db.escapeString(user_id);
            res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_UNAUTHORIZED;
            }
            PQclear(res);

            // Delete task (cascades to comments and files)
            query = "DELETE FROM tasks WHERE task_id = " + db.escapeString(task_id);
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Delete task error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }
};
