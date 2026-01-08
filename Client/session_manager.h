#pragma once

#include <string>
#include <vector>
#include <map>

struct UserInfo {
    std::string user_id;
    std::string username;
    std::string email;
    bool is_verified;
};

struct ProjectInfo {
    std::string project_id;
    std::string project_name;
    std::string description;
    std::string owner_id;
    std::string owner_name;
    std::string status;
    std::string start_date;
    std::string end_date;
    std::string created_at;
};

struct TaskInfo {
    std::string task_id;
    std::string task_name;
    std::string description;
    std::string assigned_to;
    std::string assigned_name;
    std::string created_by;
    std::string creator_name;
    std::string status;
    std::string priority;
    std::string start_date;
    std::string due_date;
    std::string completed_at;
    std::string created_at;
};

struct ChatMessage {
    std::string chat_id;
    std::string sender_id;
    std::string sender_name;
    std::string message;
    std::string timestamp;
};

struct ContactInfo {
    std::string contact_id;
    std::string user_id;
    std::string username;
    std::string email;
    std::string status;
};

struct FileInfo {
    std::string file_id;
    std::string file_name;
    std::string file_path;
    std::string file_type;
    int file_size;
    std::string uploaded_by;
    std::string uploader_name;
    std::string uploaded_at;
};

class SessionManager {
private:
    UserInfo current_user;
    bool logged_in;
    std::string current_project_id;

    std::vector<ProjectInfo> projects;
    std::map<std::string, std::vector<TaskInfo>> project_tasks; // project_id -> tasks
    std::map<std::string, std::vector<ChatMessage>> project_chats; // project_id -> messages
    std::vector<ContactInfo> contacts;

public:
    SessionManager() : logged_in(false) {}

    // User session
    void setCurrentUser(const UserInfo& user) {
        current_user = user;
        logged_in = true;
    }

    void clearSession() {
        current_user = UserInfo();
        logged_in = false;
        current_project_id.clear();
        projects.clear();
        project_tasks.clear();
        project_chats.clear();
        contacts.clear();
    }

    bool isLoggedIn() const {
        return logged_in;
    }

    const UserInfo& getCurrentUser() const {
        return current_user;
    }

    std::string getCurrentUserId() const {
        return current_user.user_id;
    }

    std::string getCurrentUsername() const {
        return current_user.username;
    }

    // Project management
    void setProjects(const std::vector<ProjectInfo>& proj_list) {
        projects = proj_list;
    }

    const std::vector<ProjectInfo>& getProjects() const {
        return projects;
    }

    void addProject(const ProjectInfo& project) {
        projects.push_back(project);
    }

    void updateProject(const ProjectInfo& updated_project) {
        for (auto& project : projects) {
            if (project.project_id == updated_project.project_id) {
                project = updated_project;
                break;
            }
        }
    }

    void removeProject(const std::string& project_id) {
        for (auto it = projects.begin(); it != projects.end(); ++it) {
            if (it->project_id == project_id) {
                projects.erase(it);
                project_tasks.erase(project_id);
                project_chats.erase(project_id);
                break;
            }
        }
    }

    ProjectInfo* getProject(const std::string& project_id) {
        for (auto& project : projects) {
            if (project.project_id == project_id) {
                return &project;
            }
        }
        return nullptr;
    }

    // Current project
    void setCurrentProject(const std::string& project_id) {
        current_project_id = project_id;
    }

    std::string getCurrentProjectId() const {
        return current_project_id;
    }

    ProjectInfo* getCurrentProject() {
        if (current_project_id.empty()) {
            return nullptr;
        }
        return getProject(current_project_id);
    }

    // Task management
    void setTasks(const std::string& project_id, const std::vector<TaskInfo>& tasks) {
        project_tasks[project_id] = tasks;
    }

    const std::vector<TaskInfo>& getTasks(const std::string& project_id) {
        return project_tasks[project_id];
    }

    std::vector<TaskInfo>& getTasksRef(const std::string& project_id) {
        return project_tasks[project_id];
    }

    void addTask(const std::string& project_id, const TaskInfo& task) {
        project_tasks[project_id].push_back(task);
    }

    void updateTask(const std::string& project_id, const TaskInfo& updated_task) {
        auto& tasks = project_tasks[project_id];
        for (auto& task : tasks) {
            if (task.task_id == updated_task.task_id) {
                task = updated_task;
                break;
            }
        }
    }

    void removeTask(const std::string& project_id, const std::string& task_id) {
        auto& tasks = project_tasks[project_id];
        for (auto it = tasks.begin(); it != tasks.end(); ++it) {
            if (it->task_id == task_id) {
                tasks.erase(it);
                break;
            }
        }
    }

    TaskInfo* getTask(const std::string& project_id, const std::string& task_id) {
        auto& tasks = project_tasks[project_id];
        for (auto& task : tasks) {
            if (task.task_id == task_id) {
                return &task;
            }
        }
        return nullptr;
    }

    // Chat management
    void setChatMessages(const std::string& project_id, const std::vector<ChatMessage>& messages) {
        project_chats[project_id] = messages;
    }

    const std::vector<ChatMessage>& getChatMessages(const std::string& project_id) {
        return project_chats[project_id];
    }

    void addChatMessage(const std::string& project_id, const ChatMessage& message) {
        project_chats[project_id].push_back(message);
    }

    // Contact management
    void setContacts(const std::vector<ContactInfo>& contact_list) {
        contacts = contact_list;
    }

    const std::vector<ContactInfo>& getContacts() const {
        return contacts;
    }

    void addContact(const ContactInfo& contact) {
        contacts.push_back(contact);
    }

    // Helper to check if user is project owner
    bool isProjectOwner(const std::string& project_id) const {
        for (const auto& project : projects) {
            if (project.project_id == project_id) {
                return project.owner_id == current_user.user_id;
            }
        }
        return false;
    }

    bool isCurrentProjectOwner() const {
        if (current_project_id.empty()) {
            return false;
        }
        return isProjectOwner(current_project_id);
    }
};
