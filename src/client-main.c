/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <cspi/spi.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include "eekboard/eekboard.h"
#include "client.h"

static gboolean opt_system = FALSE;
static gboolean opt_session = FALSE;
static gchar *opt_address = NULL;

#ifdef HAVE_CSPI
static gboolean opt_focus = FALSE;
static gboolean opt_keystroke = FALSE;
#endif  /* HAVE_CSPI */

static gchar *opt_keyboard = NULL;

static gchar *opt_model = NULL;
static gchar *opt_layouts = NULL;
static gchar *opt_options = NULL;

static gboolean opt_fullscreen = FALSE;

static const GOptionEntry options[] = {
    {"system", 'y', 0, G_OPTION_ARG_NONE, &opt_system,
     N_("Connect to the system bus")},
    {"session", 'e', 0, G_OPTION_ARG_NONE, &opt_session,
     N_("Connect to the session bus")},
    {"address", 'a', 0, G_OPTION_ARG_STRING, &opt_address,
     N_("Connect to the given D-Bus address")},
#ifdef HAVE_CSPI
    {"listen-focus", 'f', 0, G_OPTION_ARG_NONE, &opt_focus,
     N_("Listen focus change events with AT-SPI")},
    {"listen-keystroke", 's', 0, G_OPTION_ARG_NONE, &opt_keystroke,
     N_("Listen keystroke events with AT-SPI")},
#endif  /* HAVE_CSPI */
    {"keyboard", 'k', 0, G_OPTION_ARG_STRING, &opt_keyboard,
     N_("Specify keyboard")},
    {"model", '\0', 0, G_OPTION_ARG_STRING, &opt_model,
     N_("Specify model")},
    {"layouts", '\0', 0, G_OPTION_ARG_STRING, &opt_layouts,
     N_("Specify layouts")},
    {"options", '\0', 0, G_OPTION_ARG_STRING, &opt_options,
     N_("Specify options")},
    {"fullscreen", 'F', 0, G_OPTION_ARG_NONE, &opt_fullscreen,
     N_("Create window in fullscreen mode")},
    {NULL}
};

static void
on_notify_keyboard_visible (GObject    *object,
                            GParamSpec *spec,
                            gpointer    user_data)
{
    GMainLoop *loop = user_data;
    gboolean visible;

    g_object_get (object, "keyboard-visible", &visible, NULL);

    /* user explicitly closed the window */
    if (!visible && eekboard_context_is_enabled (EEKBOARD_CONTEXT(object)))
        g_main_loop_quit (loop);
}

static void
on_context_destroyed (EekboardContext *context,
                      gpointer         user_data)
{
    GMainLoop *loop = user_data;

    g_main_loop_quit (loop);
}

static void
on_destroyed (EekboardEekboard *eekboard,
              gpointer          user_data)
{
    GMainLoop *loop = user_data;

    g_main_loop_quit (loop);
}

int
main (int argc, char **argv)
{
    EekboardClient *client;
    EekboardEekboard *eekboard;
    EekboardContext *context;
    GBusType bus_type;
    GDBusConnection *connection;
    GError *error;
    GConfClient *gconfc;
    GOptionContext *option_context;
    GMainLoop *loop;

    if (!gtk_init_check (&argc, &argv)) {
        g_printerr ("Can't init GTK\n");
        exit (1);
    }

    option_context = g_option_context_new ("eekboard-desktop-client");
    g_option_context_add_main_entries (option_context, options, NULL);
    g_option_context_parse (option_context, &argc, &argv, NULL);
    g_option_context_free (option_context);

    if (opt_system)
        bus_type = G_BUS_TYPE_SYSTEM;
    else if (opt_address)
        bus_type = G_BUS_TYPE_NONE;
    else
        bus_type = G_BUS_TYPE_SESSION;

    switch (bus_type) {
    case G_BUS_TYPE_SYSTEM:
    case G_BUS_TYPE_SESSION:
        error = NULL;
        connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
        if (connection == NULL) {
            g_printerr ("Can't connect to the bus: %s\n", error->message);
            exit (1);
        }
        break;
    case G_BUS_TYPE_NONE:
        error = NULL;
        connection = g_dbus_connection_new_for_address_sync (opt_address,
                                                             0,
                                                             NULL,
                                                             NULL,
                                                             &error);
        if (connection == NULL) {
            g_printerr ("Can't connect to the bus at %s: %s\n",
                        opt_address,
                        error->message);
            exit (1);
        }
        break;
    default:
        g_assert_not_reached ();
        break;
    }

    client = eekboard_client_new (connection);
    if (client == NULL) {
        g_printerr ("Can't create a client\n");
        exit (1);
    }

    gconfc = gconf_client_get_default ();

#ifdef HAVE_CSPI
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
                !eekboard_client_enable_cspi_focus (client)) {
                g_printerr ("Can't register focus change event listeners\n");
                exit (1);
            }

            if (opt_keystroke &&
                !eekboard_client_enable_cspi_keystroke (client)) {
                g_printerr ("Can't register keystroke event listeners\n");
                exit (1);
            }
        } else {
            g_printerr ("Desktop accessibility support is disabled\n");
            exit (1);
        }
    }
#endif  /* HAVE_CSPI */

    if (opt_keyboard && (opt_model || opt_layouts || opt_options)) {
        g_printerr ("Can't use --keyboard option with xklavier options\n");
        exit (1);
    }

    if (opt_keyboard) {
        gchar *file;

        if (g_str_has_suffix (opt_keyboard, ".xml"))
            file = g_strdup (opt_keyboard);
        else
            file = g_strdup_printf ("%s/%s.xml", KEYBOARDDIR, opt_keyboard);
        if (!eekboard_client_load_keyboard_from_file (client, file)) {
            g_printerr ("Can't load keyboard file %s\n", file);
            g_free (file);
            exit (1);
        }
        g_free (file);
    } else if (opt_model || opt_layouts || opt_options) {
        if (!eekboard_client_set_xkl_config (client,
                                             opt_model,
                                             opt_layouts,
                                             opt_options)) {
            g_printerr ("Can't set xklavier config\n");
            exit (1);
        }
    } else if (!eekboard_client_enable_xkl (client)) {
        g_printerr ("Can't register xklavier event listeners\n");
        exit (1);
    }

#ifdef HAVE_FAKEKEY
    if (!eekboard_client_enable_fakekey (client)) {
        g_printerr ("Can't init fakekey\n");
        exit (1);
    }
#endif  /* HAVE_FAKEKEY */

    loop = g_main_loop_new (NULL, FALSE);
    if (!opt_focus) {
        g_object_get (client, "context", &context, NULL);
        g_signal_connect (context, "notify::keyboard-visible",
                          G_CALLBACK(on_notify_keyboard_visible), loop);
        g_signal_connect (context, "destroyed",
                          G_CALLBACK(on_context_destroyed), loop);
        g_object_unref (context);
    }

    if (opt_fullscreen) {
        g_object_get (client, "context", &context, NULL);
        eekboard_context_set_fullscreen (context, TRUE, NULL);
        g_object_unref (context);
    }

    g_object_get (client, "eekboard", &eekboard, NULL);
    g_signal_connect (eekboard, "destroyed",
                      G_CALLBACK(on_destroyed), loop);

    g_main_loop_run (loop);
    g_main_loop_unref (loop);

    return 0;
}
