#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <map>
#include <mutex>

#include "../Common/protocol.h"
#include "../Common/json_helper.h"
#include "../Common/message_logger.h"
#include "database_handler.h"
#include "email_service.h"
#include "auth_service.h"
#include "project_service.h"
#include "task_service.h"
#include "comment_service.h"
#include "chat_service.h"
#include "file_service.h"
#include "contact_service.h"

#define PORT 8080
#define MAX_CLIENTS 100

// Global services
DatabaseHandler* g_db = nullptr;
EmailService* g_email = nullptr;
AuthService* g_auth = nullptr;
ProjectService* g_project = nullptr;
TaskService* g_task = nullptr;
CommentService* g_comment = nullptr;
ChatService* g_chat = nullptr;
FileService* g_file = nullptr;
ContactService* g_contact = nullptr;
MessageLogger* g_logger = nullptr;

// Track active sessions
std::map<int, std::string> active_sessions; // socket_fd -> user_id
std::mutex session_mutex;

// Send message helper
bool sendMessage(int socket_fd, MessageType type, const std::string& payload) {
    MessageHeader header;
    header.type = type;
    header.payload_size = payload.length();
    header.toNetwork();

    // Send header
    if (send(socket_fd, &header, sizeof(header), 0) != sizeof(header)) {
        return false;
    }

    // Send payload
    if (payload.length() > 0) {
        if (send(socket_fd, payload.c_str(), payload.length(), 0) != (ssize_t)payload.length()) {
            return false;
        }
    }

    return true;
}

// Receive message helper
bool receiveMessage(int socket_fd, MessageType& type, std::string& payload) {
    MessageHeader header;

    // Receive header
    ssize_t received = recv(socket_fd, &header, sizeof(header), MSG_WAITALL);
    if (received != sizeof(header)) {
        return false;
    }

    header.toHost();
    type = (MessageType)header.type;

    // Receive payload
    if (header.payload_size > 0) {
        char* buffer = new char[header.payload_size + 1];
        received = recv(socket_fd, buffer, header.payload_size, MSG_WAITALL);

        if (received != (ssize_t)header.payload_size) {
            delete[] buffer;
            return false;
        }

        buffer[header.payload_size] = '\0';
        payload = std::string(buffer);
        delete[] buffer;
    }

    return true;
}

// Send error response
void sendError(int socket_fd, StatusCode status, const std::string& message) {
    JsonBuilder response;
    response.add("status", (int)status)
           .add("message", message);

    sendMessage(socket_fd, MSG_ERROR, response.build());
}

// Send success response
void sendSuccess(int socket_fd, MessageType type, const std::string& data = "") {
    JsonBuilder response;
    response.add("status", (int)STATUS_SUCCESS);

    if (!data.empty()) {
        response.addRaw("data", data);
    }

    sendMessage(socket_fd, type, response.build());
}

// Get current user ID from session
std::string getCurrentUserId(int socket_fd) {
    std::lock_guard<std::mutex> lock(session_mutex);
    auto it = active_sessions.find(socket_fd);
    if (it != active_sessions.end()) {
        return it->second;
    }
    return "";
}

// Handle authentication messages
void handleAuth(int socket_fd, MessageType type, const std::string& payload) {
    JsonParser parser(payload);

    switch(type) {
        case MSG_REGISTER_REQUEST: {
            std::string username = parser.getString("username");
            std::string email = parser.getString("email");
            std::string password = parser.getString("password");

            std::string user_id;
            StatusCode status = g_auth->registerUser(username, email, password, user_id);

            if (status == STATUS_SUCCESS) {
                // Send OTP
                std::string otp_id;
                status = g_auth->sendOTP(email, "registration", otp_id);

                JsonBuilder response;
                response.add("status", (int)status)
                       .add("user_id", user_id)
                       .add("email", email);

                sendMessage(socket_fd, MSG_REGISTER_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Registration failed");
            }
            break;
        }

        case MSG_LOGIN_REQUEST: {
            std::string username_or_email = parser.getString("username");
            std::string password = parser.getString("password");

            std::string user_id, email;
            StatusCode status = g_auth->checkCredentials(username_or_email, password, user_id, email);

            if (status == STATUS_SUCCESS) {
                // Send OTP
                std::string otp_id;
                status = g_auth->sendOTP(username_or_email, "login", otp_id);

                JsonBuilder response;
                response.add("status", (int)status)
                       .add("user_id", user_id)
                       .add("email", email);

                sendMessage(socket_fd, MSG_LOGIN_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Login failed");
            }
            break;
        }

        case MSG_OTP_VERIFY_REQUEST: {
            std::string email = parser.getString("email");
            std::string otp_code = parser.getString("otp_code");
            std::string otp_type = parser.getString("otp_type");

            std::string user_id;
            StatusCode status = g_auth->verifyOTP(email, otp_code, otp_type, user_id);

            if (status == STATUS_SUCCESS) {
                // Create session
                {
                    std::lock_guard<std::mutex> lock(session_mutex);
                    active_sessions[socket_fd] = user_id;
                }

                std::string username, verified_email;
                bool is_verified;
                g_auth->getUserInfo(user_id, username, verified_email, is_verified);

                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .add("user_id", user_id)
                       .add("username", username)
                       .add("email", verified_email);

                sendMessage(socket_fd, MSG_OTP_VERIFY_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "OTP verification failed");
            }
            break;
        }

        case MSG_LOGOUT_REQUEST: {
            std::lock_guard<std::mutex> lock(session_mutex);
            active_sessions.erase(socket_fd);

            sendSuccess(socket_fd, MSG_LOGOUT_RESPONSE);
            break;
        }

        case MSG_CHANGE_PASSWORD_REQUEST: {
            std::string user_id = getCurrentUserId(socket_fd);
            if (user_id.empty()) {
                sendError(socket_fd, STATUS_UNAUTHORIZED, "Not logged in");
                break;
            }

            std::string old_password = parser.getString("old_password");
            std::string new_password = parser.getString("new_password");

            if (old_password.empty() || new_password.empty()) {
                sendError(socket_fd, STATUS_INVALID_REQUEST, "Missing password");
                break;
            }

            StatusCode status = g_auth->changePassword(user_id, old_password, new_password);
            if (status == STATUS_SUCCESS) {
                sendSuccess(socket_fd, MSG_CHANGE_PASSWORD_RESPONSE);
            } else {
                sendError(socket_fd, status, "Failed to change password");
            }
            break;
        }

        case MSG_RESET_PASSWORD_REQUEST: {
            std::string username = parser.getString("username");
            std::string email = parser.getString("email");

            if (username.empty() || email.empty()) {
                sendError(socket_fd, STATUS_INVALID_REQUEST, "Missing reset fields");
                break;
            }

            std::string otp_id;
            StatusCode status = g_auth->requestPasswordReset(username, email, otp_id);
            if (status == STATUS_SUCCESS) {
                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .add("email", email);
                sendMessage(socket_fd, MSG_RESET_PASSWORD_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Failed to reset password");
            }
            break;
        }

        case MSG_RESET_PASSWORD_CONFIRM_REQUEST: {
            std::string email = parser.getString("email");
            std::string otp_code = parser.getString("otp_code");
            std::string new_password = parser.getString("new_password");

            if (email.empty() || otp_code.empty() || new_password.empty()) {
                sendError(socket_fd, STATUS_INVALID_REQUEST, "Missing reset fields");
                break;
            }

            StatusCode status = g_auth->resetPasswordWithOTP(email, otp_code, new_password);
            if (status == STATUS_SUCCESS) {
                sendSuccess(socket_fd, MSG_RESET_PASSWORD_CONFIRM_RESPONSE);
            } else {
                sendError(socket_fd, status, "Failed to confirm reset");
            }
            break;
        }

        default:
            sendError(socket_fd, STATUS_INVALID_REQUEST, "Unknown auth message type");
    }
}

// Handle project messages
void handleProject(int socket_fd, MessageType type, const std::string& payload) {
    std::string user_id = getCurrentUserId(socket_fd);
    if (user_id.empty()) {
        sendError(socket_fd, STATUS_UNAUTHORIZED, "Not logged in");
        return;
    }

    JsonParser parser(payload);

    switch(type) {
        case MSG_GET_PROJECTS_REQUEST: {
            std::string projects_json = g_project->getUserProjects(user_id);

            JsonBuilder response;
            response.add("status", (int)STATUS_SUCCESS)
                   .addRaw("projects", projects_json);

            sendMessage(socket_fd, MSG_GET_PROJECTS_RESPONSE, response.build());
            break;
        }

        case MSG_CREATE_PROJECT_REQUEST: {
            std::string project_name = parser.getString("project_name");
            std::string description = parser.getString("description");

            // TODO: Parse member list from JSON array
            std::vector<std::string> member_ids;

            std::string project_id;
            StatusCode status = g_project->createProject(user_id, project_name, description, member_ids, project_id);

            if (status == STATUS_SUCCESS) {
                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .add("project_id", project_id);

                sendMessage(socket_fd, MSG_CREATE_PROJECT_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Failed to create project");
            }
            break;
        }

        case MSG_UPDATE_PROJECT_REQUEST: {
            std::string project_id = parser.getString("project_id");
            std::string project_name = parser.getString("project_name");
            std::string description = parser.getString("description");
            std::string status_str = parser.getString("status");
            std::string start_date = parser.getString("start_date");
            std::string end_date = parser.getString("end_date");

            StatusCode status = g_project->updateProject(project_id, user_id, project_name,
                                                        description, status_str, start_date, end_date);

            if (status == STATUS_SUCCESS) {
                sendSuccess(socket_fd, MSG_UPDATE_PROJECT_RESPONSE);
            } else {
                sendError(socket_fd, status, "Failed to update project");
            }
            break;
        }

        case MSG_DELETE_PROJECT_REQUEST: {
            std::string project_id = parser.getString("project_id");

            StatusCode status = g_project->deleteProject(project_id, user_id);

            if (status == STATUS_SUCCESS) {
                sendSuccess(socket_fd, MSG_DELETE_PROJECT_RESPONSE);
            } else {
                sendError(socket_fd, status, "Failed to delete project");
            }
            break;
        }

        case MSG_GET_PROJECT_DETAILS_REQUEST: {
            std::string project_id = parser.getString("project_id");
            std::string details = g_project->getProjectDetails(project_id);

            JsonBuilder response;
            response.add("status", (int)STATUS_SUCCESS)
                   .addRaw("project", details);

            sendMessage(socket_fd, MSG_GET_PROJECT_DETAILS_RESPONSE, response.build());
            break;
        }

        case MSG_INVITE_TO_PROJECT_REQUEST: {
            std::string project_id = parser.getString("project_id");
            std::string invitee = parser.getString("invitee");

            StatusCode status = g_project->inviteToProject(project_id, user_id, invitee);

            if (status == STATUS_SUCCESS) {
                sendSuccess(socket_fd, MSG_INVITE_TO_PROJECT_RESPONSE);
            } else {
                sendError(socket_fd, status, "Failed to invite user");
            }
            break;
        }

        case MSG_REMOVE_PROJECT_MEMBER_REQUEST: {
            std::string project_id = parser.getString("project_id");
            std::string member_id = parser.getString("member_id");

            StatusCode status = g_project->removeMember(project_id, user_id, member_id);

            if (status == STATUS_SUCCESS) {
                sendSuccess(socket_fd, MSG_REMOVE_PROJECT_MEMBER_RESPONSE);
            } else {
                sendError(socket_fd, status, "Failed to remove member");
            }
            break;
        }

        default:
            sendError(socket_fd, STATUS_INVALID_REQUEST, "Unknown project message type");
    }
}

// Handle task messages
void handleTask(int socket_fd, MessageType type, const std::string& payload) {
    std::string user_id = getCurrentUserId(socket_fd);
    if (user_id.empty()) {
        sendError(socket_fd, STATUS_UNAUTHORIZED, "Not logged in");
        return;
    }

    JsonParser parser(payload);

    switch(type) {
        case MSG_GET_TASKS_REQUEST: {
            std::string project_id = parser.getString("project_id");
            std::string tasks_json = g_task->getProjectTasks(project_id);

            JsonBuilder response;
            response.add("status", (int)STATUS_SUCCESS)
                   .addRaw("tasks", tasks_json);

            sendMessage(socket_fd, MSG_GET_TASKS_RESPONSE, response.build());
            break;
        }

        case MSG_CREATE_TASK_REQUEST: {
            std::string project_id = parser.getString("project_id");
            std::string task_name = parser.getString("task_name");
            std::string description = parser.getString("description");
            std::string assigned_to = parser.getString("assigned_to");
            std::string priority = parser.getString("priority");
            std::string start_date = parser.getString("start_date");
            std::string due_date = parser.getString("due_date");

            std::string task_id;
            StatusCode status = g_task->createTask(project_id, user_id, task_name, description,
                                                  assigned_to, priority, start_date, due_date, task_id);

            if (status == STATUS_SUCCESS) {
                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .add("task_id", task_id);

                sendMessage(socket_fd, MSG_CREATE_TASK_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Failed to create task");
            }
            break;
        }

        case MSG_UPDATE_TASK_REQUEST: {
            std::string task_id = parser.getString("task_id");
            std::string task_name = parser.getString("task_name");
            std::string description = parser.getString("description");
            std::string assigned_to = parser.getString("assigned_to");
            std::string status_str = parser.getString("status");
            std::string priority = parser.getString("priority");
            std::string start_date = parser.getString("start_date");
            std::string due_date = parser.getString("due_date");

            StatusCode status = g_task->updateTask(task_id, user_id, task_name, description,
                                                  assigned_to, status_str, priority, start_date, due_date);

            if (status == STATUS_SUCCESS) {
                sendSuccess(socket_fd, MSG_UPDATE_TASK_RESPONSE);
            } else {
                sendError(socket_fd, status, "Failed to update task");
            }
            break;
        }

        case MSG_DELETE_TASK_REQUEST: {
            std::string task_id = parser.getString("task_id");

            StatusCode status = g_task->deleteTask(task_id, user_id);

            if (status == STATUS_SUCCESS) {
                sendSuccess(socket_fd, MSG_DELETE_TASK_RESPONSE);
            } else {
                sendError(socket_fd, status, "Failed to delete task");
            }
            break;
        }

        default:
            sendError(socket_fd, STATUS_INVALID_REQUEST, "Unknown task message type");
    }
}

// Handle task comment messages
void handleTaskComment(int socket_fd, MessageType type, const std::string& payload) {
    std::string user_id = getCurrentUserId(socket_fd);
    if (user_id.empty()) {
        sendError(socket_fd, STATUS_UNAUTHORIZED, "Not logged in");
        return;
    }

    JsonParser parser(payload);

    switch(type) {
        case MSG_GET_TASK_COMMENTS_REQUEST: {
            std::string task_id = parser.getString("task_id");
            std::string comments_json;
            StatusCode status = g_comment->getTaskComments(task_id, user_id, comments_json);

            if (status == STATUS_SUCCESS) {
                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .addRaw("comments", comments_json);

                sendMessage(socket_fd, MSG_GET_TASK_COMMENTS_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Failed to get task comments");
            }
            break;
        }

        case MSG_ADD_TASK_COMMENT_REQUEST: {
            std::string task_id = parser.getString("task_id");
            std::string content = parser.getString("content");

            std::string comment_id;
            StatusCode status = g_comment->addTaskComment(task_id, user_id, content, comment_id);

            if (status == STATUS_SUCCESS) {
                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .add("comment_id", comment_id);

                sendMessage(socket_fd, MSG_ADD_TASK_COMMENT_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Failed to add task comment");
            }
            break;
        }

        default:
            sendError(socket_fd, STATUS_INVALID_REQUEST, "Unknown comment message type");
    }
}

// Handle chat messages
void handleChat(int socket_fd, MessageType type, const std::string& payload) {
    std::string user_id = getCurrentUserId(socket_fd);
    if (user_id.empty()) {
        sendError(socket_fd, STATUS_UNAUTHORIZED, "Not logged in");
        return;
    }

    JsonParser parser(payload);

    switch(type) {
        case MSG_SEND_CHAT_REQUEST: {
            std::string project_id = parser.getString("project_id");
            std::string message = parser.getString("message");

            std::string chat_id;
            StatusCode status = g_chat->sendMessage(project_id, user_id, message, chat_id);

            if (status == STATUS_SUCCESS) {
                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .add("chat_id", chat_id);

                sendMessage(socket_fd, MSG_SEND_CHAT_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Failed to send message");
            }
            break;
        }

        case MSG_GET_CHAT_HISTORY_REQUEST: {
            std::string project_id = parser.getString("project_id");
            int limit = parser.getInt("limit", 100);

            std::string messages = g_chat->getChatHistory(project_id, limit);

            JsonBuilder response;
            response.add("status", (int)STATUS_SUCCESS)
                   .addRaw("messages", messages);

            sendMessage(socket_fd, MSG_GET_CHAT_HISTORY_RESPONSE, response.build());
            break;
        }

        default:
            sendError(socket_fd, STATUS_INVALID_REQUEST, "Unknown chat message type");
    }
}

// Handle file messages
void handleFile(int socket_fd, MessageType type, const std::string& payload) {
    std::string user_id = getCurrentUserId(socket_fd);
    if (user_id.empty()) {
        sendError(socket_fd, STATUS_UNAUTHORIZED, "Not logged in");
        return;
    }

    JsonParser parser(payload);

    switch(type) {
        case MSG_UPLOAD_FILE_REQUEST: {
            std::string task_id = parser.getString("task_id");
            std::string file_name = parser.getString("file_name");
            std::string file_path = parser.getString("file_path");
            std::string file_type = parser.getString("file_type");
            int file_size = parser.getInt("file_size");

            std::string file_id;
            StatusCode status = g_file->uploadFile(task_id, user_id, file_name, file_path,
                                                  file_type, file_size, file_id);

            if (status == STATUS_SUCCESS) {
                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .add("file_id", file_id);

                sendMessage(socket_fd, MSG_UPLOAD_FILE_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Failed to upload file");
            }
            break;
        }

        case MSG_GET_FILES_REQUEST: {
            std::string task_id = parser.getString("task_id");
            std::string files = g_file->getTaskFiles(task_id);

            JsonBuilder response;
            response.add("status", (int)STATUS_SUCCESS)
                   .addRaw("files", files);

            sendMessage(socket_fd, MSG_GET_FILES_RESPONSE, response.build());
            break;
        }

        default:
            sendError(socket_fd, STATUS_INVALID_REQUEST, "Unknown file message type");
    }
}

// Handle contact messages
void handleContact(int socket_fd, MessageType type, const std::string& payload) {
    std::string user_id = getCurrentUserId(socket_fd);
    if (user_id.empty()) {
        sendError(socket_fd, STATUS_UNAUTHORIZED, "Not logged in");
        return;
    }

    JsonParser parser(payload);

    switch(type) {
        case MSG_ADD_CONTACT_REQUEST: {
            std::string contact = parser.getString("contact");

            std::string contact_id;
            StatusCode status = g_contact->addContact(user_id, contact, contact_id);

            if (status == STATUS_SUCCESS) {
                JsonBuilder response;
                response.add("status", (int)STATUS_SUCCESS)
                       .add("contact_id", contact_id);

                sendMessage(socket_fd, MSG_ADD_CONTACT_RESPONSE, response.build());
            } else {
                sendError(socket_fd, status, "Failed to add contact");
            }
            break;
        }

        case MSG_GET_CONTACTS_REQUEST: {
            std::string contacts = g_contact->getContacts(user_id);

            JsonBuilder response;
            response.add("status", (int)STATUS_SUCCESS)
                   .addRaw("contacts", contacts);

            sendMessage(socket_fd, MSG_GET_CONTACTS_RESPONSE, response.build());
            break;
        }

        default:
            sendError(socket_fd, STATUS_INVALID_REQUEST, "Unknown contact message type");
    }
}

// Handle client connection
void handleClient(int client_socket) {
    std::cout << "Client connected: " << client_socket << std::endl;

    while (true) {
        MessageType type;
        std::string payload;

        if (!receiveMessage(client_socket, type, payload)) {
            break;
        }

        g_logger->logReceived(type, payload, std::to_string(client_socket));

        // Route message to appropriate handler
        if (type >= MSG_LOGIN_REQUEST && type <= MSG_RESET_PASSWORD_RESPONSE) {
            handleAuth(client_socket, type, payload);
        } else if (type >= MSG_GET_PROJECTS_REQUEST && type <= MSG_REMOVE_PROJECT_MEMBER_RESPONSE) {
            handleProject(client_socket, type, payload);
        } else if (type >= MSG_GET_TASKS_REQUEST && type <= MSG_DELETE_TASK_RESPONSE) {
            handleTask(client_socket, type, payload);
        } else if (type >= MSG_GET_TASK_COMMENTS_REQUEST && type <= MSG_ADD_TASK_COMMENT_RESPONSE) {
            handleTaskComment(client_socket, type, payload);
        } else if (type >= MSG_SEND_CHAT_REQUEST && type <= MSG_GET_CHAT_HISTORY_RESPONSE) {
            handleChat(client_socket, type, payload);
        } else if (type >= MSG_UPLOAD_FILE_REQUEST && type <= MSG_GET_FILES_RESPONSE) {
            handleFile(client_socket, type, payload);
        } else if (type >= MSG_ADD_CONTACT_REQUEST && type <= MSG_GET_CONTACTS_RESPONSE) {
            handleContact(client_socket, type, payload);
        } else {
            sendError(client_socket, STATUS_INVALID_REQUEST, "Unknown message type");
        }
    }

    // Cleanup session
    {
        std::lock_guard<std::mutex> lock(session_mutex);
        active_sessions.erase(client_socket);
    }

    close(client_socket);
    std::cout << "Client disconnected: " << client_socket << std::endl;
}

int main() {
    std::cout << "===== Task Management Server =====" << std::endl;

    // Initialize services
    try {
        g_db = new DatabaseHandler();
        g_email = new EmailService();
        g_auth = new AuthService(*g_db, *g_email);
        g_project = new ProjectService(*g_db);
        g_task = new TaskService(*g_db);
        g_comment = new CommentService(*g_db);
        g_chat = new ChatService(*g_db);
        g_file = new FileService(*g_db);
        g_contact = new ContactService(*g_db);
        g_logger = new MessageLogger("server.log", true);
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize services: " << e.what() << std::endl;
        return 1;
    }

    // Create TCP socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Set socket options
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_socket);
        return 1;
    }

    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Accept connections
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        // Handle client in new thread
        std::thread client_thread(handleClient, client_socket);
        client_thread.detach();
    }

    // Cleanup
    close(server_socket);
    delete g_contact;
    delete g_file;
    delete g_chat;
    delete g_comment;
    delete g_task;
    delete g_project;
    delete g_auth;
    delete g_email;
    delete g_db;
    delete g_logger;

    return 0;
}
