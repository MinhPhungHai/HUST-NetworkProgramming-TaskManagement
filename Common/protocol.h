#pragma once

#include <string>
#include <cstring>
#include <arpa/inet.h>

// Protocol version
#define PROTOCOL_VERSION "1.0"

// Message types
enum MessageType {
    // Authentication
    MSG_LOGIN_REQUEST = 1,
    MSG_LOGIN_RESPONSE = 2,
    MSG_REGISTER_REQUEST = 3,
    MSG_REGISTER_RESPONSE = 4,
    MSG_OTP_REQUEST = 5,
    MSG_OTP_RESPONSE = 6,
    MSG_OTP_VERIFY_REQUEST = 7,
    MSG_OTP_VERIFY_RESPONSE = 8,
    MSG_LOGOUT_REQUEST = 9,
    MSG_LOGOUT_RESPONSE = 10,

    // Project management
    MSG_GET_PROJECTS_REQUEST = 20,
    MSG_GET_PROJECTS_RESPONSE = 21,
    MSG_CREATE_PROJECT_REQUEST = 22,
    MSG_CREATE_PROJECT_RESPONSE = 23,
    MSG_UPDATE_PROJECT_REQUEST = 24,
    MSG_UPDATE_PROJECT_RESPONSE = 25,
    MSG_DELETE_PROJECT_REQUEST = 26,
    MSG_DELETE_PROJECT_RESPONSE = 27,
    MSG_GET_PROJECT_DETAILS_REQUEST = 28,
    MSG_GET_PROJECT_DETAILS_RESPONSE = 29,
    MSG_INVITE_TO_PROJECT_REQUEST = 30,
    MSG_INVITE_TO_PROJECT_RESPONSE = 31,

    // Task management
    MSG_GET_TASKS_REQUEST = 40,
    MSG_GET_TASKS_RESPONSE = 41,
    MSG_CREATE_TASK_REQUEST = 42,
    MSG_CREATE_TASK_RESPONSE = 43,
    MSG_UPDATE_TASK_REQUEST = 44,
    MSG_UPDATE_TASK_RESPONSE = 45,
    MSG_DELETE_TASK_REQUEST = 46,
    MSG_DELETE_TASK_RESPONSE = 47,
    MSG_GET_TASK_COMMENTS_REQUEST = 48,
    MSG_GET_TASK_COMMENTS_RESPONSE = 49,
    MSG_ADD_TASK_COMMENT_REQUEST = 50,
    MSG_ADD_TASK_COMMENT_RESPONSE = 51,

    // Chat
    MSG_SEND_CHAT_REQUEST = 60,
    MSG_SEND_CHAT_RESPONSE = 61,
    MSG_GET_CHAT_HISTORY_REQUEST = 62,
    MSG_GET_CHAT_HISTORY_RESPONSE = 63,

    // File management
    MSG_UPLOAD_FILE_REQUEST = 80,
    MSG_UPLOAD_FILE_RESPONSE = 81,
    MSG_GET_FILES_REQUEST = 82,
    MSG_GET_FILES_RESPONSE = 83,

    // User/Contact management
    MSG_ADD_CONTACT_REQUEST = 100,
    MSG_ADD_CONTACT_RESPONSE = 101,
    MSG_GET_CONTACTS_REQUEST = 102,
    MSG_GET_CONTACTS_RESPONSE = 103,

    // Notifications
    MSG_GET_NOTIFICATIONS_REQUEST = 120,
    MSG_GET_NOTIFICATIONS_RESPONSE = 121,
    MSG_MARK_NOTIFICATION_READ_REQUEST = 122,
    MSG_MARK_NOTIFICATION_READ_RESPONSE = 123,

    // Error
    MSG_ERROR = 255
};

// Message header structure (fixed size)
struct MessageHeader {
    uint32_t type;          // Message type
    uint32_t payload_size;  // Size of JSON payload
    char version[8];        // Protocol version

    MessageHeader() : type(0), payload_size(0) {
        strncpy(version, PROTOCOL_VERSION, sizeof(version) - 1);
        version[sizeof(version) - 1] = '\0';
    }

    // Convert to network byte order
    void toNetwork() {
        type = htonl(type);
        payload_size = htonl(payload_size);
    }

    // Convert from network byte order
    void toHost() {
        type = ntohl(type);
        payload_size = ntohl(payload_size);
    }
};

// Status codes
enum StatusCode {
    STATUS_SUCCESS = 0,
    STATUS_ERROR = 1,
    STATUS_INVALID_CREDENTIALS = 2,
    STATUS_USER_NOT_FOUND = 3,
    STATUS_USER_ALREADY_EXISTS = 4,
    STATUS_INVALID_OTP = 5,
    STATUS_OTP_EXPIRED = 6,
    STATUS_UNAUTHORIZED = 7,
    STATUS_NOT_FOUND = 8,
    STATUS_INVALID_REQUEST = 9,
    STATUS_DATABASE_ERROR = 10,
    STATUS_EMAIL_SEND_ERROR = 11
};

// Helper functions
inline const char* getMessageTypeName(MessageType type) {
    switch(type) {
        case MSG_LOGIN_REQUEST: return "LOGIN_REQUEST";
        case MSG_LOGIN_RESPONSE: return "LOGIN_RESPONSE";
        case MSG_REGISTER_REQUEST: return "REGISTER_REQUEST";
        case MSG_REGISTER_RESPONSE: return "REGISTER_RESPONSE";
        case MSG_OTP_REQUEST: return "OTP_REQUEST";
        case MSG_OTP_RESPONSE: return "OTP_RESPONSE";
        case MSG_OTP_VERIFY_REQUEST: return "OTP_VERIFY_REQUEST";
        case MSG_OTP_VERIFY_RESPONSE: return "OTP_VERIFY_RESPONSE";
        case MSG_GET_PROJECTS_REQUEST: return "GET_PROJECTS_REQUEST";
        case MSG_GET_PROJECTS_RESPONSE: return "GET_PROJECTS_RESPONSE";
        case MSG_CREATE_PROJECT_REQUEST: return "CREATE_PROJECT_REQUEST";
        case MSG_CREATE_PROJECT_RESPONSE: return "CREATE_PROJECT_RESPONSE";
        case MSG_UPDATE_PROJECT_REQUEST: return "UPDATE_PROJECT_REQUEST";
        case MSG_UPDATE_PROJECT_RESPONSE: return "UPDATE_PROJECT_RESPONSE";
        case MSG_DELETE_PROJECT_REQUEST: return "DELETE_PROJECT_REQUEST";
        case MSG_DELETE_PROJECT_RESPONSE: return "DELETE_PROJECT_RESPONSE";
        case MSG_GET_TASKS_REQUEST: return "GET_TASKS_REQUEST";
        case MSG_GET_TASKS_RESPONSE: return "GET_TASKS_RESPONSE";
        case MSG_CREATE_TASK_REQUEST: return "CREATE_TASK_REQUEST";
        case MSG_CREATE_TASK_RESPONSE: return "CREATE_TASK_RESPONSE";
        case MSG_UPDATE_TASK_REQUEST: return "UPDATE_TASK_REQUEST";
        case MSG_UPDATE_TASK_RESPONSE: return "UPDATE_TASK_RESPONSE";
        case MSG_DELETE_TASK_REQUEST: return "DELETE_TASK_REQUEST";
        case MSG_DELETE_TASK_RESPONSE: return "DELETE_TASK_RESPONSE";
        case MSG_GET_TASK_COMMENTS_REQUEST: return "GET_TASK_COMMENTS_REQUEST";
        case MSG_GET_TASK_COMMENTS_RESPONSE: return "GET_TASK_COMMENTS_RESPONSE";
        case MSG_ADD_TASK_COMMENT_REQUEST: return "ADD_TASK_COMMENT_REQUEST";
        case MSG_ADD_TASK_COMMENT_RESPONSE: return "ADD_TASK_COMMENT_RESPONSE";
        case MSG_SEND_CHAT_REQUEST: return "SEND_CHAT_REQUEST";
        case MSG_SEND_CHAT_RESPONSE: return "SEND_CHAT_RESPONSE";
        case MSG_GET_CHAT_HISTORY_REQUEST: return "GET_CHAT_HISTORY_REQUEST";
        case MSG_GET_CHAT_HISTORY_RESPONSE: return "GET_CHAT_HISTORY_RESPONSE";
        case MSG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

inline const char* getStatusCodeName(StatusCode code) {
    switch(code) {
        case STATUS_SUCCESS: return "SUCCESS";
        case STATUS_ERROR: return "ERROR";
        case STATUS_INVALID_CREDENTIALS: return "INVALID_CREDENTIALS";
        case STATUS_USER_NOT_FOUND: return "USER_NOT_FOUND";
        case STATUS_USER_ALREADY_EXISTS: return "USER_ALREADY_EXISTS";
        case STATUS_INVALID_OTP: return "INVALID_OTP";
        case STATUS_OTP_EXPIRED: return "OTP_EXPIRED";
        case STATUS_UNAUTHORIZED: return "UNAUTHORIZED";
        case STATUS_NOT_FOUND: return "NOT_FOUND";
        case STATUS_INVALID_REQUEST: return "INVALID_REQUEST";
        case STATUS_DATABASE_ERROR: return "DATABASE_ERROR";
        case STATUS_EMAIL_SEND_ERROR: return "EMAIL_SEND_ERROR";
        default: return "UNKNOWN";
    }
}
