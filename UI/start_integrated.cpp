#include <gtk/gtk.h>
#include <stdio.h>
#include "login.h"
#include "app_state.h"

// Entry point; all screens are defined in separate translation units.
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Initialize network connection to server
    printf("Connecting to server at 127.0.0.1:8080...\n");

    if (!app_network_init()) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_OK,
                                                   "Failed to connect to server!\n\n"
                                                   "Make sure the server is running:\n"
                                                   "./build/server");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return 1;
    }

    printf("Successfully connected to server!\n");

    // Show login screen
    show_login_screen();

    // Run GTK main loop
    gtk_main();

    // Cleanup network on exit
    app_network_cleanup();

    return 0;
}
