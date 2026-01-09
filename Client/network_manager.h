#pragma once

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>
#include <fstream>
#include <vector>
#include "../Common/protocol.h"
#include "../Common/json_helper.h"
#include "../Common/message_logger.h"
#include "../Common/base64_util.h"

class NetworkManager {
private:
    int socket_fd;
    std::string server_host;
    int server_port;
    bool connected;
    std::mutex socket_mutex;
    MessageLogger* logger;

    // Send message helper
    bool sendMessage(MessageType type, const std::string& payload) {
        std::lock_guard<std::mutex> lock(socket_mutex);

        if (!connected) {
            return false;
        }

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

        if (logger) {
            logger->logSent(type, payload, server_host);
        }

        return true;
    }

    // Receive message helper
    bool receiveMessage(MessageType& type, std::string& payload) {
        std::lock_guard<std::mutex> lock(socket_mutex);

        if (!connected) {
            return false;
        }

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

        if (logger) {
            logger->logReceived(type, payload, server_host);
        }

        return true;
    }

public:
    NetworkManager(const std::string& host = "127.0.0.1", int port = 8080)
        : socket_fd(-1), server_host(host), server_port(port), connected(false), logger(nullptr) {
        try {
            logger = new MessageLogger("client.log", false);
        } catch (...) {
            logger = nullptr;
        }
    }

    ~NetworkManager() {
        disconnect();
        if (logger) {
            delete logger;
        }
    }

    // Connect to server
    bool connect() {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            return false;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);

        if (inet_pton(AF_INET, server_host.c_str(), &server_addr.sin_addr) <= 0) {
            close(socket_fd);
            socket_fd = -1;
            return false;
        }

        if (::connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(socket_fd);
            socket_fd = -1;
            return false;
        }

        connected = true;
        return true;
    }

    // Disconnect from server
    void disconnect() {
        std::lock_guard<std::mutex> lock(socket_mutex);
        if (socket_fd >= 0) {
            close(socket_fd);
            socket_fd = -1;
        }
        connected = false;
    }

    bool isConnected() const {
        return connected;
    }

    // Authentication methods
    bool registerUser(const std::string& username, const std::string& email,
                     const std::string& password, std::string& response_data) {
        JsonBuilder request;
        request.add("username", username)
               .add("email", email)
               .add("password", password);

        if (!sendMessage(MSG_REGISTER_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool login(const std::string& username, const std::string& password,
              std::string& response_data) {
        JsonBuilder request;
        request.add("username", username)
               .add("password", password);

        if (!sendMessage(MSG_LOGIN_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool verifyOTP(const std::string& email, const std::string& otp_code,
                   const std::string& otp_type, std::string& response_data) {
        JsonBuilder request;
        request.add("email", email)
               .add("otp_code", otp_code)
               .add("otp_type", otp_type);

        if (!sendMessage(MSG_OTP_VERIFY_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool logout(std::string& response_data) {
        JsonBuilder request;

        if (!sendMessage(MSG_LOGOUT_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool changePassword(const std::string& old_password, const std::string& new_password,
                       std::string& response_data) {
        JsonBuilder request;
        request.add("old_password", old_password)
               .add("new_password", new_password);

        if (!sendMessage(MSG_CHANGE_PASSWORD_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool resetPassword(const std::string& username, const std::string& email,
                      std::string& response_data) {
        JsonBuilder request;
        request.add("username", username)
               .add("email", email);

        if (!sendMessage(MSG_RESET_PASSWORD_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool confirmResetPassword(const std::string& email, const std::string& otp_code,
                             const std::string& new_password, std::string& response_data) {
        JsonBuilder request;
        request.add("email", email)
               .add("otp_code", otp_code)
               .add("new_password", new_password);

        if (!sendMessage(MSG_RESET_PASSWORD_CONFIRM_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    // Project methods
    bool getProjects(std::string& response_data) {
        JsonBuilder request;

        if (!sendMessage(MSG_GET_PROJECTS_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool createProject(const std::string& project_name, const std::string& description,
                      std::string& response_data) {
        JsonBuilder request;
        request.add("project_name", project_name)
               .add("description", description);

        if (!sendMessage(MSG_CREATE_PROJECT_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool updateProject(const std::string& project_id, const std::string& project_name,
                      const std::string& description, const std::string& status,
                      const std::string& start_date, const std::string& end_date,
                      std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id)
               .add("project_name", project_name)
               .add("description", description)
               .add("status", status)
               .add("start_date", start_date)
               .add("end_date", end_date);

        if (!sendMessage(MSG_UPDATE_PROJECT_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool deleteProject(const std::string& project_id, std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id);

        if (!sendMessage(MSG_DELETE_PROJECT_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool getProjectDetails(const std::string& project_id, std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id);

        if (!sendMessage(MSG_GET_PROJECT_DETAILS_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool inviteToProject(const std::string& project_id, const std::string& invitee,
                        std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id)
               .add("invitee", invitee);

        if (!sendMessage(MSG_INVITE_TO_PROJECT_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool removeProjectMember(const std::string& project_id, const std::string& member_id,
                            std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id)
               .add("member_id", member_id);

        if (!sendMessage(MSG_REMOVE_PROJECT_MEMBER_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    // Task methods
    bool getTasks(const std::string& project_id, std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id);

        if (!sendMessage(MSG_GET_TASKS_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool createTask(const std::string& project_id, const std::string& task_name,
                   const std::string& description, const std::string& assigned_to,
                   const std::string& priority, const std::string& start_date,
                   const std::string& due_date, std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id)
               .add("task_name", task_name)
               .add("description", description)
               .add("assigned_to", assigned_to)
               .add("priority", priority)
               .add("start_date", start_date)
               .add("due_date", due_date);

        if (!sendMessage(MSG_CREATE_TASK_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool updateTask(const std::string& task_id, const std::string& task_name,
                   const std::string& description, const std::string& assigned_to,
                   const std::string& status, const std::string& priority,
                   const std::string& start_date, const std::string& due_date,
                   std::string& response_data) {
        JsonBuilder request;
        request.add("task_id", task_id)
               .add("task_name", task_name)
               .add("description", description)
               .add("assigned_to", assigned_to)
               .add("status", status)
               .add("priority", priority)
               .add("start_date", start_date)
               .add("due_date", due_date);

        if (!sendMessage(MSG_UPDATE_TASK_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool deleteTask(const std::string& task_id, std::string& response_data) {
        JsonBuilder request;
        request.add("task_id", task_id);

        if (!sendMessage(MSG_DELETE_TASK_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    // Task comment methods
    bool getTaskComments(const std::string& task_id, std::string& response_data) {
        JsonBuilder request;
        request.add("task_id", task_id);

        if (!sendMessage(MSG_GET_TASK_COMMENTS_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool addTaskComment(const std::string& task_id, const std::string& content,
                       std::string& response_data) {
        JsonBuilder request;
        request.add("task_id", task_id)
               .add("content", content);

        if (!sendMessage(MSG_ADD_TASK_COMMENT_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    // Chat methods
    bool sendChatMessage(const std::string& project_id, const std::string& message,
                        std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id)
               .add("message", message);

        if (!sendMessage(MSG_SEND_CHAT_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool getChatHistory(const std::string& project_id, int limit,
                       std::string& response_data) {
        JsonBuilder request;
        request.add("project_id", project_id)
               .add("limit", limit);

        if (!sendMessage(MSG_GET_CHAT_HISTORY_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    // File methods
    bool uploadFile(const std::string& task_id, const std::string& file_name,
                   const std::string& file_path, const std::string& file_type,
                   int file_size, std::string& response_data) {
        JsonBuilder request;
        request.add("task_id", task_id)
               .add("file_name", file_name)
               .add("file_path", file_path)
               .add("file_type", file_type)
               .add("file_size", file_size);

        if (!sendMessage(MSG_UPLOAD_FILE_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool getFiles(const std::string& task_id, std::string& response_data) {
        JsonBuilder request;
        request.add("task_id", task_id);

        if (!sendMessage(MSG_GET_FILES_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool uploadFileWithContent(const std::string& task_id, const std::string& local_file_path,
                               std::string& response_data) {
        // Read file from local disk
        std::ifstream file(local_file_path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return false;
        }

        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> file_data(file_size);
        if (!file.read(file_data.data(), file_size)) {
            return false;
        }
        file.close();

        // Extract file name from path
        size_t last_slash = local_file_path.find_last_of("/\\");
        std::string file_name = (last_slash != std::string::npos)
                                ? local_file_path.substr(last_slash + 1)
                                : local_file_path;

        // Determine file type from extension
        std::string file_type = "application/octet-stream";
        size_t dot_pos = file_name.find_last_of('.');
        if (dot_pos != std::string::npos) {
            file_type = file_name.substr(dot_pos + 1);
        }

        // Encode file data as base64
        std::string encoded_data = base64_encode(file_data.data(), file_size);

        // Build request
        JsonBuilder request;
        request.add("task_id", task_id)
               .add("file_name", file_name)
               .add("file_type", file_type)
               .add("file_size", (int)file_size)
               .add("file_data", encoded_data);

        if (!sendMessage(MSG_UPLOAD_FILE_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool downloadFileContent(const std::string& file_id, std::vector<char>& file_data_out,
                            std::string& file_name_out, std::string& file_type_out) {
        JsonBuilder request;
        request.add("file_id", file_id);

        if (!sendMessage(MSG_DOWNLOAD_FILE_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        std::string response_data;
        if (!receiveMessage(response_type, response_data)) {
            return false;
        }

        // Parse response
        JsonParser parser(response_data);
        int status = parser.getInt("status");
        if (status != STATUS_SUCCESS) {
            return false;
        }

        file_name_out = parser.getString("file_name");
        file_type_out = parser.getString("file_type");
        std::string encoded_data = parser.getString("file_data");

        // Decode base64 data
        std::string decoded_data = base64_decode(encoded_data);
        file_data_out.assign(decoded_data.begin(), decoded_data.end());

        return true;
    }

    bool deleteFile(const std::string& file_id, std::string& response_data) {
        JsonBuilder request;
        request.add("file_id", file_id);

        if (!sendMessage(MSG_DELETE_FILE_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    // Contact methods
    bool addContact(const std::string& contact, std::string& response_data) {
        JsonBuilder request;
        request.add("contact", contact);

        if (!sendMessage(MSG_ADD_CONTACT_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }

    bool getContacts(std::string& response_data) {
        JsonBuilder request;

        if (!sendMessage(MSG_GET_CONTACTS_REQUEST, request.build())) {
            return false;
        }

        MessageType response_type;
        return receiveMessage(response_type, response_data);
    }
};
