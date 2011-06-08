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
#include "eekboard/eekboard.h"
#include "client.h"

#define DEFAULT_LAYOUT "us-qwerty"

static gboolean opt_system = FALSE;
static gboolean opt_session = FALSE;
static gchar *opt_address = NULL;

static gboolean opt_use_system_layout = FALSE;
static gboolean opt_focus = FALSE;
static gchar *opt_focus_method = NULL;
static gboolean opt_keystroke = FALSE;

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
    {"use-system-layout", 'x', 0, G_OPTION_ARG_NONE, &opt_use_system_layout,
     N_("Use system keyboard layout")},
#if ENABLE_FOCUS_LISTENER
    {"listen-focus", 'f', 0, G_OPTION_ARG_NONE, &opt_focus,
     N_("Listen focus change events")},
    {"focus-method", '\0', 0, G_OPTION_ARG_STRING, &opt_focus_method,
     N_("Use the given focus method (\"atspi\" or \"ibus\")")},
#endif  /* ENABLE_FOCUS_LISTENER */
#ifdef HAVE_ATSPI
    {"listen-keystroke", 's', 0, G_OPTION_ARG_NONE, &opt_keystroke,
     N_("Listen keystroke events with AT-SPI")},
#endif  /* HAVE_ATSPI */
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

enum {
    FOCUS_NONE,
    FOCUS_ATSPI,
    FOCUS_IBUS
};

int
main (int argc, char **argv)
{
    EekboardClient *client;
    EekboardEekboard *eekboard;
    EekboardContext *context;
    GBusType bus_type;
    GDBusConnection *connection;
    GError *error;
    GOptionContext *option_context;
    GMainLoop *loop;
    gint focus;

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

    focus = FOCUS_NONE;
    if (opt_focus) {
        if (opt_focus_method == NULL ||
            g_strcmp0 (opt_focus_method, "atspi") == 0)
            focus = FOCUS_ATSPI;
        else if (g_strcmp0 (opt_focus_method, "ibus") == 0)
            focus = FOCUS_IBUS;
        else {
            g_printerr ("Unknown focus method \"%s\".  "
                        "Try \"atspi\" or \"ibus\"\n", opt_focus_method);
            exit (1);
        }
    }
        
#ifdef HAVE_ATSPI
    if (focus == FOCUS_ATSPI || opt_keystroke) {
        GSettings *settings = g_settings_new ("org.gnome.desktop.interface");

        if (g_settings_get_boolean (settings, "toolkit-accessibility")) {
            if (atspi_init () != 0) {
                g_printerr ("Can't init AT-SPI 2\n");
                exit (1);
            }

            if (focus == FOCUS_ATSPI &&
                !eekboard_client_enable_atspi_focus (client)) {
                g_printerr ("Can't register AT-SPI focus change event listeners\n");
                exit (1);
            }

            if (opt_keystroke &&
                !eekboard_client_enable_atspi_keystroke (client)) {
                g_printerr ("Can't register AT-SPI keystroke event listeners\n");
                exit (1);
            }
        } else {
            g_printerr ("Desktop accessibility support is disabled\n");
            exit (1);
        }
    }
#endif  /* HAVE_ATSPI */

#ifdef HAVE_IBUS
    if (focus == FOCUS_IBUS) {
        ibus_init ();

        if (focus == FOCUS_IBUS &&
            !eekboard_client_enable_ibus_focus (client)) {
            g_printerr ("Can't register IBus focus change event listeners\n");
            exit (1);
        }
    }
#endif  /* HAVE_IBUS */

    if (opt_use_system_layout && (opt_keyboard || opt_model || opt_layouts || opt_options)) {
        g_printerr ("Can't use --use-system-layout option with keyboard options\n");
        exit (1);
    }

    if (opt_use_system_layout) {
        if (!eekboard_client_enable_xkl (client)) {
            g_printerr ("Can't register xklavier event listeners\n");
            exit (1);
        }
    } else if (opt_model || opt_layouts || opt_options) {
        if (!eekboard_client_set_xkl_config (client,
                                             opt_model,
                                             opt_layouts,
                                             opt_options)) {
            g_printerr ("Can't set xklavier config\n");
            exit (1);
        }
    } else {
        gchar *file;

        if (!opt_keyboard)
            opt_keyboard = DEFAULT_LAYOUT;

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
