#include <gtk/gtk.h>
#include <string.h>
#include "app_state.h"
#include "project_view.h"
#include "project_list.h"
#include "gantt.h"

static void on_back_to_projects(GtkWidget *widget, gpointer data) {
    gtk_widget_hide(project_view_window);
    show_project_list_screen();
}

static void on_view_gantt_chart(GtkWidget *widget, gpointer data) {
    show_gantt_chart_window();
}

static void on_add_task_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Create Task",
        GTK_WINDOW(project_view_window),
        GTK_DIALOG_MODAL,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Create", GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 550, 650);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    // Task Name
    GtkWidget *name_label = gtk_label_new("Task Name *");
    gtk_label_set_xalign(GTK_LABEL(name_label), 0);
    gtk_box_pack_start(GTK_BOX(box), name_label, FALSE, FALSE, 0);

    GtkWidget *name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(name_entry), "Enter task name");
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

    // Assigned To (People)
    GtkWidget *people_label = gtk_label_new("Assign To (username or email)");
    gtk_label_set_xalign(GTK_LABEL(people_label), 0);
    gtk_box_pack_start(GTK_BOX(box), people_label, FALSE, FALSE, 3);

    GtkWidget *people_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(people_entry), "username or email@example.com");
    gtk_box_pack_start(GTK_BOX(box), people_entry, FALSE, FALSE, 3);

    // Status
    GtkWidget *status_label = gtk_label_new("Status");
    gtk_label_set_xalign(GTK_LABEL(status_label), 0);
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 3);

    GtkWidget *status_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Not Started");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "In Progress");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(status_combo), "Completed");
    gtk_combo_box_set_active(GTK_COMBO_BOX(status_combo), 0);
    gtk_box_pack_start(GTK_BOX(box), status_combo, FALSE, FALSE, 3);

    // Start Date
    GtkWidget *start_date_label = gtk_label_new("Start Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(start_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), start_date_label, FALSE, FALSE, 3);

    GtkWidget *start_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(start_date_entry), "2024-01-15");
    gtk_box_pack_start(GTK_BOX(box), start_date_entry, FALSE, FALSE, 3);

    // Due Date
    GtkWidget *due_date_label = gtk_label_new("Due Date (YYYY-MM-DD)");
    gtk_label_set_xalign(GTK_LABEL(due_date_label), 0);
    gtk_box_pack_start(GTK_BOX(box), due_date_label, FALSE, FALSE, 3);

    GtkWidget *due_date_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(due_date_entry), "2024-01-30");
    gtk_box_pack_start(GTK_BOX(box), due_date_entry, FALSE, FALSE, 3);

    // File Attachment (optional)
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
            GtkWidget *error = gtk_message_dialog_new(GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Task name is required!");
            gtk_dialog_run(GTK_DIALOG(error));
            gtk_widget_destroy(error);
            gtk_widget_destroy(dialog);
            return;
        }

        // Get description
        GtkTextBuffer *desc_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(desc_buffer, &start, &end);
        const char *description = gtk_text_buffer_get_text(desc_buffer, &start, &end, FALSE);

        // Get other fields
        const char *people = gtk_entry_get_text(GTK_ENTRY(people_entry));
        const char *status = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(status_combo));
        const char *start_date = gtk_entry_get_text(GTK_ENTRY(start_date_entry));
        const char *due_date = gtk_entry_get_text(GTK_ENTRY(due_date_entry));
        const char *file_path = gtk_entry_get_text(GTK_ENTRY(file_entry));

        // Format deadline display
        char deadline_display[100] = "";
        if (strlen(start_date) > 0 && strlen(due_date) > 0) {
            snprintf(deadline_display, sizeof(deadline_display), "%s to %s", start_date, due_date);
        } else if (strlen(due_date) > 0) {
            snprintf(deadline_display, sizeof(deadline_display), "Due: %s", due_date);
        }

        // Add task to table (in real app, send to server with all fields)
        GtkWidget *task_tree = GTK_WIDGET(data);
        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(task_tree)));
        GtkTreeIter iter;
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, task_name,
            1, people[0] ? people : "",
            2, file_path[0] ? file_path : "",
            3, status ? status : "Not Started",
            4, deadline_display,
            -1);

        // In production: send this data to backend API
        // POST /api/tasks with: task_name, description, assigned_to, status, start_date, due_date
        (void)description; // Suppress unused warning for now
    }

    gtk_widget_destroy(dialog);
}

static void on_delete_task_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *tree_view = GTK_WIDGET(data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(project_view_window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
            "Do you want to delete this task?");

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
            gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
        }
        gtk_widget_destroy(dialog);
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(project_view_window),
            GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            "Please select a task to delete");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

static void on_cell_edited(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(data);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreePath *path = gtk_tree_path_new_from_string(path_string);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        GtkTreePath *col_path = gtk_tree_path_new_from_string(path_string);
        GtkTreeViewColumn *column = gtk_tree_view_get_column(tree_view, 0);
        int col_num = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(column), "column_num"));

        gtk_list_store_set(GTK_LIST_STORE(model), &iter, col_num, new_text, -1);
        gtk_tree_path_free(col_path);
    }
    gtk_tree_path_free(path);
}

static void on_chat_send(GtkWidget *widget, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "chat_entry"));
    GtkWidget *text_view = GTK_WIDGET(g_object_get_data(G_OBJECT(widget), "chat_view"));

    const char *message = gtk_entry_get_text(GTK_ENTRY(entry));
    if (strlen(message) > 0) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(buffer, &iter);

        char formatted_msg[512];
        snprintf(formatted_msg, sizeof(formatted_msg), "%s: %s\n", current_username, message);
        gtk_text_buffer_insert(buffer, &iter, formatted_msg, -1);

        gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
}

void show_project_view_screen() {
    if (project_list_window) gtk_widget_hide(project_list_window);

    project_view_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(project_view_window), "Project View - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(project_view_window), 1500, 850);
    gtk_window_set_position(GTK_WINDOW(project_view_window), GTK_WIN_POS_CENTER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(project_view_window), main_box);

    // Header with project name and back button
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header), 15);

    GtkWidget *back_btn = gtk_button_new_with_label("← Back");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_back_to_projects), NULL);
    gtk_box_pack_start(GTK_BOX(header), back_btn, FALSE, FALSE, 0);

    GtkWidget *project_title = gtk_label_new("Project Alpha *");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 20");
    gtk_widget_override_font(project_title, font_desc);
    gtk_box_pack_start(GTK_BOX(header), project_title, FALSE, FALSE, 10);

    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    // Main content area (horizontal split)
    GtkWidget *content_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(main_box), content_paned, TRUE, TRUE, 0);

    // Left side: Gantt chart and tasks
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Gantt chart button
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

    // Task table
    GtkWidget *task_frame = gtk_frame_new("Tasks");
    gtk_container_set_border_width(GTK_CONTAINER(task_frame), 10);

    GtkWidget *task_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(task_frame), task_box);

    // Task table buttons
    GtkWidget *task_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *add_task_btn = gtk_button_new_with_label("+ Add Task");
    GtkWidget *delete_task_btn = gtk_button_new_with_label("Delete Task");
    gtk_box_pack_start(GTK_BOX(task_buttons), add_task_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_buttons), delete_task_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(task_box), task_buttons, FALSE, FALSE, 5);

    // Task tree view
    GtkWidget *task_scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkListStore *store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *task_tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    // Task columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), task_tree);
    column = gtk_tree_view_column_new_with_attributes("Task", renderer, "text", 0, NULL);
    g_object_set_data(G_OBJECT(column), "column_num", GINT_TO_POINTER(0));
    gtk_tree_view_append_column(GTK_TREE_VIEW(task_tree), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), task_tree);
    column = gtk_tree_view_column_new_with_attributes("People", renderer, "text", 1, NULL);
    g_object_set_data(G_OBJECT(column), "column_num", GINT_TO_POINTER(1));
    gtk_tree_view_append_column(GTK_TREE_VIEW(task_tree), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), task_tree);
    column = gtk_tree_view_column_new_with_attributes("File", renderer, "text", 2, NULL);
    g_object_set_data(G_OBJECT(column), "column_num", GINT_TO_POINTER(2));
    gtk_tree_view_append_column(GTK_TREE_VIEW(task_tree), column);

    // Status column with combo
    GtkListStore *status_store = gtk_list_store_new(1, G_TYPE_STRING);
    GtkTreeIter status_iter;
    const char *statuses[] = {"Not Started", "In Progress", "Completed"};
    for (int i = 0; i < 3; i++) {
        gtk_list_store_append(status_store, &status_iter);
        gtk_list_store_set(status_store, &status_iter, 0, statuses[i], -1);
    }

    renderer = gtk_cell_renderer_combo_new();
    g_object_set(renderer, "editable", TRUE, "model", status_store, "text-column", 0, NULL);
    column = gtk_tree_view_column_new_with_attributes("Status", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(task_tree), column);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, NULL);
    g_signal_connect(renderer, "edited", G_CALLBACK(on_cell_edited), task_tree);
    column = gtk_tree_view_column_new_with_attributes("Deadline", renderer, "text", 4, NULL);
    g_object_set_data(G_OBJECT(column), "column_num", GINT_TO_POINTER(4));
    gtk_tree_view_append_column(GTK_TREE_VIEW(task_tree), column);

    gtk_container_add(GTK_CONTAINER(task_scrolled), task_tree);
    gtk_box_pack_start(GTK_BOX(task_box), task_scrolled, TRUE, TRUE, 0);

    g_object_set_data(G_OBJECT(add_task_btn), "task_tree", task_tree);
    g_object_set_data(G_OBJECT(delete_task_btn), "task_tree", task_tree);
    g_signal_connect(add_task_btn, "clicked", G_CALLBACK(on_add_task_clicked), task_tree);
    g_signal_connect(delete_task_btn, "clicked", G_CALLBACK(on_delete_task_clicked), task_tree);

    gtk_box_pack_start(GTK_BOX(left_box), task_frame, TRUE, TRUE, 0);

    // Right side: Chat panel
    GtkWidget *chat_frame = gtk_frame_new("Chat");
    gtk_container_set_border_width(GTK_CONTAINER(chat_frame), 10);

    GtkWidget *chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(chat_frame), chat_box);

    // Chat history
    GtkWidget *chat_scrolled = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *chat_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(chat_view), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(chat_scrolled), chat_view);
    gtk_box_pack_start(GTK_BOX(chat_box), chat_scrolled, TRUE, TRUE, 0);

    // Chat input
    GtkWidget *chat_input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *chat_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(chat_entry), "Type a message...");
    GtkWidget *send_btn = gtk_button_new_with_label("Send");

    g_object_set_data(G_OBJECT(send_btn), "chat_entry", chat_entry);
    g_object_set_data(G_OBJECT(send_btn), "chat_view", chat_view);
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
}
