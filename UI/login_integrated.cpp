#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include "app_state.h"
#include "register.h"
#include "project_list.h"
#include "../Client/network_wrapper.h"
#include "../Common/protocol.h"

struct ResetPasswordWidgets {
    GtkWidget *new_entry;
    GtkWidget *confirm_entry;
};

static std::string trim_copy(const std::string &value) {
    size_t start = value.find_first_not_of(" \t\n\r");
    size_t end = value.find_last_not_of(" \t\n\r");
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }
    return value.substr(start, end - start + 1);
}

static char temp_login_email[256] = "";  // Store email from login response

static void show_otp_dialog() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Enter OTP",
        GTK_WINDOW(login_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Verify", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);
    gtk_container_add(GTK_CONTAINER(content), box);

    char msg[512];
    snprintf(msg, sizeof(msg), "OTP sent to: %s\nCheck SERVER terminal for OTP code!", temp_login_email);
    GtkWidget *label = gtk_label_new(msg);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 4);

    GtkWidget *otp_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(otp_entry), "Enter OTP (6 digits)");
    gtk_entry_set_max_length(GTK_ENTRY(otp_entry), 6);
    gtk_box_pack_start(GTK_BOX(box), otp_entry, FALSE, FALSE, 4);

    gtk_widget_show_all(dialog);
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));

    if (resp == GTK_RESPONSE_ACCEPT) {
        const char *otp = gtk_entry_get_text(GTK_ENTRY(otp_entry));

        if (strlen(otp) == 0) {
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                    "Please enter OTP");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            gtk_widget_destroy(dialog);
            return;
        }

        // Verify OTP with server
        char response[4096];
        if (!network_verify_otp(temp_login_email, otp, "login", response, sizeof(response))) {
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                    "Failed to verify OTP (network error)");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            gtk_widget_destroy(dialog);
            return;
        }

        // Check status
        int status = json_get_status(response);

        if (status == 0) {  // STATUS_SUCCESS
            // Parse user info
            json_get_string(response, "user_id", current_user_id, sizeof(current_user_id));
            json_get_string(response, "username", current_username, sizeof(current_username));
            json_get_string(response, "email", current_email, sizeof(current_email));

            printf("Login successful! User: %s (ID: %s)\n", current_username, current_user_id);

            // Success - go to project list
            gtk_widget_hide(login_window);
            gtk_widget_destroy(dialog);
            show_project_list_screen();
            return;
        } else if (status == 5) {  // STATUS_INVALID_OTP
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                    "Invalid OTP code!");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else if (status == 6) {  // STATUS_OTP_EXPIRED
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                    "OTP expired! Please login again.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else {
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                    "OTP verification failed!");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        }
    }

    gtk_widget_destroy(dialog);
}

static void on_login_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *username_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "username"));
    GtkWidget *password_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "password"));

    const char *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(password_entry));

    if (strlen(username) == 0 || strlen(password) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(login_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Please enter username/email and password");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Check network connection
    if (!network_is_connected()) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(login_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Not connected to server!\nMake sure server is running on 127.0.0.1:8080");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Send login request to server
    char response[4096];
    if (!network_login(username, password, response, sizeof(response))) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(login_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Failed to send login request (network error)");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Parse response
    int status = json_get_status(response);

    if (status == 0) {  // STATUS_SUCCESS
        // Get email from response (OTP was sent to this email)
        json_get_string(response, "email", temp_login_email, sizeof(temp_login_email));

        // Store username temporarily
        g_strlcpy(current_username, username, sizeof(current_username));

        printf("Login request successful. OTP sent to: %s\n", temp_login_email);
        printf("CHECK SERVER TERMINAL FOR OTP CODE!\n");

        // Show OTP dialog
        show_otp_dialog();
    } else if (status == 2) {  // STATUS_INVALID_CREDENTIALS
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(login_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Invalid username/email or password!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else if (status == 3) {  // STATUS_USER_NOT_FOUND
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(login_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "User not found! Please register first.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "Login failed (status: %d)", status);
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(login_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "%s", err_msg);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

static void on_toggle_reset_password(GtkToggleButton *toggle, gpointer data) {
    ResetPasswordWidgets *widgets = static_cast<ResetPasswordWidgets*>(data);
    if (!widgets) {
        return;
    }
    gboolean visible = gtk_toggle_button_get_active(toggle);
    gtk_entry_set_visibility(GTK_ENTRY(widgets->new_entry), visible);
    gtk_entry_set_visibility(GTK_ENTRY(widgets->confirm_entry), visible);
}

static void show_forgot_password_dialog() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Reset Password",
        GTK_WINDOW(login_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);
    gtk_container_add(GTK_CONTAINER(content), box);

    GtkWidget *user_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(user_entry), "Username");
    gtk_box_pack_start(GTK_BOX(box), user_entry, FALSE, FALSE, 0);

    GtkWidget *email_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(email_entry), "Email");
    gtk_box_pack_start(GTK_BOX(box), email_entry, FALSE, FALSE, 0);

    GtkWidget *new_pw_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(new_pw_entry), "New password");
    gtk_entry_set_visibility(GTK_ENTRY(new_pw_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(box), new_pw_entry, FALSE, FALSE, 0);

    GtkWidget *confirm_pw_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(confirm_pw_entry), "Retype new password");
    gtk_entry_set_visibility(GTK_ENTRY(confirm_pw_entry), FALSE);
    gtk_box_pack_start(GTK_BOX(box), confirm_pw_entry, FALSE, FALSE, 0);

    GtkWidget *show_toggle = gtk_check_button_new_with_label("Show password");
    gtk_box_pack_start(GTK_BOX(box), show_toggle, FALSE, FALSE, 0);

    ResetPasswordWidgets *pw_widgets = new ResetPasswordWidgets();
    pw_widgets->new_entry = new_pw_entry;
    pw_widgets->confirm_entry = confirm_pw_entry;
    g_signal_connect(show_toggle, "toggled", G_CALLBACK(on_toggle_reset_password), pw_widgets);
    g_object_set_data_full(G_OBJECT(dialog), "reset_pw_widgets", pw_widgets,
                           [](gpointer data) { delete static_cast<ResetPasswordWidgets*>(data); });

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        std::string username = trim_copy(gtk_entry_get_text(GTK_ENTRY(user_entry)));
        std::string email = trim_copy(gtk_entry_get_text(GTK_ENTRY(email_entry)));
        const char *new_pw = gtk_entry_get_text(GTK_ENTRY(new_pw_entry));
        const char *confirm_pw = gtk_entry_get_text(GTK_ENTRY(confirm_pw_entry));

        if (username.empty() || email.empty() || !new_pw || !confirm_pw ||
            strlen(new_pw) == 0 || strlen(confirm_pw) == 0) {
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                    "Please fill all required fields.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else if (strcmp(new_pw, confirm_pw) != 0) {
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                    "Passwords do not match.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else if (!network_is_connected()) {
            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                    "Not connected to server.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else {
            char response[4096];
            if (!network_reset_password(username.c_str(), email.c_str(), response, sizeof(response))) {
                GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                        "Failed to request reset (network error).");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            } else {
                int status = json_get_status(response);
                if (status == STATUS_SUCCESS) {
                    char otp_email_buf[256] = "";
                    json_get_string(response, "email", otp_email_buf, sizeof(otp_email_buf));
                    std::string otp_email = trim_copy(otp_email_buf);
                    if (otp_email.empty()) {
                        otp_email = email;
                    }
                    GtkWidget *otp_dialog = gtk_dialog_new_with_buttons(
                        "Enter OTP",
                        GTK_WINDOW(dialog),
                        GTK_DIALOG_MODAL,
                        "Cancel", GTK_RESPONSE_CANCEL,
                        "Verify", GTK_RESPONSE_ACCEPT,
                        NULL);

                    GtkWidget *otp_content = gtk_dialog_get_content_area(GTK_DIALOG(otp_dialog));
                    GtkWidget *otp_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
                    gtk_container_set_border_width(GTK_CONTAINER(otp_box), 12);
                    gtk_container_add(GTK_CONTAINER(otp_content), otp_box);

                    char msg[256];
                    snprintf(msg, sizeof(msg), "OTP sent to: %s", otp_email.c_str());
                    GtkWidget *otp_label = gtk_label_new(msg);
                    gtk_label_set_line_wrap(GTK_LABEL(otp_label), TRUE);
                    gtk_box_pack_start(GTK_BOX(otp_box), otp_label, FALSE, FALSE, 0);

                    GtkWidget *otp_entry = gtk_entry_new();
                    gtk_entry_set_placeholder_text(GTK_ENTRY(otp_entry), "Enter OTP");
                    gtk_entry_set_max_length(GTK_ENTRY(otp_entry), 6);
                    gtk_box_pack_start(GTK_BOX(otp_box), otp_entry, FALSE, FALSE, 0);

                    gtk_widget_show_all(otp_dialog);
                    if (gtk_dialog_run(GTK_DIALOG(otp_dialog)) == GTK_RESPONSE_ACCEPT) {
                        const char *otp_code = gtk_entry_get_text(GTK_ENTRY(otp_entry));
                        if (!otp_code || strlen(otp_code) == 0) {
                            GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(otp_dialog),
                                                                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                                    "Please enter OTP.");
                            gtk_dialog_run(GTK_DIALOG(err));
                            gtk_widget_destroy(err);
                        } else {
                            char confirm_resp[4096];
                            std::string otp_trim = trim_copy(otp_code);
                            if (!network_confirm_reset_password(otp_email.c_str(), otp_trim.c_str(), new_pw, confirm_resp, sizeof(confirm_resp))) {
                                GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(otp_dialog),
                                                                        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                                        "Failed to reset password (network error).");
                                gtk_dialog_run(GTK_DIALOG(err));
                                gtk_widget_destroy(err);
                            } else {
                                int confirm_status = json_get_status(confirm_resp);
                                if (confirm_status == STATUS_SUCCESS) {
                                    GtkWidget *ok = gtk_message_dialog_new(GTK_WINDOW(otp_dialog),
                                                                           GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                                           "Password updated successfully.");
                                    gtk_dialog_run(GTK_DIALOG(ok));
                                    gtk_widget_destroy(ok);
                                } else if (confirm_status == STATUS_INVALID_OTP) {
                                    GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(otp_dialog),
                                                                            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                                            "Invalid OTP.");
                                    gtk_dialog_run(GTK_DIALOG(err));
                                    gtk_widget_destroy(err);
                                } else if (confirm_status == STATUS_OTP_EXPIRED) {
                                    GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(otp_dialog),
                                                                            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                                            "OTP expired.");
                                    gtk_dialog_run(GTK_DIALOG(err));
                                    gtk_widget_destroy(err);
                                } else {
                                    GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(otp_dialog),
                                                                            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                                            "Failed to reset password.");
                                    gtk_dialog_run(GTK_DIALOG(err));
                                    gtk_widget_destroy(err);
                                }
                            }
                        }
                    }

                    gtk_widget_destroy(otp_dialog);
                } else if (status == STATUS_USER_NOT_FOUND) {
                    GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                            "User not found or email mismatch.");
                    gtk_dialog_run(GTK_DIALOG(err));
                    gtk_widget_destroy(err);
                } else {
                    GtkWidget *err = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                            "Failed to reset password.");
                    gtk_dialog_run(GTK_DIALOG(err));
                    gtk_widget_destroy(err);
                }
            }
        }
    }

    gtk_widget_destroy(dialog);
}

static gboolean on_forgot_password_clicked(GtkWidget *widget, gpointer data) {
    show_forgot_password_dialog();
    return TRUE;
}

static gboolean on_login_link_clicked(GtkWidget *widget, gpointer data) {
    if (login_window) gtk_widget_hide(login_window);
    show_register_screen();
    return TRUE;
}

void show_login_screen() {
    login_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(login_window), "Login - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(login_window), 620, 650);
    gtk_window_set_position(GTK_WINDOW(login_window), GTK_WIN_POS_CENTER);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 30);
    gtk_container_add(GTK_CONTAINER(login_window), box);

    // Title
    GtkWidget *title = gtk_label_new("Project Management System");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 20");
    gtk_widget_override_font(title, font_desc);
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 10);

    // Connection status
    char conn_status[128];
    if (network_is_connected()) {
        snprintf(conn_status, sizeof(conn_status), "✓ Connected to server (127.0.0.1:8080)");
    } else {
        snprintf(conn_status, sizeof(conn_status), "✗ Not connected to server!");
    }
    GtkWidget *status_label = gtk_label_new(conn_status);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 5);

    // Username/Email entry
    GtkWidget *username_label = gtk_label_new("Username or Email:");
    gtk_label_set_xalign(GTK_LABEL(username_label), 0);
    gtk_box_pack_start(GTK_BOX(box), username_label, FALSE, FALSE, 5);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Enter username or email");
    gtk_box_pack_start(GTK_BOX(box), username_entry, FALSE, FALSE, 5);

    // Password entry
    GtkWidget *password_label = gtk_label_new("Password:");
    gtk_label_set_xalign(GTK_LABEL(password_label), 0);
    gtk_box_pack_start(GTK_BOX(box), password_label, FALSE, FALSE, 5);

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Enter password");
    gtk_box_pack_start(GTK_BOX(box), password_entry, FALSE, FALSE, 5);

    // Info hint about OTP delivery
    GtkWidget *otp_hint = gtk_label_new("⚠ After clicking Login, check the SERVER TERMINAL for your OTP code!\nOTP will be sent to the email linked to this account.");
    gtk_label_set_line_wrap(GTK_LABEL(otp_hint), TRUE);
    gtk_label_set_xalign(GTK_LABEL(otp_hint), 0);
    gtk_box_pack_start(GTK_BOX(box), otp_hint, FALSE, FALSE, 10);

    // Buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(button_box), TRUE);

    // Login button
    GtkWidget *login_btn = gtk_button_new_with_label("Login with OTP");
    gtk_button_set_relief(GTK_BUTTON(login_btn), GTK_RELIEF_NONE);

    // Store references for callbacks
    g_object_set_data(G_OBJECT(login_btn), "username", username_entry);
    g_object_set_data(G_OBJECT(login_btn), "password", password_entry);

    g_signal_connect(login_btn, "clicked", G_CALLBACK(on_login_clicked), login_btn);

    gtk_box_pack_start(GTK_BOX(button_box), login_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), button_box, FALSE, FALSE, 10);

    // Register link
    GtkWidget *register_link = gtk_link_button_new_with_label("", "Don't have an account? Register");
    g_signal_connect(register_link, "activate-link", G_CALLBACK(on_login_link_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), register_link, FALSE, FALSE, 5);

    GtkWidget *forgot_link = gtk_link_button_new_with_label("", "Forgot your password?");
    g_signal_connect(forgot_link, "activate-link", G_CALLBACK(on_forgot_password_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), forgot_link, FALSE, FALSE, 5);

    g_signal_connect(login_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(login_window);
    gtk_window_present(GTK_WINDOW(login_window));
}
