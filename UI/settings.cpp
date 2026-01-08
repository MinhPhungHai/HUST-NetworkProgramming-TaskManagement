#include <gtk/gtk.h>
#include <stdio.h>
#include "app_state.h"
#include "settings.h"
#include "login.h"
#include "project_list.h"
#include "../Client/network_wrapper.h"

static void on_logout_confirm(GtkWidget *widget, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_YES) {
        char response[4096];
        if (network_logout(response, sizeof(response))) {
            if (json_get_status(response) != 0) {
                GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(settings_window),
                                                           GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                           "Logout failed on server.");
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
            }
        }

        current_username[0] = '\0';
        current_email[0] = '\0';
        current_user_id[0] = '\0';
        current_project_id[0] = '\0';
        current_project_name[0] = '\0';
        gtk_widget_hide(settings_window);
        gtk_widget_hide(project_list_window);
        show_login_screen();
    }

    gtk_widget_destroy(widget);
}

static void on_logout_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(settings_window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Do you want to logout?");
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO);
    g_signal_connect(dialog, "response", G_CALLBACK(on_logout_confirm), NULL);
    gtk_widget_show(dialog);
}

static void on_user_info_clicked(GtkWidget *widget, gpointer data) {
    show_user_info_dialog();
}

void show_settings_screen() {
    if (project_list_window) gtk_widget_hide(project_list_window);

    settings_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(settings_window), "Settings - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(settings_window), 1200, 720);
    gtk_window_set_position(GTK_WINDOW(settings_window), GTK_WIN_POS_CENTER);

    // Main container with centered alignment
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(settings_window), main_box);

    // Header area
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header_box), 20);

    GtkWidget *back_btn = gtk_button_new_with_label("← Back");
    gtk_widget_set_size_request(back_btn, 100, 35);
    g_signal_connect(back_btn, "clicked", G_CALLBACK(show_project_list_screen), NULL);
    gtk_box_pack_start(GTK_BOX(header_box), back_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 0);

    // Center container for content
    GtkWidget *center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_box), center_box, TRUE, TRUE, 0);

    // Content frame for card effect
    GtkWidget *content_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(content_frame), GTK_SHADOW_ETCHED_OUT);
    gtk_widget_set_size_request(content_frame, 600, 500);
    gtk_container_add(GTK_CONTAINER(center_box), content_frame);

    // Inner content box
    GtkWidget *content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_set_border_width(GTK_CONTAINER(content_box), 40);
    gtk_container_add(GTK_CONTAINER(content_frame), content_box);

    // Title
    GtkWidget *title = gtk_label_new("Settings");
    PangoFontDescription *title_font = pango_font_description_from_string("Sans Bold 28");
    gtk_widget_override_font(title, title_font);
    gtk_widget_set_margin_bottom(title, 30);
    gtk_box_pack_start(GTK_BOX(content_box), title, FALSE, FALSE, 0);

    // Account section
    GtkWidget *account_label = gtk_label_new("Account");
    PangoFontDescription *section_font = pango_font_description_from_string("Sans Bold 16");
    gtk_widget_override_font(account_label, section_font);
    gtk_label_set_xalign(GTK_LABEL(account_label), 0);
    gtk_widget_set_margin_bottom(account_label, 10);
    gtk_box_pack_start(GTK_BOX(content_box), account_label, FALSE, FALSE, 0);

    // User Information button
    GtkWidget *user_info_btn = gtk_button_new_with_label("User Information");
    gtk_widget_set_size_request(user_info_btn, -1, 50);
    g_signal_connect(user_info_btn, "clicked", G_CALLBACK(on_user_info_clicked), NULL);
    gtk_widget_set_margin_bottom(user_info_btn, 30);
    gtk_box_pack_start(GTK_BOX(content_box), user_info_btn, FALSE, FALSE, 0);

    // Actions section
    GtkWidget *actions_label = gtk_label_new("Actions");
    gtk_widget_override_font(actions_label, section_font);
    gtk_label_set_xalign(GTK_LABEL(actions_label), 0);
    gtk_widget_set_margin_bottom(actions_label, 10);
    gtk_box_pack_start(GTK_BOX(content_box), actions_label, FALSE, FALSE, 0);

    // Logout button
    GtkWidget *logout_btn = gtk_button_new_with_label("Logout");
    gtk_widget_set_size_request(logout_btn, -1, 50);
    g_signal_connect(logout_btn, "clicked", G_CALLBACK(on_logout_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(content_box), logout_btn, FALSE, FALSE, 0);

    // User info label at bottom
    GtkWidget *user_info = gtk_label_new("");
    char user_text[256];
    snprintf(user_text, sizeof(user_text), "Logged in as: %s",
             current_username[0] ? current_username : "Guest");
    gtk_label_set_text(GTK_LABEL(user_info), user_text);
    PangoFontDescription *small_font = pango_font_description_from_string("Sans 11");
    gtk_widget_override_font(user_info, small_font);
    gtk_widget_set_margin_top(user_info, 40);
    gtk_box_pack_start(GTK_BOX(content_box), user_info, FALSE, FALSE, 0);

    pango_font_description_free(title_font);
    pango_font_description_free(section_font);
    pango_font_description_free(small_font);

    g_signal_connect(settings_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(settings_window);
    gtk_window_present(GTK_WINDOW(settings_window));
}

void show_user_info_dialog() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("User Information",
        GTK_WINDOW(settings_window),
        GTK_DIALOG_MODAL,
        "Close", GTK_RESPONSE_CLOSE,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 450, 300);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_set_border_width(GTK_CONTAINER(box), 30);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // Username field
    GtkWidget *username_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *username_label = gtk_label_new("Username");
    PangoFontDescription *label_font = pango_font_description_from_string("Sans Bold 11");
    gtk_widget_override_font(username_label, label_font);
    gtk_label_set_xalign(GTK_LABEL(username_label), 0);

    GtkWidget *username_value = gtk_label_new(current_username[0] ? current_username : "Not set");
    PangoFontDescription *value_font = pango_font_description_from_string("Sans 13");
    gtk_widget_override_font(username_value, value_font);
    gtk_label_set_xalign(GTK_LABEL(username_value), 0);
    gtk_label_set_selectable(GTK_LABEL(username_value), TRUE);

    gtk_box_pack_start(GTK_BOX(username_box), username_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(username_box), username_value, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), username_box, FALSE, FALSE, 0);

    // Separator
    GtkWidget *sep1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(box), sep1, FALSE, FALSE, 5);

    // Email field
    GtkWidget *email_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *email_label = gtk_label_new("Email");
    gtk_widget_override_font(email_label, label_font);
    gtk_label_set_xalign(GTK_LABEL(email_label), 0);

    GtkWidget *email_value = gtk_label_new(current_email[0] ? current_email : "Not set");
    gtk_widget_override_font(email_value, value_font);
    gtk_label_set_xalign(GTK_LABEL(email_value), 0);
    gtk_label_set_selectable(GTK_LABEL(email_value), TRUE);

    gtk_box_pack_start(GTK_BOX(email_box), email_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(email_box), email_value, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), email_box, FALSE, FALSE, 0);

    // Separator
    GtkWidget *sep2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(box), sep2, FALSE, FALSE, 5);

    pango_font_description_free(label_font);
    pango_font_description_free(value_font);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
