#include <gtk/gtk.h>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>
#include <nlohmann/json.hpp>
#include "app_state.h"
#include "project_list.h"
#include "project_view.h"
#include "settings.h"
#include "../Client/network_wrapper.h"

using nlohmann::json;

struct ProjectMeta {
    std::string id;
    std::string name;
    std::string description;
    std::string status;
    std::string start_date;
    std::string end_date;
    std::string owner_id;
    std::string owner_name;
    int member_count;
    int task_count;
};

static GtkWidget *project_grid = nullptr;

static void free_project_meta(gpointer data) {
    delete static_cast<ProjectMeta*>(data);
}

static ProjectMeta* get_meta(GtkWidget *widget) {
    return static_cast<ProjectMeta*>(g_object_get_data(G_OBJECT(widget), "project_meta"));
}

static std::string status_to_server(const std::string &ui_status) {
    if (ui_status == "Planning") return "planning";
    if (ui_status == "In Progress") return "in_progress";
    if (ui_status == "Completed") return "completed";
    if (ui_status == "On Hold") return "on_hold";
    return ui_status;
}

static std::string status_to_ui(const std::string &server_status) {
    if (server_status == "planning") return "Planning";
    if (server_status == "in_progress") return "In Progress";
    if (server_status == "completed") return "Completed";
    if (server_status == "on_hold") return "On Hold";
    return server_status.empty() ? "Planning" : server_status;
}

static void show_error_dialog(GtkWindow *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void clear_project_grid() {
    if (!project_grid) {
        return;
    }
    GList *children = gtk_container_get_children(GTK_CONTAINER(project_grid));
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

static void load_projects();

static void on_project_clicked(GtkWidget *widget, gpointer data) {
    ProjectMeta *meta = get_meta(widget);
    if (!meta) {
        return;
    }

    g_strlcpy(current_project_id, meta->id.c_str(), sizeof(current_project_id));
    g_strlcpy(current_project_name, meta->name.c_str(), sizeof(current_project_name));
    is_project_owner = (meta->owner_id == current_user_id) ? 1 : 0;

    show_project_view_screen();
    if (project_view_window) {
        gtk_window_present(GTK_WINDOW(project_view_window));
    }
}

static void on_edit_project_clicked(GtkWidget *widget, gpointer data) {
    ProjectMeta *meta = get_meta(widget);
    if (!meta) {
        return;
    }
    if (meta->owner_id != current_user_id) {
        show_error_dialog(GTK_WINDOW(project_list_window), "Only project owner can edit.");
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Edit Project",
        GTK_WINDOW(project_list_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 550, 650);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    GtkWidget *name_label = gtk_label_new("Project Name *");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_box_pack_start(GTK_BOX(box), name_label, FALSE, FALSE, 0);

    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(name_entry), meta->name.c_str());
    gtk_box_pack_start(GTK_BOX(box), name_entry, FALSE, FALSE, 5);

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
    if (!meta->description.empty()) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text));
        gtk_text_buffer_set_text(buffer, meta->description.c_str(), -1);
    }

    GtkWidget *start_date_label = gtk_label_new("Start Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(start_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), start_date_label, FALSE, FALSE, 3);

    GtkWidget *start_date_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(start_date_entry), meta->start_date.c_str());
    gtk_box_pack_start(GTK_BOX(box), start_date_entry, FALSE, FALSE, 3);

    GtkWidget *end_date_label = gtk_label_new("End Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(end_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), end_date_label, FALSE, FALSE, 3);

    GtkWidget *end_date_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(end_date_entry), meta->end_date.c_str());
    gtk_box_pack_start(GTK_BOX(box), end_date_entry, FALSE, FALSE, 3);

    GtkWidget *status_label = gtk_label_new("Status");
    gtk_label_set_xalign(GTK_LABEL(status_label), 0);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 3);

    GtkWidget *status_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Planning");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "In Progress");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Completed");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "On Hold");
    std::string status_ui = status_to_ui(meta->status);
    if (status_ui == "In Progress") gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 1);
    else if (status_ui == "Completed") gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 2);
    else if (status_ui == "On Hold") gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 3);
    else gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);
    gtk_box_pack_start(GTK_BOX(box), status_combo, FALSE, FALSE, 3);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        gchar *desc_text_value = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        const char *start_date = gtk_entry_get_text(GTK_ENTRY(start_date_entry));
        const char *end_date = gtk_entry_get_text(GTK_ENTRY(end_date_entry));
        gchar *status_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));

        if (strlen(name) == 0) {
            show_error_dialog(GTK_WINDOW(dialog), "Project name is required!");
        } else {
            char response[4096];
            std::string server_status = status_text ? status_to_server(status_text) : "";
            if (!network_update_project(meta->id.c_str(), name, desc_text_value ? desc_text_value : "",
                                        server_status.c_str(), start_date, end_date, response, sizeof(response))) {
                show_error_dialog(GTK_WINDOW(dialog), "Failed to update project (network error)");
            } else if (json_get_status(response) != 0) {
                show_error_dialog(GTK_WINDOW(dialog), "Failed to update project");
            } else {
                load_projects();
            }
        }

        g_free(desc_text_value);
        g_free(status_text);
    }

    gtk_widget_destroy(dialog);
}

static void on_delete_project_clicked(GtkWidget *widget, gpointer data) {
    ProjectMeta *meta = get_meta(widget);
    if (!meta) {
        return;
    }
    if (meta->owner_id != current_user_id) {
        show_error_dialog(GTK_WINDOW(project_list_window), "Only project owner can delete.");
        return;
    }

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(project_list_window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Delete project '%s'?", meta->name.c_str());

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
        char response[4096];
        if (!network_delete_project(meta->id.c_str(), response, sizeof(response))) {
            show_error_dialog(GTK_WINDOW(project_list_window), "Failed to delete project (network error)");
        } else if (json_get_status(response) != 0) {
            show_error_dialog(GTK_WINDOW(project_list_window), "Failed to delete project");
        } else {
            load_projects();
        }
    }
    gtk_widget_destroy(dialog);
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

static void load_projects() {
    clear_project_grid();

    if (!network_is_connected()) {
        show_error_dialog(GTK_WINDOW(project_list_window), "Not connected to server.");
        return;
    }

    char response[8192];
    if (!network_get_projects(response, sizeof(response))) {
        show_error_dialog(GTK_WINDOW(project_list_window), "Failed to load projects (network error).");
        return;
    }

    json payload = json::parse(response, nullptr, false);
    if (payload.is_discarded()) {
        show_error_dialog(GTK_WINDOW(project_list_window), "Failed to parse project data.");
        return;
    }

    if (payload.value("status", 1) != 0) {
        show_error_dialog(GTK_WINDOW(project_list_window), "Failed to load projects.");
        return;
    }

    if (!payload.contains("projects") || !payload["projects"].is_array()) {
        show_error_dialog(GTK_WINDOW(project_list_window), "Project data missing.");
        return;
    }

    const json &projects = payload["projects"];
    int index = 0;

    for (const auto &item : projects) {
        ProjectMeta *meta = new ProjectMeta();
        meta->id = item.value("project_id", "");
        meta->name = item.value("project_name", "");
        meta->description = item.value("description", "");
        meta->status = item.value("status", "");
        meta->start_date = item.value("start_date", "");
        meta->end_date = item.value("end_date", "");
        meta->owner_id = item.value("owner_id", "");
        meta->owner_name = item.value("owner_name", "");
        meta->member_count = item.value("member_count", -1);
        meta->task_count = item.value("task_count", -1);

        GtkWidget *project_frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(project_frame), GTK_SHADOW_ETCHED_IN);
        gtk_widget_set_size_request(project_frame, 300, 200);

        g_object_set_data_full(G_OBJECT(project_frame), "project_meta", meta, free_project_meta);

        GtkWidget *project_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_container_set_border_width(GTK_CONTAINER(project_box), 16);
        gtk_widget_set_halign(project_box, GTK_ALIGN_FILL);
        gtk_widget_set_valign(project_box, GTK_ALIGN_FILL);
        gtk_container_add(GTK_CONTAINER(project_frame), project_box);

        GtkWidget *project_label = gtk_label_new(meta->name.c_str());
        PangoFontDescription *proj_font = pango_font_description_from_string("Sans Bold 16");
        gtk_widget_override_font(project_label, proj_font);
        gtk_label_set_line_wrap(GTK_LABEL(project_label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(project_label), 20);
        gtk_widget_set_margin_bottom(project_label, 6);
        gtk_box_pack_start(GTK_BOX(project_box), project_label, FALSE, FALSE, 0);

        std::string info = "Owner: " + (meta->owner_name.empty() ? "Unknown" : meta->owner_name);
        info += " • Status: " + status_to_ui(meta->status);
        if (meta->task_count >= 0 && meta->member_count >= 0) {
            info += " • " + std::to_string(meta->task_count) + " tasks";
            info += " • " + std::to_string(meta->member_count) + " members";
        } else if (meta->task_count >= 0) {
            info += " • " + std::to_string(meta->task_count) + " tasks";
        } else if (meta->member_count >= 0) {
            info += " • " + std::to_string(meta->member_count) + " members";
        }
        GtkWidget *info_label = gtk_label_new(info.c_str());
        PangoFontDescription *info_font = pango_font_description_from_string("Sans 11");
        gtk_widget_override_font(info_label, info_font);
        gtk_widget_set_margin_bottom(info_label, 6);
        gtk_box_pack_start(GTK_BOX(project_box), info_label, FALSE, FALSE, 0);

        std::string dates;
        if (!meta->start_date.empty() && !meta->end_date.empty()) {
            dates = meta->start_date + " to " + meta->end_date;
        } else if (!meta->end_date.empty()) {
            dates = "Due: " + meta->end_date;
        }
        GtkWidget *date_label = gtk_label_new(dates.c_str());
        gtk_label_set_xalign(GTK_LABEL(date_label), 0);
        gtk_box_pack_start(GTK_BOX(project_box), date_label, FALSE, FALSE, 0);

        GtkWidget *button_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        GtkWidget *open_btn = gtk_button_new_with_label("Open");
        GtkWidget *edit_btn = gtk_button_new_with_label("Edit");
        GtkWidget *delete_btn = gtk_button_new_with_label("Delete");

        g_object_set_data(G_OBJECT(open_btn), "project_meta", meta);
        g_object_set_data(G_OBJECT(edit_btn), "project_meta", meta);
        g_object_set_data(G_OBJECT(delete_btn), "project_meta", meta);

        g_signal_connect(open_btn, "clicked", G_CALLBACK(on_project_clicked), NULL);
        g_signal_connect(edit_btn, "clicked", G_CALLBACK(on_edit_project_clicked), NULL);
        g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_project_clicked), NULL);

        gboolean is_owner = (meta->owner_id == current_user_id);
        gtk_widget_set_sensitive(edit_btn, is_owner);
        gtk_widget_set_sensitive(delete_btn, is_owner);

        gtk_box_pack_start(GTK_BOX(button_row), open_btn, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(button_row), edit_btn, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(button_row), delete_btn, TRUE, TRUE, 0);
        gtk_box_pack_end(GTK_BOX(project_box), button_row, FALSE, FALSE, 0);

        gtk_grid_attach(GTK_GRID(project_grid), project_frame, index % 4, index / 4, 1, 1);

        pango_font_description_free(proj_font);
        pango_font_description_free(info_font);
        index++;
    }

    if (index == 0) {
        GtkWidget *no_projects = gtk_label_new("No projects found");
        PangoFontDescription *no_font = pango_font_description_from_string("Sans 18");
        gtk_widget_override_font(no_projects, no_font);
        gtk_widget_set_margin_top(no_projects, 100);
        gtk_grid_attach(GTK_GRID(project_grid), no_projects, 0, 0, 4, 1);
        pango_font_description_free(no_font);
    }

    gtk_widget_show_all(project_grid);
}

void show_project_list_screen() {
    if (login_window) gtk_widget_hide(login_window);
    if (register_window) gtk_widget_hide(register_window);
    if (project_view_window) gtk_widget_hide(project_view_window);
    if (settings_window) gtk_widget_hide(settings_window);

    project_list_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(project_list_window), "Projects - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(project_list_window), 1200, 720);
    gtk_window_set_position(GTK_WINDOW(project_list_window), GTK_WIN_POS_CENTER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(project_list_window), main_box);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header), 20);
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "header");

    GtkWidget *title = gtk_label_new("My Projects");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 24");
    gtk_widget_override_font(title, font_desc);
    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    project_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(project_grid), 20);
    gtk_grid_set_row_spacing(GTK_GRID(project_grid), 20);
    gtk_container_set_border_width(GTK_CONTAINER(project_grid), 20);
    gtk_grid_set_column_homogeneous(GTK_GRID(project_grid), TRUE);

    gtk_container_add(GTK_CONTAINER(scrolled), project_grid);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled, TRUE, TRUE, 0);

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
    gtk_window_present(GTK_WINDOW(project_list_window));

    load_projects();
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

    GtkWidget *name_label = gtk_label_new("Project Name *");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_box_pack_start(GTK_BOX(box), name_label, FALSE, FALSE, 0);

    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Enter project name");
    gtk_box_pack_start(GTK_BOX(box), name_entry, FALSE, FALSE, 5);

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

    GtkWidget *start_date_label = gtk_label_new("Start Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(start_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), start_date_label, FALSE, FALSE, 3);

    GtkWidget *start_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(start_date_entry), "2024-01-15");
    gtk_box_pack_start(GTK_BOX(box), start_date_entry, FALSE, FALSE, 3);

    GtkWidget *end_date_label = gtk_label_new("End Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(end_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), end_date_label, FALSE, FALSE, 3);

    GtkWidget *end_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(end_date_entry), "2024-12-31");
    gtk_box_pack_start(GTK_BOX(box), end_date_entry, FALSE, FALSE, 3);

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

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(name_entry));

        if (strlen(name) == 0) {
            show_error_dialog(GTK_WINDOW(dialog), "Project name is required!");
            gtk_widget_destroy(dialog);
            return;
        }

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        gchar *desc_value = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        const char *start_date = gtk_entry_get_text(GTK_ENTRY(start_date_entry));
        const char *end_date = gtk_entry_get_text(GTK_ENTRY(end_date_entry));
        gchar *status_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));
        char response[4096];
        if (!network_create_project(name, desc_value ? desc_value : "", response, sizeof(response))) {
            show_error_dialog(GTK_WINDOW(dialog), "Failed to create project (network error).");
        } else if (json_get_status(response) != 0) {
            show_error_dialog(GTK_WINDOW(dialog), "Failed to create project.");
        } else {
            char project_id[256] = "";
            json_get_string(response, "project_id", project_id, sizeof(project_id));

            std::string server_status = status_text ? status_to_server(status_text) : "";
            if ((server_status != "planning") || (start_date && strlen(start_date) > 0) || (end_date && strlen(end_date) > 0)) {
                char update_resp[4096];
                network_update_project(project_id, name, desc_value ? desc_value : "", server_status.c_str(), start_date, end_date, update_resp, sizeof(update_resp));
            }

            GtkWidget *success = gtk_message_dialog_new(GTK_WINDOW(project_list_window),
                GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                "Project created successfully!");
            gtk_dialog_run(GTK_DIALOG(success));
            gtk_widget_destroy(success);

            load_projects();
        }

        g_free(desc_value);
        g_free(status_text);
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
            char response[4096];
            if (!network_add_contact(friend_info, response, sizeof(response))) {
                show_error_dialog(GTK_WINDOW(project_list_window), "Failed to add contact (network error).");
            } else if (json_get_status(response) != 0) {
                show_error_dialog(GTK_WINDOW(project_list_window), "Failed to add contact.");
            } else {
                GtkWidget *success = gtk_message_dialog_new(GTK_WINDOW(project_list_window),
                    GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                    "Contact request sent!");
                gtk_dialog_run(GTK_DIALOG(success));
                gtk_widget_destroy(success);
            }
        }
    }

    gtk_widget_destroy(dialog);
}
