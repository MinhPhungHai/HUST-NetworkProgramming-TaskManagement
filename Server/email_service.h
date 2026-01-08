#pragma once

#include <string>
#include <cstdlib>
#include <random>
#include <sstream>
#include <iostream>

class EmailService {
private:
    std::string smtp_server;
    std::string smtp_user;
    std::string smtp_password;
    int smtp_port;

    // Generate random OTP code
    std::string generateOTP(int length = 6) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 9);

        std::string otp;
        for (int i = 0; i < length; i++) {
            otp += std::to_string(dis(gen));
        }
        return otp;
    }

public:
    EmailService(const std::string& server = "smtp.gmail.com",
                const std::string& user = "",
                const std::string& password = "",
                int port = 587)
        : smtp_server(server), smtp_user(user), smtp_password(password), smtp_port(port) {}

    // Send OTP email
    bool sendOTP(const std::string& to_email, const std::string& otp_code, const std::string& otp_type) {
        // For development/testing: just log the OTP
        // In production, implement actual email sending using libcurl or similar
        std::cout << "\n========== EMAIL SERVICE ==========" << std::endl;
        std::cout << "To: " << to_email << std::endl;
        std::cout << "Subject: Your OTP Code for " << otp_type << std::endl;
        std::cout << "OTP Code: " << otp_code << std::endl;
        std::cout << "This code will expire in 10 minutes." << std::endl;
        std::cout << "===================================\n" << std::endl;

        // TODO: Implement actual email sending in production
        // For now, we'll simulate successful email send
        return true;
    }

    // Generate and return new OTP code
    std::string createOTP() {
        return generateOTP(6);
    }

    // Validate OTP format
    bool isValidOTPFormat(const std::string& otp) {
        if (otp.length() != 6) return false;
        for (char c : otp) {
            if (!isdigit(c)) return false;
        }
        return true;
    }
};

/*
 * PRODUCTION EMAIL IMPLEMENTATION GUIDE:
 *
 * To implement actual email sending in production, you can use:
 * 1. libcurl with SMTP protocol
 * 2. External email service API (SendGrid, Mailgun, etc.)
 *
 * Example using libcurl:
 *
 * #include <curl/curl.h>
 *
 * bool sendEmailWithCURL(const std::string& to, const std::string& subject, const std::string& body) {
 *     CURL *curl;
 *     CURLcode res = CURLE_OK;
 *
 *     curl = curl_easy_init();
 *     if(curl) {
 *         curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
 *         curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
 *         curl_easy_setopt(curl, CURLOPT_USERNAME, smtp_user.c_str());
 *         curl_easy_setopt(curl, CURLOPT_PASSWORD, smtp_password.c_str());
 *         curl_easy_setopt(curl, CURLOPT_MAIL_FROM, smtp_user.c_str());
 *
 *         struct curl_slist *recipients = NULL;
 *         recipients = curl_slist_append(recipients, to.c_str());
 *         curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 *
 *         // Set email content
 *         std::string email_content = "To: " + to + "\r\n" +
 *                                    "Subject: " + subject + "\r\n\r\n" +
 *                                    body + "\r\n";
 *
 *         curl_easy_setopt(curl, CURLOPT_READDATA, &email_content);
 *
 *         res = curl_easy_perform(curl);
 *
 *         curl_slist_free_all(recipients);
 *         curl_easy_cleanup(curl);
 *
 *         return (res == CURLE_OK);
 *     }
 *     return false;
 * }
 *
 * Note: For Gmail, you need to:
 * - Enable 2-factor authentication
 * - Create an "App Password" for the application
 * - Use the app password instead of your regular Gmail password
 */
