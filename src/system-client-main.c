#include <stdlib.h>
#include <cspi/spi.h>
#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include "system-client.h"

gboolean opt_xkl = FALSE;
gboolean opt_cspi = FALSE;
gboolean opt_fakekey = FALSE;

static const GOptionEntry options[] = {
    {"xklavier", 'x', 0, G_OPTION_ARG_NONE, &opt_xkl,
     "Listen xklavier events"},
    {"accessibility", 'a', 0, G_OPTION_ARG_NONE, &opt_cspi,
     "Listen accessibility events"},
    {"fakekey", 'k', 0, G_OPTION_ARG_NONE, &opt_fakekey,
     "Generate X key events via libfakekey"},
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
    if (opt_cspi) {
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

            if (!eekboard_system_client_enable_cspi (client)) {
                g_printerr ("Can't register accessibility event listeners\n");
                exit (1);
            }
        } else {
            g_printerr ("System accessibility support is disabled");
            exit (1);
        }
    }
    if (opt_xkl &&
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
