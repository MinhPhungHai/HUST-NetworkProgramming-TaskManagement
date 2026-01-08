#pragma once

#include "database_handler.h"
#include "email_service.h"
#include "../Common/protocol.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <ctime>

class AuthService {
private:
    DatabaseHandler& db;
    EmailService& email_service;

    // Hash password using SHA256
    std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);

        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    // Get timestamp for OTP expiration (10 minutes from now)
    std::string getOTPExpiryTimestamp() {
        time_t now = time(nullptr);
        now += 600; // 10 minutes
        struct tm* tm_info = localtime(&now);
        char buffer[26];
        strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        return std::string(buffer);
    }

    // Check if user exists by username or email
    bool userExists(const std::string& username_or_email, std::string& user_id, std::string& email) {
        try {
            std::string query = "SELECT user_id, email FROM users WHERE username = " +
                              db.escapeString(username_or_email) + " OR email = " +
                              db.escapeString(username_or_email);

            PGresult* res = db.executeQuery(query);
            int rows = PQntuples(res);

            if (rows > 0) {
                user_id = PQgetvalue(res, 0, 0);
                email = PQgetvalue(res, 0, 1);
                PQclear(res);
                return true;
            }

            PQclear(res);
            return false;
        } catch (const std::exception& e) {
            return false;
        }
    }

public:
    AuthService(DatabaseHandler& database, EmailService& email)
        : db(database), email_service(email) {}

    // Register new user
    StatusCode registerUser(const std::string& username, const std::string& email,
                          const std::string& password, std::string& user_id_out) {
        try {
            // Check if username already exists
            std::string existing_id, existing_email;
            if (userExists(username, existing_id, existing_email)) {
                return STATUS_USER_ALREADY_EXISTS;
            }

            // Check if email already exists
            std::string query = "SELECT user_id FROM users WHERE email = " + db.escapeString(email);
            PGresult* res = db.executeQuery(query);
            int rows = PQntuples(res);
            PQclear(res);

            if (rows > 0) {
                return STATUS_USER_ALREADY_EXISTS;
            }

            // Generate user ID
            std::string user_id = db.generateUUID();

            // Hash password
            std::string password_hash = hashPassword(password);

            // Insert new user (initially not verified)
            query = "INSERT INTO users (user_id, username, email, password_hash, is_verified, created_at) "
                   "VALUES (" + db.escapeString(user_id) + ", " +
                   db.escapeString(username) + ", " +
                   db.escapeString(email) + ", " +
                   db.escapeString(password_hash) + ", false, NOW())";

            db.executeQuery(query);
            user_id_out = user_id;

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Register error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Generate and send OTP
    StatusCode sendOTP(const std::string& username_or_email, const std::string& otp_type,
                     std::string& otp_id_out) {
        try {
            std::string user_id, email;

            // For registration, email might not be in database yet
            if (otp_type == "registration") {
                // Assume username_or_email is the email for registration
                email = username_or_email;
                user_id = ""; // Will be set after registration
            } else {
                // For login, find user
                if (!userExists(username_or_email, user_id, email)) {
                    return STATUS_USER_NOT_FOUND;
                }
            }

            // Generate OTP
            std::string otp_code = email_service.createOTP();

            // Generate OTP ID
            std::string otp_id = db.generateUUID();

            // Store OTP in database
            std::string expiry = getOTPExpiryTimestamp();
            std::string query = "INSERT INTO otp_verifications (otp_id, user_id, email, otp_code, otp_type, expires_at, is_used, created_at) "
                              "VALUES (" + db.escapeString(otp_id) + ", " +
                              (user_id.empty() ? "NULL" : db.escapeString(user_id)) + ", " +
                              db.escapeString(email) + ", " +
                              db.escapeString(otp_code) + ", " +
                              db.escapeString(otp_type) + ", " +
                              db.escapeString(expiry) + ", false, NOW())";

            db.executeQuery(query);

            // Send OTP email
            if (!email_service.sendOTP(email, otp_code, otp_type)) {
                return STATUS_EMAIL_SEND_ERROR;
            }

            otp_id_out = otp_id;
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Send OTP error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Verify OTP
    StatusCode verifyOTP(const std::string& email, const std::string& otp_code,
                       const std::string& otp_type, std::string& user_id_out) {
        try {
            // Find OTP record
            std::string query = "SELECT otp_id, user_id, expires_at, is_used FROM otp_verifications "
                              "WHERE email = " + db.escapeString(email) +
                              " AND otp_code = " + db.escapeString(otp_code) +
                              " AND otp_type = " + db.escapeString(otp_type) +
                              " ORDER BY created_at DESC LIMIT 1";

            PGresult* res = db.executeQuery(query);
            int rows = PQntuples(res);

            if (rows == 0) {
                PQclear(res);
                return STATUS_INVALID_OTP;
            }

            std::string otp_id = PQgetvalue(res, 0, 0);
            std::string user_id = PQgetvalue(res, 0, 1) ? PQgetvalue(res, 0, 1) : "";
            std::string expires_at = PQgetvalue(res, 0, 2);
            bool is_used = (std::string(PQgetvalue(res, 0, 3)) == "t");

            PQclear(res);

            // Check if already used
            if (is_used) {
                return STATUS_INVALID_OTP;
            }

            // Check if expired
            time_t now = time(nullptr);
            struct tm tm_expiry = {};
            strptime(expires_at.c_str(), "%Y-%m-%d %H:%M:%S", &tm_expiry);
            time_t expiry_time = mktime(&tm_expiry);

            if (now > expiry_time) {
                return STATUS_OTP_EXPIRED;
            }

            // Mark OTP as used
            query = "UPDATE otp_verifications SET is_used = true WHERE otp_id = " +
                   db.escapeString(otp_id);
            db.executeQuery(query);

            // If registration OTP, mark user as verified
            if (otp_type == "registration" || otp_type == "login") {
                // Find user by email and mark as verified
                query = "UPDATE users SET is_verified = true WHERE email = " +
                       db.escapeString(email) + " RETURNING user_id";
                res = db.executeQuery(query);
                if (PQntuples(res) > 0) {
                    user_id_out = PQgetvalue(res, 0, 0);
                }
                PQclear(res);
            } else {
                user_id_out = user_id;
            }

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Verify OTP error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Login user (check credentials before sending OTP)
    StatusCode checkCredentials(const std::string& username_or_email, const std::string& password,
                              std::string& user_id_out, std::string& email_out) {
        try {
            std::string user_id, email;
            if (!userExists(username_or_email, user_id, email)) {
                return STATUS_USER_NOT_FOUND;
            }

            // Get password hash
            std::string query = "SELECT password_hash FROM users WHERE user_id = " +
                              db.escapeString(user_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_USER_NOT_FOUND;
            }

            std::string stored_hash = PQgetvalue(res, 0, 0);
            PQclear(res);

            // Verify password
            std::string input_hash = hashPassword(password);
            if (input_hash != stored_hash) {
                return STATUS_INVALID_CREDENTIALS;
            }

            user_id_out = user_id;
            email_out = email;
            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Check credentials error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Change password for logged-in user
    StatusCode changePassword(const std::string& user_id, const std::string& old_password,
                             const std::string& new_password) {
        try {
            std::string query = "SELECT password_hash FROM users WHERE user_id = " +
                              db.escapeString(user_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_USER_NOT_FOUND;
            }

            std::string stored_hash = PQgetvalue(res, 0, 0);
            PQclear(res);

            std::string old_hash = hashPassword(old_password);
            if (old_hash != stored_hash) {
                return STATUS_INVALID_CREDENTIALS;
            }

            std::string new_hash = hashPassword(new_password);
            query = "UPDATE users SET password_hash = " + db.escapeString(new_hash) +
                   ", updated_at = NOW() WHERE user_id = " + db.escapeString(user_id);
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Change password error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Reset password using username + email (name optional)
    StatusCode resetPassword(const std::string& username, const std::string& email,
                            const std::string& name, const std::string& new_password) {
        try {
            if (!name.empty() && name != username) {
                return STATUS_INVALID_REQUEST;
            }

            std::string query = "SELECT user_id FROM users WHERE username = " +
                              db.escapeString(username) + " AND email = " +
                              db.escapeString(email);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_USER_NOT_FOUND;
            }

            std::string user_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            std::string new_hash = hashPassword(new_password);
            query = "UPDATE users SET password_hash = " + db.escapeString(new_hash) +
                   ", updated_at = NOW() WHERE user_id = " + db.escapeString(user_id);
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Reset password error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    StatusCode requestPasswordReset(const std::string& username, const std::string& email,
                                   std::string& otp_id_out) {
        try {
            std::string query = "SELECT user_id FROM users WHERE username = " +
                              db.escapeString(username) + " AND email = " +
                              db.escapeString(email);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_USER_NOT_FOUND;
            }
            PQclear(res);

            return sendOTP(email, "reset_password", otp_id_out);
        } catch (const std::exception& e) {
            std::cerr << "Request reset error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    StatusCode resetPasswordWithOTP(const std::string& email, const std::string& otp_code,
                                   const std::string& new_password) {
        try {
            std::string query = "SELECT otp_id, expires_at, is_used FROM otp_verifications "
                              "WHERE email = " + db.escapeString(email) +
                              " AND otp_code = " + db.escapeString(otp_code) +
                              " AND otp_type = " + db.escapeString("reset_password") +
                              " ORDER BY created_at DESC LIMIT 1";

            PGresult* res = db.executeQuery(query);
            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_INVALID_OTP;
            }

            std::string otp_id = PQgetvalue(res, 0, 0);
            std::string expires_at = PQgetvalue(res, 0, 1);
            bool is_used = (std::string(PQgetvalue(res, 0, 2)) == "t");
            PQclear(res);

            if (is_used) {
                return STATUS_INVALID_OTP;
            }

            time_t now = time(nullptr);
            struct tm tm_expiry = {};
            strptime(expires_at.c_str(), "%Y-%m-%d %H:%M:%S", &tm_expiry);
            time_t expiry_time = mktime(&tm_expiry);

            if (now > expiry_time) {
                return STATUS_OTP_EXPIRED;
            }

            query = "UPDATE otp_verifications SET is_used = true WHERE otp_id = " +
                   db.escapeString(otp_id);
            db.executeQuery(query);

            query = "SELECT user_id FROM users WHERE email = " + db.escapeString(email);
            res = db.executeQuery(query);
            if (PQntuples(res) == 0) {
                PQclear(res);
                return STATUS_USER_NOT_FOUND;
            }
            std::string user_id = PQgetvalue(res, 0, 0);
            PQclear(res);

            std::string new_hash = hashPassword(new_password);
            query = "UPDATE users SET password_hash = " + db.escapeString(new_hash) +
                   ", updated_at = NOW() WHERE user_id = " + db.escapeString(user_id);
            db.executeQuery(query);

            return STATUS_SUCCESS;
        } catch (const std::exception& e) {
            std::cerr << "Reset password OTP error: " << e.what() << std::endl;
            return STATUS_DATABASE_ERROR;
        }
    }

    // Get user info
    bool getUserInfo(const std::string& user_id, std::string& username,
                    std::string& email, bool& is_verified) {
        try {
            std::string query = "SELECT username, email, is_verified FROM users WHERE user_id = " +
                              db.escapeString(user_id);
            PGresult* res = db.executeQuery(query);

            if (PQntuples(res) == 0) {
                PQclear(res);
                return false;
            }

            username = PQgetvalue(res, 0, 0);
            email = PQgetvalue(res, 0, 1);
            is_verified = (std::string(PQgetvalue(res, 0, 2)) == "t");

            PQclear(res);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Get user info error: " << e.what() << std::endl;
            return false;
        }
    }
};
