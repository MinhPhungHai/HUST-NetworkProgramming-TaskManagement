#include <gtk/gtk.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include "app_state.h"
#include "project_view.h"
#include "project_list.h"
#include "gantt.h"
#include "../Client/network_wrapper.h"

using nlohmann::json;

namespace {
struct MemberInfo {
    std::string id;
    std::string name;
};

enum TaskCols {
    COL_TASK_NAME = 0,
    COL_ASSIGNEE,
    COL_FILE,
    COL_STATUS,
    COL_DEADLINE,
    COL_TASK_ID,
    COL_ASSIGNEE_ID,
    COL_START_DATE,
    COL_DUE_DATE,
    COL_DESCRIPTION,
    COL_PRIORITY,
    COL_COUNT
};

std::vector<MemberInfo> g_members;
std::unordered_map<std::string, std::string> g_member_ids;
GtkWidget *g_task_tree = nullptr;
GtkWidget *g_chat_view = nullptr;
GtkWidget *g_project_title = nullptr;

void show_comments_dialog();

std::string status_to_server(const std::string &ui_status) {
    if (ui_status == "Not Started") return "todo";
    if (ui_status == "In Progress") return "in_progress";
    if (ui_status == "Completed") return "completed";
    return ui_status;
}

std::string status_to_ui(const std::string &server_status) {
    if (server_status == "todo") return "Not Started";
    if (server_status == "in_progress") return "In Progress";
    if (server_status == "completed") return "Completed";
    return server_status.empty() ? "Not Started" : server_status;
}

void show_error_dialog(GtkWindow *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                               "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

std::string get_basename(const std::string &path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

std::string extension_to_mime(const std::string &ext) {
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "pdf") return "application/pdf";
    if (ext == "txt") return "text/plain";
    if (ext == "doc" || ext == "docx") return "application/msword";
    if (ext == "xls" || ext == "xlsx") return "application/vnd.ms-excel";
    return "application/octet-stream";
}

bool get_file_meta(const std::string &path, std::string &file_name, std::string &file_type, int &file_size) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    file_size = static_cast<int>(st.st_size);
    file_name = get_basename(path);
    std::string ext;
    size_t dot = file_name.find_last_of('.');
    if (dot != std::string::npos) {
        ext = file_name.substr(dot + 1);
    }
    file_type = extension_to_mime(ext);
    return true;
}

std::string build_deadline(const std::string &start_date, const std::string &due_date) {
    if (!start_date.empty() && !due_date.empty()) {
        return start_date + " to " + due_date;
    }
    if (!due_date.empty()) {
        return "Due: " + due_date;
    }
    if (!start_date.empty()) {
        return "Start: " + start_date;
    }
    return "";
}

std::string trim_copy(const std::string &value) {
    size_t start = value.find_first_not_of(" \t\n\r");
    size_t end = value.find_last_not_of(" \t\n\r");
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }
    return value.substr(start, end - start + 1);
}

std::string find_member_id(const std::string &input) {
    std::string key = trim_copy(input);
    if (key.empty()) {
        return "";
    }
    auto it = g_member_ids.find(key);
    if (it != g_member_ids.end()) {
        return it->second;
    }
    return "";
}

std::string load_files_label(const std::string &task_id) {
    char response[4096];
    if (!network_get_files(task_id.c_str(), response, sizeof(response))) {
        return "";
    }
    json payload = json::parse(response, nullptr, false);
    if (payload.is_discarded() || payload.value("status", 1) != 0) {
        return "";
    }
    if (!payload.contains("files") || !payload["files"].is_array()) {
        return "";
    }
    const json &files = payload["files"];
    if (files.empty()) {
        return "";
    }
    std::string first = files[0].value("file_name", "");
    if (files.size() == 1) {
        return first;
    }
    return first + " (+" + std::to_string(files.size() - 1) + ")";
}

struct CommentDialogState {
    std::string task_id;
    GtkWidget *text_view;
    GtkWidget *entry;
};

void refresh_task_comments(CommentDialogState *state) {
    if (!state || !state->text_view) {
        return;
    }
    char response[8192];
    if (!network_get_task_comments(state->task_id.c_str(), response, sizeof(response))) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to load comments (network error).");
        return;
    }

    json payload = json::parse(response, nullptr, false);
    if (payload.is_discarded() || payload.value("status", 1) != 0) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to load comments.");
        return;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->text_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    if (!payload.contains("comments") || !payload["comments"].is_array()) {
        return;
    }

    for (const auto &comment : payload["comments"]) {
        std::string username = comment.value("username", "unknown");
        std::string content = comment.value("content", "");
        std::string created_at = comment.value("created_at", "");
        std::string line = username + " (" + created_at + "): " + content + "\n";
        gtk_text_buffer_insert_at_cursor(buffer, line.c_str(), -1);
    }
}

void on_add_comment_clicked(GtkWidget *widget, gpointer data) {
    CommentDialogState *state = static_cast<CommentDialogState*>(data);
    if (!state || !state->entry) {
        return;
    }

    const char *content = gtk_entry_get_text(GTK_ENTRY(state->entry));
    if (!content || strlen(content) == 0) {
        return;
    }

    char response[4096];
    if (!network_add_task_comment(state->task_id.c_str(), content, response, sizeof(response))) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to add comment (network error).");
        return;
    }

    if (json_get_status(response) != 0) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to add comment.");
        return;
    }

    gtk_entry_set_text(GTK_ENTRY(state->entry), "");
    refresh_task_comments(state);
}

void show_comments_dialog() {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_task_tree));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Please select a task to view comments.");
        return;
    }

    gchar *task_id = nullptr;
    gchar *task_name = nullptr;
    gtk_tree_model_get(model, &iter,
        COL_TASK_ID, &task_id,
        COL_TASK_NAME, &task_name,
        -1);

    if (!task_id || strlen(task_id) == 0) {
        g_free(task_id);
        g_free(task_name);
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Task Comments",
        GTK_WINDOW(project_view_window),
        GTK_DIALOG_MODAL,
        "Close", GTK_RESPONSE_CLOSE,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 500);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);
    gtk_container_add(GTK_CONTAINER(content), box);

    std::string title = "Task: ";
    title += task_name ? task_name : "";
    GtkWidget *title_label = gtk_label_new(title.c_str());
    gtk_label_set_xalign(GTK_LABEL(title_label), 0);
    gtk_box_pack_start(GTK_BOX(box), title_label, FALSE, FALSE, 2);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled), text_view);
    gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Add a comment...");
    GtkWidget *add_btn = gtk_button_new_with_label("Send");
    gtk_box_pack_start(GTK_BOX(input_box), entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(input_box), add_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), input_box, FALSE, FALSE, 0);

    CommentDialogState *state = new CommentDialogState();
    state->task_id = task_id;
    state->text_view = text_view;
    state->entry = entry;

    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_add_comment_clicked), state);
    g_signal_connect(entry, "activate", G_CALLBACK(on_add_comment_clicked), state);
    g_object_set_data_full(G_OBJECT(dialog), "comment_state", state,
                           [](gpointer data) { delete static_cast<CommentDialogState*>(data); });

    gtk_widget_show_all(dialog);
    refresh_task_comments(state);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    g_free(task_id);
    g_free(task_name);
}

void load_project_details() {
    if (!network_is_connected()) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Not connected to server.");
        return;
    }

    char response[8192];
    if (!network_get_project_details(current_project_id, response, sizeof(response))) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to load project details.");
        return;
    }

    json payload = json::parse(response, nullptr, false);
    if (payload.is_discarded() || payload.value("status", 1) != 0) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to load project details.");
        return;
    }

    if (!payload.contains("project") || !payload["project"].is_object()) {
        return;
    }

    const json &project = payload["project"];
    std::string name = project.value("project_name", current_project_name);
    g_strlcpy(current_project_name, name.c_str(), sizeof(current_project_name));
    if (g_project_title) {
        gtk_label_set_text(GTK_LABEL(g_project_title), current_project_name);
    }

    g_members.clear();
    g_member_ids.clear();
    if (project.contains("members") && project["members"].is_array()) {
        for (const auto &member : project["members"]) {
            MemberInfo info;
            info.id = member.value("user_id", "");
            info.name = member.value("username", "");
            if (!info.id.empty()) {
                g_members.push_back(info);
                if (!info.name.empty()) {
                    g_member_ids[info.name] = info.id;
                }
            }
        }
    }
}

void load_tasks() {
    if (!g_task_tree) {
        return;
    }

    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(g_task_tree)));
    gtk_list_store_clear(store);

    if (!network_is_connected()) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Not connected to server.");
        return;
    }

    char response[8192];
    if (!network_get_tasks(current_project_id, response, sizeof(response))) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to load tasks.");
        return;
    }

    json payload = json::parse(response, nullptr, false);
    if (payload.is_discarded() || payload.value("status", 1) != 0) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to load tasks.");
        return;
    }

    if (!payload.contains("tasks") || !payload["tasks"].is_array()) {
        return;
    }

    for (const auto &task : payload["tasks"]) {
        std::string task_id = task.value("task_id", "");
        std::string task_name = task.value("task_name", "");
        std::string description = task.value("description", "");
        std::string assigned_id = task.value("assigned_to", "");
        std::string assigned_name = task.value("assigned_name", "");
        std::string status = status_to_ui(task.value("status", ""));
        std::string priority = task.value("priority", "");
        std::string start_date = task.value("start_date", "");
        std::string due_date = task.value("due_date", "");

        std::string deadline = build_deadline(start_date, due_date);
        std::string file_label = load_files_label(task_id);

        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            COL_TASK_NAME, task_name.c_str(),
            COL_ASSIGNEE, assigned_name.c_str(),
            COL_FILE, file_label.c_str(),
            COL_STATUS, status.c_str(),
            COL_DEADLINE, deadline.c_str(),
            COL_TASK_ID, task_id.c_str(),
            COL_ASSIGNEE_ID, assigned_id.c_str(),
            COL_START_DATE, start_date.c_str(),
            COL_DUE_DATE, due_date.c_str(),
            COL_DESCRIPTION, description.c_str(),
            COL_PRIORITY, priority.c_str(),
            -1);
    }
}

void load_chat_history() {
    if (!g_chat_view) {
        return;
    }

    if (!network_is_connected()) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Not connected to server.");
        return;
    }

    char response[8192];
    if (!network_get_chat_history(current_project_id, 50, response, sizeof(response))) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to load chat history.");
        return;
    }

    json payload = json::parse(response, nullptr, false);
    if (payload.is_discarded() || payload.value("status", 1) != 0) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to load chat history.");
        return;
    }

    if (!payload.contains("messages") || !payload["messages"].is_array()) {
        return;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_chat_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    for (const auto &msg : payload["messages"]) {
        std::string sender = msg.value("sender_name", "");
        std::string content = msg.value("message", "");
        std::string line = sender + ": " + content + "\n";
        gtk_text_buffer_insert_at_cursor(buffer, line.c_str(), -1);
    }
}

void on_back_to_projects(GtkWidget *widget, gpointer data) {
    gtk_widget_hide(project_view_window);
    show_project_list_screen();
}

void on_view_gantt_chart(GtkWidget *widget, gpointer data) {
    show_gantt_chart_window();
}

void on_status_edited(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(data);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gchar *task_id = nullptr;
        gchar *priority = nullptr;
        gchar *start_date = nullptr;
        gchar *due_date = nullptr;
        gtk_tree_model_get(model, &iter, COL_TASK_ID, &task_id, -1);
        if (task_id && strlen(task_id) > 0) {
            std::string status_server = status_to_server(new_text);
            char response[4096];
            gtk_tree_model_get(model, &iter,
                               COL_PRIORITY, &priority,
                               COL_START_DATE, &start_date,
                               COL_DUE_DATE, &due_date,
                               -1);
            if (network_update_task(task_id, "", "", "", status_server.c_str(),
                                    priority ? priority : "",
                                    start_date ? start_date : "",
                                    due_date ? due_date : "",
                                    response, sizeof(response))
                && json_get_status(response) == 0) {
                load_tasks();
            }
        }
        g_free(task_id);
        g_free(priority);
        g_free(start_date);
        g_free(due_date);
    }
    gtk_tree_path_free(path);
}

void on_add_task_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Create Task",
        GTK_WINDOW(project_view_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Create", GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 550, 700);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    GtkWidget *name_label = gtk_label_new("Task Name *");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_box_pack_start(GTK_BOX(box), name_label, FALSE, FALSE, 0);

    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Enter task name");
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

    GtkWidget *people_label = gtk_label_new("Assign To (username or email)");
    gtk_label_set_xalign(GTK_LABEL(people_label), 0);
    gtk_box_pack_start(GTK_BOX(box), people_label, FALSE, FALSE, 3);

    GtkWidget *people_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(people_entry), "username or email@example.com");
    gtk_box_pack_start(GTK_BOX(box), people_entry, FALSE, FALSE, 3);

    GtkWidget *status_label = gtk_label_new("Status");
    gtk_label_set_xalign(GTK_LABEL(status_label), 0);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 3);

    GtkWidget *status_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Not Started");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "In Progress");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Completed");
    gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);
    gtk_box_pack_start(GTK_BOX(box), status_combo, FALSE, FALSE, 3);

    GtkWidget *priority_label = gtk_label_new("Priority");
    gtk_label_set_xalign(GTK_LABEL(priority_label), 0);
    gtk_box_pack_start(GTK_BOX(box), priority_label, FALSE, FALSE, 3);

    GtkWidget *priority_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "low");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "medium");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "high");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "urgent");
    gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo), 1);
    gtk_box_pack_start(GTK_BOX(box), priority_combo, FALSE, FALSE, 3);

    GtkWidget *start_date_label = gtk_label_new("Start Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(start_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), start_date_label, FALSE, FALSE, 3);

    GtkWidget *start_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(start_date_entry), "2024-01-15");
    gtk_box_pack_start(GTK_BOX(box), start_date_entry, FALSE, FALSE, 3);

    GtkWidget *due_date_label = gtk_label_new("Due Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(due_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), due_date_label, FALSE, FALSE, 3);

    GtkWidget *due_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(due_date_entry), "2024-01-30");
    gtk_box_pack_start(GTK_BOX(box), due_date_entry, FALSE, FALSE, 3);

    GtkWidget *file_label = gtk_label_new("File Path (optional)");
    gtk_label_set_xalign(GTK_LABEL(file_label), 0);
    gtk_box_pack_start(GTK_BOX(box), file_label, FALSE, FALSE, 3);

    GtkWidget *file_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(file_entry), "/path/to/file or leave empty");
    gtk_box_pack_start(GTK_BOX(box), file_entry, FALSE, FALSE, 3);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *task_name = gtk_entry_get_text(GTK_ENTRY(name_entry));

        if (strlen(task_name) == 0) {
            show_error_dialog(GTK_WINDOW(dialog), "Task name is required!");
            gtk_widget_destroy(dialog);
            return;
        }

        GtkTextBuffer *desc_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(desc_buffer, &start, &end);
        gchar *description = gtk_text_buffer_get_text(desc_buffer, &start, &end, FALSE);

        const char *assignee_input = gtk_entry_get_text(GTK_ENTRY(people_entry));
        std::string assigned_id = find_member_id(assignee_input ? assignee_input : "");

        gchar *status_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));
        gchar *priority_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priority_combo));
        const char *start_date = gtk_entry_get_text(GTK_ENTRY(start_date_entry));
        const char *due_date = gtk_entry_get_text(GTK_ENTRY(due_date_entry));
        const char *file_path = gtk_entry_get_text(GTK_ENTRY(file_entry));

        char response[4096];
        if (!network_create_task(current_project_id, task_name, description ? description : "",
                                 assigned_id.c_str(),
                                 priority_text ? priority_text : "medium",
                                 start_date, due_date, response, sizeof(response))) {
            show_error_dialog(GTK_WINDOW(dialog), "Failed to create task (network error).");
        } else if (json_get_status(response) != 0) {
            show_error_dialog(GTK_WINDOW(dialog), "Failed to create task.");
        } else {
            char task_id[256] = "";
            json_get_string(response, "task_id", task_id, sizeof(task_id));

            if (status_text && strcmp(status_text, "Not Started") != 0) {
                std::string status_server = status_to_server(status_text);
                char update_resp[4096];
                network_update_task(task_id, "", "", "", status_server.c_str(), "", "", "", update_resp, sizeof(update_resp));
            }

            if (file_path && strlen(file_path) > 0) {
                std::string file_name;
                std::string file_type;
                int file_size = 0;
                if (get_file_meta(file_path, file_name, file_type, file_size)) {
                    char file_resp[4096];
                    network_upload_file(task_id, file_name.c_str(), file_path, file_type.c_str(), file_size, file_resp, sizeof(file_resp));
                } else {
                    show_error_dialog(GTK_WINDOW(dialog), "Failed to read file metadata for attachment.");
                }
            }

            load_tasks();
        }

        g_free(description);
        g_free(status_text);
        g_free(priority_text);
    }

    gtk_widget_destroy(dialog);
}

void on_edit_task_clicked(GtkWidget *widget, gpointer data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_task_tree));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Please select a task to edit.");
        return;
    }

    gchar *task_id = nullptr;
    gchar *task_name = nullptr;
    gchar *assignee_id = nullptr;
    gchar *status = nullptr;
    gchar *start_date = nullptr;
    gchar *due_date = nullptr;
    gchar *description = nullptr;
    gchar *priority = nullptr;

    gtk_tree_model_get(model, &iter,
        COL_TASK_ID, &task_id,
        COL_TASK_NAME, &task_name,
        COL_ASSIGNEE_ID, &assignee_id,
        COL_STATUS, &status,
        COL_START_DATE, &start_date,
        COL_DUE_DATE, &due_date,
        COL_DESCRIPTION, &description,
        COL_PRIORITY, &priority,
        -1);

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Edit Task",
        GTK_WINDOW(project_view_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 550, 700);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    GtkWidget *name_label = gtk_label_new("Task Name *");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_box_pack_start(GTK_BOX(box), name_label, FALSE, FALSE, 0);

    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(name_entry), task_name ? task_name : "");
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
    if (description) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text));
        gtk_text_buffer_set_text(buffer, description, -1);
    }

    GtkWidget *people_label = gtk_label_new("Assign To (project member)");
    gtk_label_set_xalign(GTK_LABEL(people_label), 0);
    gtk_box_pack_start(GTK_BOX(box), people_label, FALSE, FALSE, 3);

    GtkWidget *people_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(people_combo), "Unassigned");
    int active_index = 0;
    for (size_t i = 0; i < g_members.size(); ++i) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(people_combo), g_members[i].name.c_str());
        if (assignee_id && g_members[i].id == assignee_id) {
            active_index = static_cast<int>(i) + 1;
        }
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(people_combo), active_index);
    gtk_box_pack_start(GTK_BOX(box), people_combo, FALSE, FALSE, 3);

    GtkWidget *status_label = gtk_label_new("Status");
    gtk_label_set_xalign(GTK_LABEL(status_label), 0);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 3);

    GtkWidget *status_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Not Started");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "In Progress");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Completed");
    std::string status_ui = status ? status : "";
    if (status_ui == "In Progress") gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 1);
    else if (status_ui == "Completed") gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 2);
    else gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);
    gtk_box_pack_start(GTK_BOX(box), status_combo, FALSE, FALSE, 3);

    GtkWidget *priority_label = gtk_label_new("Priority");
    gtk_label_set_xalign(GTK_LABEL(priority_label), 0);
    gtk_box_pack_start(GTK_BOX(box), priority_label, FALSE, FALSE, 3);

    GtkWidget *priority_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "low");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "medium");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "high");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "urgent");
    if (priority && strcmp(priority, "high") == 0) gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo), 2);
    else if (priority && strcmp(priority, "urgent") == 0) gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo), 3);
    else if (priority && strcmp(priority, "low") == 0) gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo), 0);
    else gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo), 1);
    gtk_box_pack_start(GTK_BOX(box), priority_combo, FALSE, FALSE, 3);

    GtkWidget *start_date_label = gtk_label_new("Start Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(start_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), start_date_label, FALSE, FALSE, 3);

    GtkWidget *start_date_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(start_date_entry), start_date ? start_date : "");
    gtk_box_pack_start(GTK_BOX(box), start_date_entry, FALSE, FALSE, 3);

    GtkWidget *due_date_label = gtk_label_new("Due Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(due_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), due_date_label, FALSE, FALSE, 3);

    GtkWidget *due_date_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(due_date_entry), due_date ? due_date : "");
    gtk_box_pack_start(GTK_BOX(box), due_date_entry, FALSE, FALSE, 3);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *new_name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        if (strlen(new_name) == 0) {
            show_error_dialog(GTK_WINDOW(dialog), "Task name is required!");
        } else {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text));
            GtkTextIter b_start, b_end;
            gtk_text_buffer_get_bounds(buffer, &b_start, &b_end);
            gchar *new_desc = gtk_text_buffer_get_text(buffer, &b_start, &b_end, FALSE);

            gint assignee_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(people_combo));
            std::string assigned_id;
            if (assignee_idx > 0 && static_cast<size_t>(assignee_idx - 1) < g_members.size()) {
                assigned_id = g_members[assignee_idx - 1].id;
            }

            gchar *new_status = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));
            gchar *new_priority = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priority_combo));
            const char *new_start = gtk_entry_get_text(GTK_ENTRY(start_date_entry));
            const char *new_due = gtk_entry_get_text(GTK_ENTRY(due_date_entry));

            std::string status_server = new_status ? status_to_server(new_status) : "";
            char response[4096];
            if (!network_update_task(task_id ? task_id : "", new_name, new_desc ? new_desc : "",
                                     assigned_id.c_str(), status_server.c_str(),
                                     new_priority ? new_priority : "", new_start, new_due,
                                     response, sizeof(response))) {
                show_error_dialog(GTK_WINDOW(dialog), "Failed to update task (network error).");
            } else if (json_get_status(response) != 0) {
                show_error_dialog(GTK_WINDOW(dialog), "Failed to update task.");
            } else {
                load_tasks();
            }

            g_free(new_desc);
            g_free(new_status);
            g_free(new_priority);
        }
    }

    gtk_widget_destroy(dialog);
    g_free(task_id);
    g_free(task_name);
    g_free(assignee_id);
    g_free(status);
    g_free(start_date);
    g_free(due_date);
    g_free(description);
    g_free(priority);
}

void on_delete_task_clicked(GtkWidget *widget, gpointer data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(g_task_tree));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Please select a task to delete.");
        return;
    }

    gchar *task_id = nullptr;
    gtk_tree_model_get(model, &iter, COL_TASK_ID, &task_id, -1);
    if (!task_id) {
        return;
    }

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(project_view_window),
        GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Do you want to delete this task?");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
        char response[4096];
        if (!network_delete_task(task_id, response, sizeof(response))) {
            show_error_dialog(GTK_WINDOW(project_view_window), "Failed to delete task (network error).");
        } else if (json_get_status(response) != 0) {
            show_error_dialog(GTK_WINDOW(project_view_window), "Failed to delete task.");
        } else {
            load_tasks();
        }
    }
    gtk_widget_destroy(dialog);
    g_free(task_id);
}

void on_chat_send(GtkWidget *widget, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "chat_entry"));
    const char *message = gtk_entry_get_text(GTK_ENTRY(entry));
    if (!message || strlen(message) == 0) {
        return;
    }

    char response[4096];
    if (!network_send_chat(current_project_id, message, response, sizeof(response))) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to send chat (network error).");
        return;
    }

    if (json_get_status(response) != 0) {
        show_error_dialog(GTK_WINDOW(project_view_window), "Failed to send chat.");
        return;
    }

    gtk_entry_set_text(GTK_ENTRY(entry), "");
    load_chat_history();
}
} // namespace

void show_project_view_screen() {
    if (project_list_window) gtk_widget_hide(project_list_window);

    project_view_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(project_view_window), "Project View - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(project_view_window), 1200, 720);
    gtk_window_set_position(GTK_WINDOW(project_view_window), GTK_WIN_POS_CENTER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(project_view_window), main_box);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header), 15);

    GtkWidget *back_btn = gtk_button_new_with_label("← Back");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_to_projects), NULL);
    gtk_box_pack_start(GTK_BOX(header), back_btn, FALSE, FALSE, 0);

    g_project_title = gtk_label_new(current_project_name[0] ? current_project_name : "Project");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 20");
    gtk_widget_override_font(g_project_title, font_desc);
    gtk_box_pack_start(GTK_BOX(header), g_project_title, FALSE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *content_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), content_paned, TRUE, TRUE, 0);

    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *gantt_button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(gantt_button_box), 10);

    GtkWidget *gantt_btn = gtk_button_new_with_label("📊 Open Gantt Chart");
    gtk_widget_set_size_request(gantt_btn, 200, 40);
    g_signal_connect(gantt_btn, "clicked", G_CALLBACK(on_view_gantt_chart), NULL);
    gtk_box_pack_start(GTK_BOX(gantt_button_box), gantt_btn, FALSE, FALSE, 0);

    GtkWidget *gantt_info = gtk_label_new("View project timeline and task dependencies");
    gtk_label_set_xalign(GTK_LABEL(gantt_info), 0);
    gtk_box_pack_start(GTK_BOX(gantt_button_box), gantt_info, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(left_box), gantt_button_box, FALSE, FALSE, 0);

    GtkWidget *task_frame = gtk_frame_new("Tasks");
    gtk_container_set_border_width(GTK_CONTAINER(task_frame), 10);

    GtkWidget *task_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(task_frame), task_box);

    GtkWidget *task_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *add_task_btn = gtk_button_new_with_label("+ Add Task");
    GtkWidget *edit_task_btn = gtk_button_new_with_label("Edit Task");
    GtkWidget *delete_task_btn = gtk_button_new_with_label("Delete Task");
    GtkWidget *comments_btn = gtk_button_new_with_label("Comments");
    gtk_box_pack_start(GTK_BOX(task_buttons), add_task_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_buttons), edit_task_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_buttons), delete_task_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_buttons), comments_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_box), task_buttons, FALSE, FALSE, 5);

    GtkWidget *task_scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkListStore *store = gtk_list_store_new(COL_COUNT,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    g_task_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Task", renderer, "text", COL_TASK_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(g_task_tree), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("People", renderer, "text", COL_ASSIGNEE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(g_task_tree), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("File", renderer, "text", COL_FILE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(g_task_tree), column);

    GtkListStore *status_store = gtk_list_store_new(1, G_TYPE_STRING);
    GtkTreeIter status_iter;
    const char *statuses[] = {"Not Started", "In Progress", "Completed"};
    for (int i = 0; i < 3; i++) {
        gtk_list_store_append(status_store, &status_iter);
        gtk_list_store_set(status_store, &status_iter, 0, statuses[i], -1);
    }

    renderer = gtk_cell_renderer_combo_new();
    g_object_set(renderer, "editable", TRUE, "model", status_store, "text-column", 0, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_status_edited), g_task_tree);
    column = gtk_tree_view_column_new_with_attributes("Status", renderer, "text", COL_STATUS, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(g_task_tree), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Deadline", renderer, "text", COL_DEADLINE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(g_task_tree), column);

    gtk_container_add(GTK_CONTAINER(task_scrolled), g_task_tree);
    gtk_box_pack_start(GTK_BOX(task_box), task_scrolled, TRUE, TRUE, 0);

    g_signal_connect(add_task_btn, "clicked", G_CALLBACK(on_add_task_clicked), NULL);
    g_signal_connect(edit_task_btn, "clicked", G_CALLBACK(on_edit_task_clicked), NULL);
    g_signal_connect(delete_task_btn, "clicked", G_CALLBACK(on_delete_task_clicked), NULL);
    g_signal_connect(comments_btn, "clicked", G_CALLBACK(show_comments_dialog), NULL);

    gtk_box_pack_start(GTK_BOX(left_box), task_frame, TRUE, TRUE, 0);

    GtkWidget *chat_frame = gtk_frame_new("Chat");
    gtk_container_set_border_width(GTK_CONTAINER(chat_frame), 10);

    GtkWidget *chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(chat_frame), chat_box);

    GtkWidget *chat_scrolled = gtk_scrolled_window_new(NULL, NULL);
    g_chat_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(g_chat_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(g_chat_view), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(chat_scrolled), g_chat_view);
    gtk_box_pack_start(GTK_BOX(chat_box), chat_scrolled, TRUE, TRUE, 0);

    GtkWidget *chat_input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *chat_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(chat_entry), "Type a message...");
    GtkWidget *send_btn = gtk_button_new_with_label("Send");

    g_object_set_data(G_OBJECT(send_btn), "chat_entry", chat_entry);
    g_signal_connect(send_btn, "clicked", G_CALLBACK(on_chat_send), send_btn);
    g_object_set_data(G_OBJECT(chat_entry), "send_btn", send_btn);
    g_signal_connect(chat_entry, "activate", G_CALLBACK(on_chat_send), send_btn);

    gtk_box_pack_start(GTK_BOX(chat_input_box), chat_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(chat_input_box), send_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(chat_box), chat_input_box, FALSE, FALSE, 0);

    gtk_paned_add1(GTK_PANED(content_paned), left_box);
    gtk_paned_add2(GTK_PANED(content_paned), chat_frame);
    gtk_paned_set_position(GTK_PANED(content_paned), 1200);

    g_signal_connect(project_view_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(project_view_window);

    load_project_details();
    load_tasks();
    load_chat_history();
}
