#pragma once

#include <gtk/gtk.h>

extern GtkWidget *login_window;
extern GtkWidget *register_window;
extern GtkWidget *project_list_window;
extern GtkWidget *project_view_window;
extern GtkWidget *settings_window;
extern GtkWidget *gantt_window;

extern char current_username[256];
extern char current_email[256];
extern int is_project_owner;

extern char otp_code[10];           // Stores generated OTP
extern char registered_email[256];  // Stores email for dialog display
