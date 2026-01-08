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
extern char current_user_id[256];  // User ID from server
extern int is_project_owner;

extern char otp_code[10];           // Stores generated OTP
extern char registered_email[256];  // Stores email for dialog display
extern char current_project_id[256];  // Current project ID
extern char current_project_name[256];

// Network functions
int app_network_init();
void app_network_cleanup();
