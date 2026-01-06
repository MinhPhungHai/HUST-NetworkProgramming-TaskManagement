#include <gtk/gtk.h>
#include <stdio.h>
#include "app_state.h"
#include "settings.h"
#include "login.h"
#include "project_list.h"

static void on_logout_confirm(GtkWidget *widget, gint response_id, gpointer data) {
    if (response_id == GTK_RESPONSE_YES) {
        gtk_widget_hide(settings_window);
        gtk_widget_hide(project_list_window);
        show_login_screen();
    }
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
    gtk_window_set_default_size(GTK_WINDOW(settings_window), 1400, 800);
    gtk_window_set_position(GTK_WINDOW(settings_window), GTK_WIN_POS_CENTER);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 30);
    gtk_container_add(GTK_CONTAINER(settings_window), box);

    GtkWidget *title = gtk_label_new("Settings");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 24");
    gtk_widget_override_font(title, font_desc);
    gtk_box_pack_start(GTK_BOX(box), title, FALSE, FALSE, 20);

    // User Information button
    GtkWidget *user_info_btn = gtk_button_new_with_label("👤 User Information");
    gtk_widget_set_size_request(user_info_btn, 300, 50);
    g_signal_connect(user_info_btn, "clicked", G_CALLBACK(on_user_info_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), user_info_btn, FALSE, FALSE, 10);

    // Logout button
    GtkWidget *logout_btn = gtk_button_new_with_label("🚪 Logout");
    gtk_widget_set_size_request(logout_btn, 300, 50);
    g_signal_connect(logout_btn, "clicked", G_CALLBACK(on_logout_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(box), logout_btn, FALSE, FALSE, 10);

    // Back button
    GtkWidget *back_btn = gtk_button_new_with_label("← Back");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(show_project_list_screen), NULL);
    gtk_box_pack_start(GTK_BOX(box), back_btn, FALSE, FALSE, 20);

    g_signal_connect(settings_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(settings_window);
}

void show_user_info_dialog() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("User Information",
        GTK_WINDOW(settings_window),
        GTK_DIALOG_MODAL,
        "Close", GTK_RESPONSE_CLOSE,
        NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    char info_text[512];
    snprintf(info_text, sizeof(info_text),
        "Username: %s\nEmail: %s\nPassword: ********",
        current_username[0] ? current_username : "Not set",
        current_email[0] ? current_email : "Not set");

    GtkWidget *info_label = gtk_label_new(info_text);
    gtk_label_set_selectable(GTK_LABEL(info_label), TRUE);
    gtk_box_pack_start(GTK_BOX(box), info_label, FALSE, FALSE, 10);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
