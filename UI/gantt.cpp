#include <gtk/gtk.h>
#include <cairo.h>
#include "app_state.h"
#include "gantt.h"
#include "project_view.h"

static gboolean on_gantt_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    // Get drawing area size
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    // Background
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    // Grid settings
    int left_margin = 150;
    int top_margin = 50;
    int row_height = 40;
    int day_width = 60;
    int num_days = 14;
    int num_tasks = 5;

    // Draw title
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, left_margin, 30);
    cairo_show_text(cr, "Project Timeline - Gantt Chart");

    // Draw timeline header (dates)
    cairo_set_font_size(cr, 10);
    for (int i = 0; i < num_days; i++) {
        char day_label[20];
        snprintf(day_label, sizeof(day_label), "Day %d", i + 1);
        cairo_move_to(cr, left_margin + i * day_width + day_width/2 - 15, top_margin - 10);
        cairo_show_text(cr, day_label);
    }

    // Draw vertical grid lines
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_set_line_width(cr, 1);
    for (int i = 0; i <= num_days; i++) {
        cairo_move_to(cr, left_margin + i * day_width, top_margin);
        cairo_line_to(cr, left_margin + i * day_width, top_margin + num_tasks * row_height);
        cairo_stroke(cr);
    }

    // Draw horizontal grid lines
    for (int i = 0; i <= num_tasks; i++) {
        cairo_move_to(cr, left_margin, top_margin + i * row_height);
        cairo_line_to(cr, left_margin + num_days * day_width, top_margin + i * row_height);
        cairo_stroke(cr);
    }

    // Task data (name, start_day, duration, color)
    struct Task {
        const char *name;
        int start;
        int duration;
        double r, g, b;
    };

    Task tasks[] = {
        {"Requirements", 0, 3, 0.4, 0.7, 0.9},
        {"Design", 2, 4, 0.6, 0.8, 0.4},
        {"Development", 5, 6, 0.9, 0.6, 0.3},
        {"Testing", 10, 3, 0.9, 0.4, 0.4},
        {"Deployment", 12, 2, 0.6, 0.4, 0.8}
    };

    // Draw tasks
    cairo_set_font_size(cr, 12);
    for (int i = 0; i < num_tasks; i++) {
        Task &task = tasks[i];

        // Draw task name
        cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
        cairo_move_to(cr, 10, top_margin + i * row_height + row_height/2 + 5);
        cairo_show_text(cr, task.name);

        // Draw task bar
        int bar_x = left_margin + task.start * day_width;
        int bar_y = top_margin + i * row_height + 5;
        int bar_width = task.duration * day_width;
        int bar_height = row_height - 10;

        // Task bar with gradient
        cairo_set_source_rgb(cr, task.r, task.g, task.b);
        cairo_rectangle(cr, bar_x, bar_y, bar_width, bar_height);
        cairo_fill(cr);

        // Task bar border
        cairo_set_source_rgb(cr, task.r * 0.7, task.g * 0.7, task.b * 0.7);
        cairo_set_line_width(cr, 2);
        cairo_rectangle(cr, bar_x, bar_y, bar_width, bar_height);
        cairo_stroke(cr);

        // Duration text on bar
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_set_font_size(cr, 10);
        char duration_text[20];
        snprintf(duration_text, sizeof(duration_text), "%d days", task.duration);
        cairo_move_to(cr, bar_x + bar_width/2 - 20, bar_y + bar_height/2 + 4);
        cairo_show_text(cr, duration_text);
    }

    // Draw legend
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_set_font_size(cr, 10);
    cairo_move_to(cr, left_margin, top_margin + num_tasks * row_height + 30);
    cairo_show_text(cr, "Legend: Task bars show duration and timeline position");

    return FALSE;
}

static void on_close_gantt(GtkWidget *widget, gpointer data) {
    gtk_widget_hide(gantt_window);
    if (project_view_window) {
        gtk_widget_show(project_view_window);
    }
}

void show_gantt_chart_window() {
    if (project_view_window) gtk_widget_hide(project_view_window);

    gantt_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gantt_window), "Gantt Chart - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(gantt_window), 1400, 800);
    gtk_window_set_position(GTK_WINDOW(gantt_window), GTK_WIN_POS_CENTER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 15);
    gtk_container_add(GTK_CONTAINER(gantt_window), main_box);

    // Header with back button
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *back_btn = gtk_button_new_with_label("← Back to Project");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_close_gantt), NULL);
    gtk_box_pack_start(GTK_BOX(header), back_btn, FALSE, FALSE, 0);

    GtkWidget *title = gtk_label_new("Gantt Chart Visualization");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 16");
    gtk_widget_override_font(title, font_desc);
    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    // Drawing area for Gantt chart
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 1000, 400);
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_gantt_draw), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), drawing_area, TRUE, TRUE, 0);

    // Information panel
    GtkWidget *info_frame = gtk_frame_new("Chart Information");
    GtkWidget *info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(info_box), 10);
    gtk_container_add(GTK_CONTAINER(info_frame), info_box);

    GtkWidget *info_label = gtk_label_new(
        "• Each bar represents a task with its duration\n"
        "• Horizontal position shows when the task starts\n"
        "• Bar width indicates task duration in days\n"
        "• Different colors represent different tasks"
    );
    gtk_label_set_xalign(GTK_LABEL(info_label), 0);
    gtk_box_pack_start(GTK_BOX(info_box), info_label, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(main_box), info_frame, FALSE, FALSE, 0);

    g_signal_connect(gantt_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(gantt_window);
}
