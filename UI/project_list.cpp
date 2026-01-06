#include <gtk/gtk.h>
#include <string.h>
#include "app_state.h"
#include "project_list.h"
#include "project_view.h"
#include "settings.h"

static void on_project_clicked(GtkWidget *widget, gpointer data) {
    show_project_view_screen();
}

static void on_create_project_clicked(GtkWidget *widget, gpointer data) {
    show_create_project_dialog();
}

static void on_add_friend_clicked(GtkWidget *widget, gpointer data) {
    show_add_friend_dialog();
}

static void on_settings_clicked(GtkWidget *widget, gpointer data) {
    show_settings_screen();
}

void show_project_list_screen() {
    if (login_window) gtk_widget_hide(login_window);
    if (register_window) gtk_widget_hide(register_window);
    if (project_view_window) gtk_widget_hide(project_view_window);
    if (settings_window) gtk_widget_hide(settings_window);

    project_list_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(project_list_window), "Projects - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(project_list_window), 1500, 850);
    gtk_window_set_position(GTK_WINDOW(project_list_window), GTK_WIN_POS_CENTER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(project_list_window), main_box);

    // Header
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header), 20);
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "header");

    GtkWidget *title = gtk_label_new("My Projects");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 24");
    gtk_widget_override_font(title, font_desc);
    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    // Project list area (scrolled window)
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *project_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(project_grid), 20);
    gtk_grid_set_row_spacing(GTK_GRID(project_grid), 20);
    gtk_container_set_border_width(GTK_CONTAINER(project_grid), 20);
    gtk_grid_set_column_homogeneous(GTK_GRID(project_grid), TRUE);

    // Sample projects (in real app, load from server)
    const char *projects[] = {"Project Alpha", "Project Beta", "Project Gamma", "Project Delta",
                              "Project Epsilon", "Project Zeta", NULL};
    int i = 0;
    while (projects[i]) {
        GtkWidget *project_frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(project_frame), GTK_SHADOW_ETCHED_IN);
        gtk_widget_set_size_request(project_frame, 300, 180);

        GtkWidget *project_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_set_border_width(GTK_CONTAINER(project_box), 20);
        gtk_widget_set_halign(project_box, GTK_ALIGN_FILL);
        gtk_widget_set_valign(project_box, GTK_ALIGN_FILL);
        gtk_container_add(GTK_CONTAINER(project_frame), project_box);

        // Project name
        GtkWidget *project_label = gtk_label_new(projects[i]);
        PangoFontDescription *proj_font = pango_font_description_from_string("Sans Bold 16");
        gtk_widget_override_font(project_label, proj_font);
        gtk_label_set_line_wrap(GTK_LABEL(project_label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(project_label), 20);
        gtk_widget_set_margin_bottom(project_label, 10);
        gtk_box_pack_start(GTK_BOX(project_box), project_label, FALSE, FALSE, 0);

        // Project info (sample)
        GtkWidget *info_label = gtk_label_new("5 tasks • 3 members");
        PangoFontDescription *info_font = pango_font_description_from_string("Sans 11");
        gtk_widget_override_font(info_label, info_font);
        gtk_widget_set_margin_bottom(info_label, 10);
        gtk_box_pack_start(GTK_BOX(project_box), info_label, FALSE, FALSE, 0);

        // Spacer
        GtkWidget *spacer = gtk_label_new("");
        gtk_box_pack_start(GTK_BOX(project_box), spacer, TRUE, TRUE, 0);

        // Open button
        GtkWidget *project_btn = gtk_button_new_with_label("Open Project");
        gtk_widget_set_size_request(project_btn, -1, 35);
        g_signal_connect(project_btn, "clicked", G_CALLBACK(on_project_clicked), NULL);
        gtk_box_pack_start(GTK_BOX(project_box), project_btn, FALSE, FALSE, 0);

        // 4 projects per row (i % 4 for column, i / 4 for row)
        gtk_grid_attach(GTK_GRID(project_grid), project_frame, i % 4, i / 4, 1, 1);

        pango_font_description_free(proj_font);
        pango_font_description_free(info_font);
        i++;
    }

    // If no projects
    if (i == 0) {
        GtkWidget *no_projects = gtk_label_new("No projects found");
        PangoFontDescription *no_font = pango_font_description_from_string("Sans 18");
        gtk_widget_override_font(no_projects, no_font);
        gtk_widget_set_margin_top(no_projects, 100);
        gtk_grid_attach(GTK_GRID(project_grid), no_projects, 0, 0, 4, 1);
        pango_font_description_free(no_font);
    }

    gtk_container_add(GTK_CONTAINER(scrolled), project_grid);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled, TRUE, TRUE, 0);

    // Bottom button bar
    GtkWidget *button_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(button_bar), 20);
    gtk_box_set_homogeneous(GTK_BOX(button_bar), FALSE);

    GtkWidget *create_btn = gtk_button_new_with_label("+ Create Project");
    GtkWidget *add_friend_btn = gtk_button_new_with_label("👤 Add Friend");
    GtkWidget *settings_btn = gtk_button_new_with_label("⚙ Settings");

    g_signal_connect(create_btn, "clicked", G_CALLBACK(on_create_project_clicked), NULL);
    g_signal_connect(add_friend_btn, "clicked", G_CALLBACK(on_add_friend_clicked), NULL);
    g_signal_connect(settings_btn, "clicked", G_CALLBACK(on_settings_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(button_bar), create_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_bar), add_friend_btn, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(button_bar), settings_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), button_bar, FALSE, FALSE, 0);

    g_signal_connect(project_list_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(project_list_window);
}

void show_create_project_dialog() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Create New Project",
        GTK_WINDOW(project_list_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Create", GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 550, 650);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // Project name
    GtkWidget *name_label = gtk_label_new("Project Name *");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_box_pack_start(GTK_BOX(box), name_label, FALSE, FALSE, 0);

    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Enter project name");
    gtk_box_pack_start(GTK_BOX(box), name_entry, FALSE, FALSE, 5);

    // Description
    GtkWidget *desc_label = gtk_label_new("Description");
    gtk_label_set_xalign(GTK_LABEL(desc_label), 0);
    gtk_box_pack_start(GTK_BOX(box), desc_label, FALSE, FALSE, 3);

    GtkWidget *desc_frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(desc_frame), GTK_SHADOW_IN);

    GtkWidget *desc_text = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(desc_text), GTK_WRAP_WORD);
    gtk_widget_set_size_request(desc_text, -1, 70);
    gtk_container_add(GTK_CONTAINER(desc_frame), desc_text);
    gtk_box_pack_start(GTK_BOX(box), desc_frame, FALSE, FALSE, 3);

    // Start Date
    GtkWidget *start_date_label = gtk_label_new("Start Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(start_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), start_date_label, FALSE, FALSE, 3);

    GtkWidget *start_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(start_date_entry), "2024-01-15");
    gtk_box_pack_start(GTK_BOX(box), start_date_entry, FALSE, FALSE, 3);

    // End Date
    GtkWidget *end_date_label = gtk_label_new("End Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(end_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), end_date_label, FALSE, FALSE, 3);

    GtkWidget *end_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(end_date_entry), "2024-12-31");
    gtk_box_pack_start(GTK_BOX(box), end_date_entry, FALSE, FALSE, 3);

    // Status
    GtkWidget *status_label = gtk_label_new("Status");
    gtk_label_set_xalign(GTK_LABEL(status_label), 0);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 3);

    GtkWidget *status_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Planning");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "In Progress");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Completed");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "On Hold");
    gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);
    gtk_box_pack_start(GTK_BOX(box), status_combo, FALSE, FALSE, 3);

    // Invite members
    GtkWidget *members_label = gtk_label_new("Invite Members (username or email, comma-separated)");
    gtk_label_set_xalign(GTK_LABEL(members_label), 0);
    gtk_box_pack_start(GTK_BOX(box), members_label, FALSE, FALSE, 3);

    GtkWidget *members_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(members_entry), "user1@email.com, user2, user3@email.com");
    gtk_box_pack_start(GTK_BOX(box), members_entry, FALSE, FALSE, 3);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(name_entry));

        if (strlen(name) == 0) {
            GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Project name is required!");
            gtk_dialog_run(GTK_DIALOG(error));
            gtk_widget_destroy(error);
            gtk_widget_destroy(dialog);
            return;
        }

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        const char *desc = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        const char *start_date = gtk_entry_get_text(GTK_ENTRY(start_date_entry));
        const char *end_date = gtk_entry_get_text(GTK_ENTRY(end_date_entry));
        const char *status = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));
        const char *members = gtk_entry_get_text(GTK_ENTRY(members_entry));

        // In real app, send to server
        // POST /api/projects with: name, desc, start_date, end_date, status, members[]
        GtkWidget *success = gtk_message_dialog_new(GTK_WINDOW(project_list_window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "Project created successfully!");
        gtk_dialog_run(GTK_DIALOG(success));
        gtk_widget_destroy(success);

        (void)desc;
        (void)start_date;
        (void)end_date;
        (void)status;
        (void)members;
    }

    gtk_widget_destroy(dialog);
}

void show_add_friend_dialog() {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Friend",
        GTK_WINDOW(project_list_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Add", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    GtkWidget *label = gtk_label_new("Enter username or email:");
    gtk_label_set_xalign(GTK_LABEL(label), 0);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 5);

    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "username or email@example.com");
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *friend_info = gtk_entry_get_text(GTK_ENTRY(entry));
        if (strlen(friend_info) > 0) {
            // In real app, send to server
            GtkWidget *success = gtk_message_dialog_new(GTK_WINDOW(project_list_window),
                GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                "Friend request sent!");
            gtk_dialog_run(GTK_DIALOG(success));
            gtk_widget_destroy(success);
        }
    }

    gtk_widget_destroy(dialog);
}
