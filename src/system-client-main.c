#include <stdlib.h>
#include <cspi/spi.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include "system-client.h"

gboolean opt_keyboard = FALSE;
gboolean opt_focus = FALSE;
gboolean opt_keystroke = FALSE;
gboolean opt_fakekey = FALSE;

static const GOptionEntry options[] = {
    {"listen-keyboard", 'k', 0, G_OPTION_ARG_NONE, &opt_keyboard,
     "Listen keyboard change events with libxklavier"},
    {"listen-focus", 'f', 0, G_OPTION_ARG_NONE, &opt_focus,
     "Listen focus change events with AT-SPI"},
    {"listen-keystroke", 's', 0, G_OPTION_ARG_NONE, &opt_keystroke,
     "Listen keystroke events with AT-SPI"},
    {"generate-key-event", 'g', 0, G_OPTION_ARG_NONE, &opt_fakekey,
     "Generate X key events with libfakekey"},
    {NULL}
};

int
main (int argc, char **argv)
{
    EekboardSystemClient *client;
    GDBusConnection *connection;
    GError *error;
    GConfClient *gconfc;
    GOptionContext *context;

    if (!gtk_init_check (&argc, &argv)) {
        g_printerr ("Can't init GTK\n");
        exit (1);
    }

    context = g_option_context_new ("eekboard-system-client");
    g_option_context_add_main_entries (context, options, NULL);
    g_option_context_parse (context, &argc, &argv, NULL);
    g_option_context_free (context);

    error = NULL;
    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (error) {
        g_printerr ("%s\n", error->message);
        exit (1);
    }
    client = eekboard_system_client_new (connection);

    gconfc = gconf_client_get_default ();
    error = NULL;
    if (opt_focus || opt_keystroke) {
        if (gconf_client_get_bool (gconfc,
                                   "/desktop/gnome/interface/accessibility",
                                   &error) ||
            gconf_client_get_bool (gconfc,
                                   "/desktop/gnome/interface/accessibility2",
                                   &error)) {
            if (SPI_init () != 0) {
                g_printerr ("Can't init CSPI\n");
                exit (1);
            }

            if (opt_focus &&
                !eekboard_system_client_enable_cspi_focus (client)) {
                g_printerr ("Can't register focus change event listeners\n");
                exit (1);
            }

            if (opt_keystroke &&
                !eekboard_system_client_enable_cspi_keystroke (client)) {
                g_printerr ("Can't register keystroke event listeners\n");
                exit (1);
            }
        } else {
            g_printerr ("System accessibility support is disabled");
            exit (1);
        }
    }
    if (opt_keyboard &&
        !eekboard_system_client_enable_xkl (client)) {
        g_printerr ("Can't register xklavier event listeners\n"); 
        exit (1);
    }

    if (opt_fakekey &&
        !eekboard_system_client_enable_fakekey (client)) {
        g_printerr ("Can't init fakekey\n"); 
        exit (1);
    }

    gtk_main ();

    return 0;
}
