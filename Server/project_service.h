#pragma once

#include "database_handler.h"
#include "../Common/protocol.h"
#include <vector>
#include <sstream>

class ProjectService {
private:
    DatabaseHandler& db;

public:
    ProjectService(DatabaseHandler& database) : db(database) {}

    // Get all projects for a user
    std::string getUserProjects(const std::string& user_id) {
        try {
            std::string query =
                "SELECT p.project_id, p.project_name, p.description, p.owner_id, "
                "p.start_date, p.end_date, p.status, p.created_at, u.username as owner_name "
                "FROM projects p "
                "JOIN users u ON p.owner_id = u.user_id "
                "WHERE p.project_id IN ("
                "  SELECT project_id FROM project_members WHERE user_id = " + db.escapeString(user_id) +
                ") OR p.owner_id = " + db.escapeString(user_id) +
                " ORDER BY p.created_at DESC";

            PGresult* res = db.executeQuery(query);
            int rows = PQntuples(res);

            std::ostringstream json;
            json << "[";

            for (int i = 0; i < rows; i++) {
                if (i > 0) json << ",";
                json << "{";
                json << "\"project_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"project_name\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"description\":\"" << (PQgetisnull(res, i, 2) ? "" : PQgetvalue(res, i, 2)) << "\",";
                json << "\"owner_id\":\"" << PQgetvalue(res, i, 3) << "\",";
                json << "\"start_date\":\"" << (PQgetisnull(res, i, 4) ? "" : PQgetvalue(res, i, 4)) << "\",";
                json << "\"end_date\":\"" << (PQgetisnull(res, i, 5) ? "" : PQgetvalue(res, i, 5)) << "\",";
                json << "\"status\":\"" << PQgetvalue(res, i, 6) << "\",";
                json << "\"created_at\":\"" << PQgetvalue(res, i, 7) << "\",";
                json << "\"owner_name\":\"" << PQgetvalue(res, i, 8) << "\"";
                json << "}";
            }

            json << "]";
            PQclear(res);

            return json.str();
        } catch (const std::exception& e) {
            std::cerr << "Get projects error: " << e.what() << std::endl;
            return "[]";
        }
    }

    // Create new project
    StatusCode createProject(const std::string& owner_id, const std::string& project_name,
                            const std::string& description, const std::vector<std::string>& member_ids,
                            std::string& project_id_out) {
        try {
            // Generate project ID
            std::string project_id = db.generateUUID();

            // Insert project
            std::string query =
                "INSERT INTO projects (project_id, project_name, description, owner_id, status, created_at) "
                "VALUES (" + db.escapeString(project_id) + ", " +
                db.escapeString(project_name) + ", " +
                db.escapeString(description) + ", " +
                db.escapeString(owner_id) + ", 'planning', NOW())";

            db.executeQuery(query);

            // Add owner as admin member
            std::string member_id = db.generateUUID();
            query = "INSERT INTO project_members (member_id, project_id, user_id, role, joined_at) "
                   "VALUES (" + db.escapeString(member_id) + ", " +
                   db.escapeString(project_id) + ", " +
                   db.escapeString(owner_id) + ", 'owner', NOW())";
            db.executeQuery(query);

            // Add invited members
            for (const auto& user_id : member_ids) {
                if (user_id != owner_id) {
                    member_id = db.generateUUID();
                    query = "INSERT INTO project_members (member_id, project_id, user_id, role, joined_at) "
                           "VALUES (" + db.escapeString(member_id) + ", " +
                           db.escapeString(project_id) + ", " +
                           db.escapeString(user_id) + ", 'member', NOW())";
                    db.executeQuery(query);

                    // Create notification for invited user
                    std::string notif_id = db.generateUUID();
                    query = "INSERT INTO notifications (notification_id, user_id, notification_type, message, is_read, created_at) "
                           "VALUES (" + db.escapeString(notif_id) + ", " +
                           db.escapeString(user_id) + ", 'project_invite', " +
                           db.escapeString("You have been invited to project: " + project_name) + ", false, NOW())";
                    db.executeQuery(query);
                }
            }

            project_id_out = project_id;
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Create project error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Update project
    StatusCode updateProject(const std::string& project_id, const std::string& user_id,
                            const std::string& project_name, const std::string& description,
                            const std::string& status, const std::string& start_date,
                            const std::string& end_date) {
        try {
            // Check if user is owner
            std::string query = "SELECT owner_id FROM projects WHERE project_id = " +
                              db.escapeString(project_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string owner_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            if (owner_id != user_id) {
                return STATUS_UNAUTHORIZED;
            }

            // Update project
            std::ostringstream update_query;
            update_query << "UPDATE projects SET ";

            bool first = true;
            if (!project_name.empty()) {
                update_query << "project_name = " << db.escapeString(project_name);
                first = false;
            }
            if (!description.empty()) {
                if (!first) update_query << ", ";
                update_query << "description = " << db.escapeString(description);
                first = false;
            }
            if (!status.empty()) {
                if (!first) update_query << ", ";
                update_query << "status = " << db.escapeString(status);
                first = false;
            }
            if (!start_date.empty()) {
                if (!first) update_query << ", ";
                update_query << "start_date = " << db.escapeString(start_date);
                first = false;
            }
            if (!end_date.empty()) {
                if (!first) update_query << ", ";
                update_query << "end_date = " << db.escapeString(end_date);
                first = false;
            }

            if (first) {
                return STATUS_INVALID_REQUEST; // Nothing to update
            }

            update_query << ", updated_at = NOW() WHERE project_id = " << db.escapeString(project_id);

            db.executeQuery(update_query.str());
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Update project error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Delete project
    StatusCode deleteProject(const std::string& project_id, const std::string& user_id) {
        try {
            // Check if user is owner
            std::string query = "SELECT owner_id FROM projects WHERE project_id = " +
                              db.escapeString(project_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string owner_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            if (owner_id != user_id) {
                return STATUS_UNAUTHORIZED;
            }

            // Delete project (cascades to members, tasks, chat, etc.)
            query = "DELETE FROM projects WHERE project_id = " + db.escapeString(project_id);
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Delete project error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Get project details with members
    std::string getProjectDetails(const std::string& project_id) {
        try {
            // Get project info
            std::string query =
                "SELECT p.project_id, p.project_name, p.description, p.owner_id, "
                "p.start_date, p.end_date, p.status, u.username as owner_name "
                "FROM projects p "
                "JOIN users u ON p.owner_id = u.user_id "
                "WHERE p.project_id = " + db.escapeString(project_id);

            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return "{}";
            }

            std::ostringstream json;
            json << "{";
            json << "\"project_id\":\"" << PQgetvalue(res, 0, 0) << "\",";
            json << "\"project_name\":\"" << PQgetvalue(res, 0, 1) << "\",";
            json << "\"description\":\"" << (PQgetisnull(res, 0, 2) ? "" : PQgetvalue(res, 0, 2)) << "\",";
            json << "\"owner_id\":\"" << PQgetvalue(res, 0, 3) << "\",";
            json << "\"start_date\":\"" << (PQgetisnull(res, 0, 4) ? "" : PQgetvalue(res, 0, 4)) << "\",";
            json << "\"end_date\":\"" << (PQgetisnull(res, 0, 5) ? "" : PQgetvalue(res, 0, 5)) << "\",";
            json << "\"status\":\"" << PQgetvalue(res, 0, 6) << "\",";
            json << "\"owner_name\":\"" << PQgetvalue(res, 0, 7) << "\",";

            PQclear(res);

            // Get members
            query = "SELECT pm.user_id, u.username, pm.role FROM project_members pm "
                   "JOIN users u ON pm.user_id = u.user_id "
                   "WHERE pm.project_id = " + db.escapeString(project_id);

            res = db.executeQuery(query);
            int rows = PQntuples(res);

            json << "\"members\":[";
            for (int i = 0; i < rows; i++) {
                if (i > 0) json << ",";
                json << "{";
                json << "\"user_id\":\"" << PQgetvalue(res, i, 0) << "\",";
                json << "\"username\":\"" << PQgetvalue(res, i, 1) << "\",";
                json << "\"role\":\"" << PQgetvalue(res, i, 2) << "\"";
                json << "}";
            }
            json << "]";

            PQclear(res);
            json << "}";

            return json.str();
        } catch (const std::exception& e) {
            std::cerr << "Get project details error: " << e.what() << std::endl;
            return "{}";
        }
    }

    // Invite user to project
    StatusCode inviteToProject(const std::string& project_id, const std::string& inviter_id,
                              const std::string& invitee_username_or_email) {
        try {
            // Check if inviter is owner or admin
            std::string query = "SELECT owner_id FROM projects WHERE project_id = " +
                              db.escapeString(project_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string owner_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            if (owner_id != inviter_id) {
                return STATUS_UNAUTHORIZED;
            }

            // Find invitee user
            query = "SELECT user_id FROM users WHERE username = " +
                   db.escapeString(invitee_username_or_email) + " OR email = " +
                   db.escapeString(invitee_username_or_email);

            res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_USER_NOT_FOUND;
            }

            std::string invitee_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            // Check if already member
            query = "SELECT member_id FROM project_members WHERE project_id = " +
                   db.escapeString(project_id) + " AND user_id = " + db.escapeString(invitee_id);

            res = db.executeQuery(query);

            if (PQntuples(res) > 0) {
                PQclear(res);
                return STATUS_ERROR; // Already a member
            }
            PQclear(res);

            // Add as member
            std::string member_id = db.generateUUID();
            query = "INSERT INTO project_members (member_id, project_id, user_id, role, joined_at) "
                   "VALUES (" + db.escapeString(member_id) + ", " +
                   db.escapeString(project_id) + ", " +
                   db.escapeString(invitee_id) + ", 'member', NOW())";
            db.executeQuery(query);

            // Create notification
            std::string notif_id = db.generateUUID();
            query = "SELECT project_name FROM projects WHERE project_id = " + db.escapeString(project_id);
            res = db.executeQuery(query);
            std::string project_name = PQgetvalue(res, 0, 0);
            PQclear(res);

            query = "INSERT INTO notifications (notification_id, user_id, notification_type, message, is_read, created_at) "
                   "VALUES (" + db.escapeString(notif_id) + ", " +
                   db.escapeString(invitee_id) + ", 'project_invite', " +
                   db.escapeString("You have been invited to project: " + project_name) + ", false, NOW())";
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Invite to project error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Remove member from project (owner only)
    StatusCode removeMember(const std::string& project_id, const std::string& owner_id,
                            const std::string& member_user_id) {
        try {
            std::string query = "SELECT owner_id FROM projects WHERE project_id = " +
                              db.escapeString(project_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }

            std::string current_owner = PQgetvalue(res, 0, 0);
            PQclear(res);

            if (current_owner != owner_id) {
                return STATUS_UNAUTHORIZED;
            }

            if (member_user_id == current_owner) {
                return STATUS_INVALID_REQUEST;
            }

            query = "SELECT member_id FROM project_members WHERE project_id = " +
                   db.escapeString(project_id) + " AND user_id = " + db.escapeString(member_user_id);
            res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_NOT_FOUND;
            }
            PQclear(res);

            query = "DELETE FROM project_members WHERE project_id = " +
                   db.escapeString(project_id) + " AND user_id = " + db.escapeString(member_user_id);
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Remove member error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }
};
