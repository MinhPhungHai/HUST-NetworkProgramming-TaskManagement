#include <gtk/gtk.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include "app_state.h"
#include "register.h"
#include "login.h"
#include "../Client/network_wrapper.h"

static gboolean is_valid_email(const char *email) {
    regex_t regex;
    int ret;

    const char *pattern = "^[a-zA-Z0-9._%+\\-]+@[a-zA-Z0-9.\\-]+\\.[a-zA-Z]{2,}$";

    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
        return FALSE;
    }

    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);

    return (ret == 0) ? TRUE : FALSE;
}

static gboolean show_register_otp_dialog(GtkWidget *parent_window) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Email Verification",
        GTK_WINDOW(parent_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Verify", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);
    gtk_container_add(GTK_CONTAINER(content), box);

    // Info label with email
    gchar *info_text = g_strdup_printf(
        "OTP sent to: %s\n\nPlease enter the 6-digit code:",
        registered_email
    );
    GtkWidget *label = gtk_label_new(info_text);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 4);
    g_free(info_text);

    // OTP entry
    GtkWidget *otp_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(otp_entry), "000000");
    gtk_entry_set_max_length(GTK_ENTRY(otp_entry), 6);
    gtk_box_pack_start(GTK_BOX(box), otp_entry, FALSE, FALSE, 4);

    gtk_widget_show_all(dialog);

    gboolean success = FALSE;

    while (TRUE) {
        gint resp = gtk_dialog_run(GTK_DIALOG(dialog));

        if (resp == GTK_RESPONSE_ACCEPT) {
            const char *otp = gtk_entry_get_text(GTK_ENTRY(otp_entry));

            if (strlen(otp) == 0) {
                GtkWidget *err = gtk_message_dialog_new(
                    GTK_WINDOW(dialog),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "Please enter OTP");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                // Continue loop to allow retry
            } else {
                char response[4096];
                if (!network_verify_otp(registered_email, otp, "registration", response, sizeof(response))) {
                    GtkWidget *err = gtk_message_dialog_new(
                        GTK_WINDOW(dialog),
                        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                        "Failed to verify OTP (network error)");
                    gtk_dialog_run(GTK_DIALOG(err));
                    gtk_widget_destroy(err);
                    continue;
                }

                int status = json_get_status(response);
                if (status == 0) {
                    GtkWidget *success_dialog = gtk_message_dialog_new(
                        GTK_WINDOW(dialog),
                        GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                        "Registration successful! Please login.");
                    gtk_dialog_run(GTK_DIALOG(success_dialog));
                    gtk_widget_destroy(success_dialog);

                    gtk_widget_hide(register_window);
                    show_login_screen();
                    success = TRUE;
                    break;
                } else if (status == 5) {
                    GtkWidget *err = gtk_message_dialog_new(
                        GTK_WINDOW(dialog),
                        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                        "Invalid OTP. Please try again.");
                    gtk_dialog_run(GTK_DIALOG(err));
                    gtk_widget_destroy(err);
                } else if (status == 6) {
                    GtkWidget *err = gtk_message_dialog_new(
                        GTK_WINDOW(dialog),
                        GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                        "OTP expired! Please request OTP again.");
                    gtk_dialog_run(GTK_DIALOG(err));
                    gtk_widget_destroy(err);
                    break;
                } else {
                GtkWidget *err = gtk_message_dialog_new(
                    GTK_WINDOW(dialog),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "OTP verification failed.");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                }
            }
        } else {
            // User clicked Cancel
            break;
        }
    }

    gtk_widget_destroy(dialog);
    return success;
}

static void on_register_otp_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *username_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "username"));
    GtkWidget *email_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "email"));
    GtkWidget *password_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "password"));
    GtkWidget *confirm_entry = GTK_WIDGET(g_object_get_data(G_OBJECT(data), "confirm"));

    const char *username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    const char *email = gtk_entry_get_text(GTK_ENTRY(email_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(password_entry));
    const char *confirm = gtk_entry_get_text(GTK_ENTRY(confirm_entry));

    if (strlen(username) == 0 || strlen(email) == 0 || strlen(password) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(register_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Please fill all fields");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Validate email format
    if (!is_valid_email(email)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(register_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Invalid email format. Please enter a valid email address.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    if (strcmp(password, confirm) != 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(register_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Passwords do not match");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    if (!network_is_connected()) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(register_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Not connected to server!\nMake sure server is running on 127.0.0.1:8080");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Store registration data
    g_strlcpy(current_username, username, sizeof(current_username));
    g_strlcpy(current_email, email, sizeof(current_email));

    char response[4096];
    if (!network_register(username, email, password, response, sizeof(response))) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(register_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Failed to send registration request (network error).");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    int status = json_get_status(response);
    if (status != 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(register_window),
                                                   GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                   "Registration failed. Please check your info.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    char server_email[256] = "";
    if (json_get_string(response, "email", server_email, sizeof(server_email))) {
        g_strlcpy(registered_email, server_email, sizeof(registered_email));
    } else {
        g_strlcpy(registered_email, email, sizeof(registered_email));
    }

    // Show OTP dialog
    show_register_otp_dialog(register_window);
}

static gboolean on_register_link_clicked(GtkWidget *widget, gpointer data) {
    gtk_widget_hide(register_window);
    show_login_screen();
    return TRUE;
}

void show_register_screen() {
    if (login_window) gtk_widget_hide(login_window);

    register_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(register_window), "Register - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(register_window), 620, 650);
    gtk_window_set_position(GTK_WINDOW(register_window), GTK_WIN_POS_CENTER);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 30);
    gtk_container_add(GTK_CONTAINER(register_window), box);

    // Title
    GtkWidget *title = gtk_label_new("Create Account");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 20");
    gtk_widget_override_font(title, font_desc);
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 10);

    // Username entry
    GtkWidget *username_label = gtk_label_new("Username:");
    gtk_label_set_xalign(GTK_LABEL(username_label), 0);
    gtk_box_pack_start(GTK_BOX(box), username_label, FALSE, FALSE, 5);

    GtkWidget *username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(username_entry), "Enter username");
    gtk_box_pack_start(GTK_BOX(box), username_entry, FALSE, FALSE, 5);

    // Email entry
    GtkWidget *email_label = gtk_label_new("Email:");
    gtk_label_set_xalign(GTK_LABEL(email_label), 0);
    gtk_box_pack_start(GTK_BOX(box), email_label, FALSE, FALSE, 5);

    GtkWidget *email_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(email_entry), "Enter email");
    gtk_box_pack_start(GTK_BOX(box), email_entry, FALSE, FALSE, 5);

    // Password entry
    GtkWidget *password_label = gtk_label_new("Password:");
    gtk_label_set_xalign(GTK_LABEL(password_label), 0);
    gtk_box_pack_start(GTK_BOX(box), password_label, FALSE, FALSE, 5);

    GtkWidget *password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(password_entry), FALSE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(password_entry), "Enter password");
    gtk_box_pack_start(GTK_BOX(box), password_entry, FALSE, FALSE, 5);

    // Confirm password entry
    GtkWidget *confirm_label = gtk_label_new("Confirm Password:");
    gtk_label_set_xalign(GTK_LABEL(confirm_label), 0);
    gtk_box_pack_start(GTK_BOX(box), confirm_label, FALSE, FALSE, 5);

    GtkWidget *confirm_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(confirm_entry), FALSE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(confirm_entry), "Confirm password");
    gtk_box_pack_start(GTK_BOX(box), confirm_entry, FALSE, FALSE, 5);

    // Buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(button_box), TRUE);

    GtkWidget *request_otp_btn = gtk_button_new_with_label("Request OTP");

    // Store references
    g_object_set_data(G_OBJECT(request_otp_btn), "username", username_entry);
    g_object_set_data(G_OBJECT(request_otp_btn), "email", email_entry);
    g_object_set_data(G_OBJECT(request_otp_btn), "password", password_entry);
    g_object_set_data(G_OBJECT(request_otp_btn), "confirm", confirm_entry);

    g_signal_connect(request_otp_btn, "clicked", G_CALLBACK(on_register_otp_clicked), request_otp_btn);

    gtk_box_pack_start(GTK_BOX(button_box), request_otp_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), button_box, FALSE, FALSE, 10);

    // Login link
    GtkWidget *login_link = gtk_link_button_new_with_label("", "Already have an account? Login");
    g_signal_connect(login_link, "activate-link", G_CALLBACK(on_register_link_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), login_link, FALSE, FALSE, 5);

    g_signal_connect(register_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(register_window);
}
