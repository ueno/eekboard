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
#ifdef HAVE_ATSPI
#include <dbus/dbus.h>
#include <atspi/atspi.h>
#endif  /* HAVE_ATSPI */
#ifdef HAVE_IBUS
#include <ibus.h>
#endif  /* HAVE_IBUS */
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "eekboard/eekboard-client.h"
#include "client.h"

#define DEFAULT_KEYBOARD "us"

static gboolean opt_system = FALSE;
static gboolean opt_session = FALSE;
static gchar *opt_address = NULL;

static gboolean opt_focus = FALSE;
static gboolean opt_keystroke = FALSE;

static gchar *opt_keyboards = NULL;

static gboolean opt_fullscreen = FALSE;

static const GOptionEntry options[] = {
    {"system", 'y', 0, G_OPTION_ARG_NONE, &opt_system,
     N_("Connect to the system bus")},
    {"session", 'e', 0, G_OPTION_ARG_NONE, &opt_session,
     N_("Connect to the session bus")},
    {"address", 'a', 0, G_OPTION_ARG_STRING, &opt_address,
     N_("Connect to the given D-Bus address")},
#if ENABLE_FOCUS_LISTENER
    {"listen-focus", 'f', 0, G_OPTION_ARG_NONE, &opt_focus,
     N_("Listen focus change events")},
#endif  /* ENABLE_FOCUS_LISTENER */
#ifdef HAVE_ATSPI
    {"listen-keystroke", 's', 0, G_OPTION_ARG_NONE, &opt_keystroke,
     N_("Listen keystroke events with AT-SPI")},
#endif  /* HAVE_ATSPI */
    {"keyboards", 'k', 0, G_OPTION_ARG_STRING, &opt_keyboards,
     N_("Specify keyboards (comma separated)")},
    {"fullscreen", 'F', 0, G_OPTION_ARG_NONE, &opt_fullscreen,
     N_("Create window in fullscreen mode")},
    {NULL}
};

static void
on_context_destroyed (EekboardContext *context,
                      gpointer         user_data)
{
    GMainLoop *loop = user_data;

    g_main_loop_quit (loop);
}

static void
on_destroyed (EekboardClient *eekboard,
              gpointer          user_data)
{
    GMainLoop *loop = user_data;

    g_main_loop_quit (loop);
}

enum FocusListenerType {
    FOCUS_NONE,
    FOCUS_ATSPI,
    FOCUS_IBUS
};

static gboolean
set_keyboards (Client              *client,
               const gchar * const *keyboards)
{
    if (g_strv_length ((gchar **)keyboards) == 0) {
        if (!client_enable_xkl (client)) {
            g_printerr ("Can't register xklavier event listeners\n");
            return FALSE;
        }
    } else {
        if (!client_set_keyboards (client, keyboards)) {
            gchar *str = g_strjoinv (", ", (gchar **)keyboards);
            g_printerr ("Can't set keyboards \"%s\"\n", str);
            g_free (str);
            return FALSE;
        }
    }
    return TRUE;
}

int
main (int argc, char **argv)
{
    Client *client = NULL;
    EekboardClient *eekboard;
    EekboardContext *context;
    GBusType bus_type;
    GDBusConnection *connection;
    GError *error;
    GOptionContext *option_context;
    GMainLoop *loop = NULL;
    gint focus;
    GSettings *settings = NULL;
    gchar **keyboards = NULL;
    gint retval = 0;

    if (!gtk_init_check (&argc, &argv)) {
        g_printerr ("Can't init GTK\n");
        exit (1);
    }

    eek_init ();

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
        error = NULL;
        connection = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, &error);
        if (connection == NULL) {
            g_printerr ("Can't connect to the system bus: %s\n",
                        error->message);
            g_error_free (error);
            exit (1);
        }
        break;
    case G_BUS_TYPE_SESSION:
        error = NULL;
        connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
        if (connection == NULL) {
            g_printerr ("Can't connect to the session bus: %s\n",
                        error->message);
            g_error_free (error);
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
            g_error_free (error);
            exit (1);
        }
        break;
    default:
        g_assert_not_reached ();
        break;
    }

    client = client_new (connection);
    g_object_unref (connection);

    if (client == NULL) {
        g_printerr ("Can't create a client\n");
        exit (1);
    }

    settings = g_settings_new ("org.fedorahosted.eekboard");
    focus = FOCUS_NONE;
    if (opt_focus) {
        gchar *focus_listener = g_settings_get_string (settings,
                                                       "focus-listener");
        const struct {
            const gchar *name;
            enum FocusListenerType type;
        } focus_listeners[] = {
#ifdef HAVE_ATSPI
            { "atspi", FOCUS_ATSPI },
#endif
#ifdef HAVE_IBUS
            { "ibus", FOCUS_IBUS },
#endif
            { NULL }
        };
        gint i;

        focus = FOCUS_NONE;
        for (i = 0; focus_listeners[i].name; i++) {
            if (g_strcmp0 (focus_listener, focus_listeners[i].name) == 0)
                focus = focus_listeners[i].type;
        }
        if (focus == FOCUS_NONE) {
            g_printerr ("Unknown focus listener \"%s\".  "
                        "Try \"atspi\" or \"ibus\"\n", focus_listener);
            retval = 1;
            goto out;
        }
    }
        
#ifdef HAVE_ATSPI
    if (focus == FOCUS_ATSPI || opt_keystroke) {
        GSettings *desktop_settings =
            g_settings_new ("org.gnome.desktop.interface");
        gboolean accessibility_enabled =
            g_settings_get_boolean (desktop_settings, "toolkit-accessibility");
        g_object_unref (desktop_settings);

        if (accessibility_enabled) {
            if (atspi_init () != 0) {
                g_printerr ("Can't init AT-SPI 2\n");
                retval = 1;
                goto out;
            }

            if (focus == FOCUS_ATSPI &&
                !client_enable_atspi_focus (client)) {
                g_printerr ("Can't register AT-SPI focus change event listeners\n");
                retval = 1;
                goto out;
            }

            if (opt_keystroke &&
                !client_enable_atspi_keystroke (client)) {
                g_printerr ("Can't register AT-SPI keystroke event listeners\n");
                retval = 1;
                goto out;
            }
        } else {
            g_printerr ("Desktop accessibility support is disabled\n");
            retval = 1;
            goto out;
        }
    }
#endif  /* HAVE_ATSPI */

#ifdef HAVE_IBUS
    if (focus == FOCUS_IBUS) {
        ibus_init ();

        if (!client_enable_ibus_focus (client)) {
            g_printerr ("Can't register IBus focus change event listeners\n");
            retval = 1;
            goto out;
        }
    }
#endif  /* HAVE_IBUS */

#ifdef HAVE_XTEST
    if (!client_enable_xtest (client)) {
        g_printerr ("Can't init xtest\n");
        g_object_unref (client);
        exit (1);
    }
#endif  /* HAVE_XTEST */

    loop = g_main_loop_new (NULL, FALSE);

    if (!opt_focus) {
        g_object_get (client, "context", &context, NULL);
        g_signal_connect (context, "destroyed",
                          G_CALLBACK(on_context_destroyed), loop);
        g_object_unref (context);
    }

    if (opt_fullscreen ||
        g_settings_get_boolean (settings, "start-fullscreen")) {
        g_object_get (client, "context", &context, NULL);
        eekboard_context_set_fullscreen (context, TRUE, NULL);
        g_object_unref (context);
    }

    g_object_get (client, "eekboard", &eekboard, NULL);
    g_signal_connect (eekboard, "destroyed",
                      G_CALLBACK(on_destroyed), loop);
    g_object_unref (eekboard);
    
    if (opt_keyboards != NULL) {
        keyboards = g_strsplit (opt_keyboards, ",", -1);

        if (!set_keyboards (client, (const gchar * const *)keyboards)) {
            g_strfreev (keyboards);
            retval = 1;
            goto out;
        }
        g_strfreev (keyboards);
    }

    g_main_loop_run (loop);

 out:
    if (loop)
        g_main_loop_unref (loop);
    if (client)
        g_object_unref (client);
    if (settings)
        g_object_unref (settings);

    return retval;
}
