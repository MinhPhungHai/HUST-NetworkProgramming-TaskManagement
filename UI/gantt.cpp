#include <gtk/gtk.h>
#include <cairo.h>
#include <cstdio>
#include <ctime>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "app_state.h"
#include "gantt.h"
#include "project_view.h"
#include "../Client/network_wrapper.h"

using nlohmann::json;

namespace {
struct TaskBar {
    std::string name;
    int start_day;
    int duration;
    double r;
    double g;
    double b;
};

std::vector<TaskBar> g_gantt_tasks;
int g_total_days = 14;

bool parse_date(const std::string &value, std::tm &out_tm) {
    if (value.size() < 10) {
        return false;
    }
    int year = 0, month = 0, day = 0;
    if (sscanf(value.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        return false;
    }
    memset(&out_tm, 0, sizeof(out_tm));
    out_tm.tm_year = year - 1900;
    out_tm.tm_mon = month - 1;
    out_tm.tm_mday = day;
    out_tm.tm_isdst = -1;
    return true;
}

int days_between(time_t start, time_t end) {
    double seconds = difftime(end, start);
    return static_cast<int>(seconds / 86400.0);
}

void load_gantt_tasks() {
    g_gantt_tasks.clear();
    g_total_days = 14;

    if (!network_is_connected()) {
        return;
    }

    char response[8192];
    if (!network_get_tasks(current_project_id, response, sizeof(response))) {
        return;
    }

    json payload = json::parse(response, nullptr, false);
    if (payload.is_discarded() || payload.value("status", 1) != 0) {
        return;
    }

    if (!payload.contains("tasks") || !payload["tasks"].is_array()) {
        return;
    }

    std::vector<std::pair<time_t, time_t>> ranges;
    struct Color { double r; double g; double b; };
    Color colors[] = {
        {0.4, 0.7, 0.9}, {0.6, 0.8, 0.4}, {0.9, 0.6, 0.3},
        {0.9, 0.4, 0.4}, {0.6, 0.4, 0.8}, {0.3, 0.6, 0.8}
    };
    int color_count = sizeof(colors) / sizeof(colors[0]);
    int color_index = 0;

    for (const auto &task : payload["tasks"]) {
        std::string name = task.value("task_name", "");
        std::string start_date = task.value("start_date", "");
        std::string due_date = task.value("due_date", "");
        std::string created_at = task.value("created_at", "");

        std::tm start_tm{};
        std::tm end_tm{};
        bool has_start = parse_date(start_date, start_tm);
        bool has_end = parse_date(due_date, end_tm);
        if (!has_start && !created_at.empty()) {
            std::string created_date = created_at.size() >= 10 ? created_at.substr(0, 10) : created_at;
            has_start = parse_date(created_date, start_tm);
        }
        if (!has_start && !has_end) {
            continue;
        }
        if (!has_start) {
            start_tm = end_tm;
        }
        if (!has_end) {
            end_tm = start_tm;
        }

        time_t start_time = mktime(&start_tm);
        time_t end_time = mktime(&end_tm);
        if (end_time < start_time) {
            std::swap(end_time, start_time);
        }
        ranges.emplace_back(start_time, end_time);

        TaskBar bar;
        bar.name = name;
        bar.r = colors[color_index % color_count].r;
        bar.g = colors[color_index % color_count].g;
        bar.b = colors[color_index % color_count].b;
        g_gantt_tasks.push_back(bar);
        color_index++;
    }

    if (ranges.empty()) {
        return;
    }

    time_t min_start = ranges[0].first;
    time_t max_end = ranges[0].second;
    for (const auto &range : ranges) {
        if (range.first < min_start) min_start = range.first;
        if (range.second > max_end) max_end = range.second;
    }

    g_total_days = days_between(min_start, max_end) + 1;
    if (g_total_days < 1) {
        g_total_days = 1;
    }

    for (size_t i = 0; i < g_gantt_tasks.size(); ++i) {
        int start_offset = days_between(min_start, ranges[i].first);
        int duration = days_between(ranges[i].first, ranges[i].second) + 1;
        g_gantt_tasks[i].start_day = start_offset;
        g_gantt_tasks[i].duration = duration > 0 ? duration : 1;
    }
}

gboolean on_gantt_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    int left_margin = 180;
    int top_margin = 60;
    int row_height = 40;
    int day_width = g_total_days > 0 ? (width - left_margin - 40) / g_total_days : 60;
    if (day_width < 40) day_width = 40;
    int num_tasks = static_cast<int>(g_gantt_tasks.size());

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
    cairo_move_to(cr, left_margin, 30);
    cairo_show_text(cr, "Project Timeline - Gantt Chart");

    cairo_set_font_size(cr, 10);
    for (int i = 0; i < g_total_days; i++) {
        char day_label[20];
        snprintf(day_label, sizeof(day_label), "Day %d", i + 1);
        cairo_move_to(cr, left_margin + i * day_width + day_width / 2 - 15, top_margin - 10);
        cairo_show_text(cr, day_label);
    }

    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_set_line_width(cr, 1);
    for (int i = 0; i <= g_total_days; i++) {
        cairo_move_to(cr, left_margin + i * day_width, top_margin);
        cairo_line_to(cr, left_margin + i * day_width, top_margin + num_tasks * row_height);
        cairo_stroke(cr);
    }

    for (int i = 0; i <= num_tasks; i++) {
        cairo_move_to(cr, left_margin, top_margin + i * row_height);
        cairo_line_to(cr, left_margin + g_total_days * day_width, top_margin + i * row_height);
        cairo_stroke(cr);
    }

    cairo_set_font_size(cr, 12);
    for (int i = 0; i < num_tasks; i++) {
        TaskBar &task = g_gantt_tasks[i];

        cairo_set_source_rgb(cr, 0.2, 0.2, 0.2);
        cairo_move_to(cr, 10, top_margin + i * row_height + row_height / 2 + 5);
        cairo_show_text(cr, task.name.c_str());

        int bar_x = left_margin + task.start_day * day_width;
        int bar_y = top_margin + i * row_height + 5;
        int bar_width = task.duration * day_width;
        int bar_height = row_height - 10;

        cairo_set_source_rgb(cr, task.r, task.g, task.b);
        cairo_rectangle(cr, bar_x, bar_y, bar_width, bar_height);
        cairo_fill(cr);

        cairo_set_source_rgb(cr, task.r * 0.7, task.g * 0.7, task.b * 0.7);
        cairo_set_line_width(cr, 2);
        cairo_rectangle(cr, bar_x, bar_y, bar_width, bar_height);
        cairo_stroke(cr);

        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_set_font_size(cr, 10);
        char duration_text[20];
        snprintf(duration_text, sizeof(duration_text), "%d days", task.duration);
        cairo_move_to(cr, bar_x + bar_width / 2 - 20, bar_y + bar_height / 2 + 4);
        cairo_show_text(cr, duration_text);
    }

    if (num_tasks == 0) {
        cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
        cairo_set_font_size(cr, 14);
        cairo_move_to(cr, left_margin, top_margin + 40);
        cairo_show_text(cr, "No tasks with dates to display.");
    }

    return FALSE;
}

void on_close_gantt(GtkWidget *widget, gpointer data) {
    gtk_widget_hide(gantt_window);
    if (project_view_window) {
        gtk_widget_show(project_view_window);
    }
}
} // namespace

void show_gantt_chart_window() {
    if (project_view_window) gtk_widget_hide(project_view_window);

    load_gantt_tasks();

    gantt_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gantt_window), "Gantt Chart - Project Management System");
    gtk_window_set_default_size(GTK_WINDOW(gantt_window), 1200, 700);
    gtk_window_set_position(GTK_WINDOW(gantt_window), GTK_WIN_POS_CENTER);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 15);
    gtk_container_add(GTK_CONTAINER(gantt_window), main_box);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *back_btn = gtk_button_new_with_label("← Back to Project");
    g_signal_connect(back_btn, "clicked", G_CALLBACK(on_close_gantt), NULL);
    gtk_box_pack_start(GTK_BOX(header), back_btn, FALSE, FALSE, 0);

    GtkWidget *title = gtk_label_new("Gantt Chart Visualization");
    PangoFontDescription *font_desc = pango_font_description_from_string("Sans 16");
    gtk_widget_override_font(title, font_desc);
    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 1000, 400);
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(on_gantt_draw), NULL);
    gtk_box_pack_start(GTK_BOX(main_box), drawing_area, TRUE, TRUE, 0);

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
