#ifndef NETWORK_WRAPPER_H
#define NETWORK_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize network connection
int network_init(const char* host, int port);

// Cleanup network connection
void network_cleanup();

// Connect to server
int network_connect();

// Disconnect from server
void network_disconnect();

// Check if connected
int network_is_connected();

// Authentication functions
int network_register(const char* username, const char* email, const char* password, char* response, int response_size);
int network_login(const char* username, const char* password, char* response, int response_size);
int network_verify_otp(const char* email, const char* otp_code, const char* otp_type, char* response, int response_size);
int network_logout(char* response, int response_size);
int network_change_password(const char* old_password, const char* new_password, char* response, int response_size);
int network_reset_password(const char* username, const char* email, char* response, int response_size);
int network_confirm_reset_password(const char* email, const char* otp_code, const char* new_password, char* response, int response_size);

// Project functions
int network_get_projects(char* response, int response_size);
int network_create_project(const char* name, const char* description, char* response, int response_size);
int network_update_project(const char* project_id, const char* name, const char* description, const char* status, const char* start_date, const char* end_date, char* response, int response_size);
int network_delete_project(const char* project_id, char* response, int response_size);
int network_get_project_details(const char* project_id, char* response, int response_size);
int network_invite_to_project(const char* project_id, const char* invitee, char* response, int response_size);
int network_remove_project_member(const char* project_id, const char* member_id, char* response, int response_size);

// Task functions
int network_get_tasks(const char* project_id, char* response, int response_size);
int network_create_task(const char* project_id, const char* name, const char* description, const char* assigned_to, const char* priority, const char* start_date, const char* due_date, char* response, int response_size);
int network_update_task(const char* task_id, const char* name, const char* description, const char* assigned_to, const char* status, const char* priority, const char* start_date, const char* due_date, char* response, int response_size);
int network_delete_task(const char* task_id, char* response, int response_size);
int network_get_task_comments(const char* task_id, char* response, int response_size);
int network_add_task_comment(const char* task_id, const char* content, char* response, int response_size);

// Chat functions
int network_send_chat(const char* project_id, const char* message, char* response, int response_size);
int network_get_chat_history(const char* project_id, int limit, char* response, int response_size);

// File functions
int network_upload_file(const char* task_id, const char* file_name, const char* file_path, const char* file_type, int file_size, char* response, int response_size);
int network_get_files(const char* task_id, char* response, int response_size);

// Contact functions
int network_add_contact(const char* contact, char* response, int response_size);
int network_get_contacts(char* response, int response_size);

// Helper functions to parse JSON responses
int json_get_status(const char* json);
int json_get_string(const char* json, const char* key, char* output, int output_size);
int json_get_int(const char* json, const char* key);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_WRAPPER_H
