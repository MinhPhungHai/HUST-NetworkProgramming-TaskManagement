#include "app_state.h"
#include "../Client/network_wrapper.h"
#include <stdio.h>

GtkWidget *login_window = NULL;
GtkWidget *register_window = NULL;
GtkWidget *project_list_window = NULL;
GtkWidget *project_view_window = NULL;
GtkWidget *settings_window = NULL;
GtkWidget *gantt_window = NULL;

char current_username[256] = "";
char current_email[256] = "";
char current_user_id[256] = "";
int is_project_owner = 0;

char otp_code[10] = "";
char registered_email[256] = "";
char current_project_id[256] = "";

int app_network_init() {
    // Initialize and connect to server at localhost:8080
    if (!network_init("127.0.0.1", 8080)) {
        fprintf(stderr, "Failed to initialize network\n");
        return 0;
    }

    if (!network_connect()) {
        fprintf(stderr, "Failed to connect to server\n");
        return 0;
    }

    printf("Connected to server at 127.0.0.1:8080\n");
    return 1;
}

void app_network_cleanup() {
    network_disconnect();
    network_cleanup();
}
