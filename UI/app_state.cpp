#include "app_state.h"

GtkWidget *login_window = NULL;
GtkWidget *register_window = NULL;
GtkWidget *project_list_window = NULL;
GtkWidget *project_view_window = NULL;
GtkWidget *settings_window = NULL;
GtkWidget *gantt_window = NULL;

char current_username[256] = "";
char current_email[256] = "";
int is_project_owner = 0;

char otp_code[10] = "";
char registered_email[256] = "";
