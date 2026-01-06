#include <gtk/gtk.h>
#include "login.h"

// Entry point; all screens are defined in separate translation units.
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    show_login_screen();
    gtk_main();
    return 0;
}
