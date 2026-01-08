#include "network_wrapper.h"
#include "network_manager.h"
#include "../Common/json_helper.h"
#include "../Common/protocol.h"
#include <cstring>

static NetworkManager* g_network = nullptr;

extern "C" {

int network_init(const char* host, int port) {
    if (g_network != nullptr) {
        return 1; // Already initialized
    }

    try {
        g_network = new NetworkManager(host, port);
        return 1;
    } catch (...) {
        return 0;
    }
}

void network_cleanup() {
    if (g_network != nullptr) {
        delete g_network;
        g_network = nullptr;
    }
}

int network_connect() {
    if (g_network == nullptr) {
        return 0;
    }
    return g_network->connect() ? 1 : 0;
}

void network_disconnect() {
    if (g_network != nullptr) {
        g_network->disconnect();
    }
}

int network_is_connected() {
    if (g_network == nullptr) {
        return 0;
    }
    return g_network->isConnected() ? 1 : 0;
}

int network_register(const char* username, const char* email, const char* password, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->registerUser(username, email, password, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_login(const char* username, const char* password, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->login(username, password, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_verify_otp(const char* email, const char* otp_code, const char* otp_type, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->verifyOTP(email, otp_code, otp_type, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_logout(char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->logout(resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_change_password(const char* old_password, const char* new_password, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->changePassword(old_password, new_password, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_reset_password(const char* username, const char* email, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->resetPassword(username, email, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_confirm_reset_password(const char* email, const char* otp_code, const char* new_password, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->confirmResetPassword(email, otp_code, new_password, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_get_projects(char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->getProjects(resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_create_project(const char* name, const char* description, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->createProject(name, description, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_update_project(const char* project_id, const char* name, const char* description, const char* status, const char* start_date, const char* end_date, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->updateProject(project_id, name, description, status, start_date, end_date, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_delete_project(const char* project_id, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->deleteProject(project_id, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_get_project_details(const char* project_id, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->getProjectDetails(project_id, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_invite_to_project(const char* project_id, const char* invitee, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->inviteToProject(project_id, invitee, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_remove_project_member(const char* project_id, const char* member_id, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->removeProjectMember(project_id, member_id, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_get_tasks(const char* project_id, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->getTasks(project_id, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_create_task(const char* project_id, const char* name, const char* description, const char* assigned_to, const char* priority, const char* start_date, const char* due_date, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->createTask(project_id, name, description, assigned_to, priority, start_date, due_date, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_update_task(const char* task_id, const char* name, const char* description, const char* assigned_to, const char* status, const char* priority, const char* start_date, const char* due_date, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->updateTask(task_id, name, description, assigned_to, status, priority, start_date, due_date, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_delete_task(const char* task_id, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->deleteTask(task_id, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_get_task_comments(const char* task_id, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->getTaskComments(task_id, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_add_task_comment(const char* task_id, const char* content, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->addTaskComment(task_id, content, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_send_chat(const char* project_id, const char* message, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->sendChatMessage(project_id, message, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_get_chat_history(const char* project_id, int limit, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->getChatHistory(project_id, limit, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_upload_file(const char* task_id, const char* file_name, const char* file_path, const char* file_type, int file_size, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->uploadFile(task_id, file_name, file_path, file_type, file_size, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_get_files(const char* task_id, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->getFiles(task_id, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_add_contact(const char* contact, char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->addContact(contact, resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

int network_get_contacts(char* response, int response_size) {
    if (g_network == nullptr) {
        return 0;
    }

    std::string resp;
    if (!g_network->getContacts(resp)) {
        return 0;
    }

    strncpy(response, resp.c_str(), response_size - 1);
    response[response_size - 1] = '\0';
    return 1;
}

// Helper functions
int json_get_status(const char* json) {
    JsonParser parser(json);
    return parser.getInt("status");
}

int json_get_string(const char* json, const char* key, char* output, int output_size) {
    JsonParser parser(json);
    std::string value = parser.getString(key);

    if (value.empty()) {
        return 0;
    }

    strncpy(output, value.c_str(), output_size - 1);
    output[output_size - 1] = '\0';
    return 1;
}

int json_get_int(const char* json, const char* key) {
    JsonParser parser(json);
    return parser.getInt(key);
}

} // extern "C"
