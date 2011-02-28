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
#include <glib/gi18n.h>

#include "eekboard/eekboard.h"

static gboolean opt_system = FALSE;
static gboolean opt_session = FALSE;
static gchar *opt_address = NULL;

static gchar *opt_set_keyboard = NULL;
static gint opt_set_group = -1;
static gboolean opt_show_keyboard = FALSE;
static gboolean opt_hide_keyboard = FALSE;
static gint opt_press_key = -1;
static gint opt_release_key = -1;
static gboolean opt_listen = FALSE;

static const GOptionEntry options[] = {
    {"system", 'y', 0, G_OPTION_ARG_NONE, &opt_system,
     N_("Connect to the system bus")},
    {"session", 'e', 0, G_OPTION_ARG_NONE, &opt_session,
     N_("Connect to the session bus")},
    {"address", 'a', 0, G_OPTION_ARG_STRING, &opt_address,
     N_("Connect to the given D-Bus address")},
    {"set-keyboard", '\0', 0, G_OPTION_ARG_STRING, &opt_set_keyboard,
     N_("Upload keyboard description from an XML file")},
    {"set-group", '\0', 0, G_OPTION_ARG_INT, &opt_set_group,
     N_("Set group of the keyboard")},
    {"show-keyboard", '\0', 0, G_OPTION_ARG_NONE, &opt_show_keyboard,
     N_("Show keyboard")},
    {"hide-keyboard", '\0', 0, G_OPTION_ARG_NONE, &opt_hide_keyboard,
     N_("Hide keyboard")},
    {"press-key", '\0', 0, G_OPTION_ARG_INT, &opt_press_key,
     N_("Press key")},
    {"release-key", '\0', 0, G_OPTION_ARG_INT, &opt_release_key,
     N_("Release key")},
    {"listen", '\0', 0, G_OPTION_ARG_NONE, &opt_listen,
     N_("Listen events")},
    {NULL}
};

static void
on_key_pressed (guint keycode, gpointer user_data)
{
    g_print ("KeyPressed %u\n", keycode);
}

static void
on_key_released (guint keycode, gpointer user_data)
{
    g_print ("KeyReleased %u\n", keycode);
}

int
main (int argc, char **argv)
{
    EekboardEekboard *eekboard = NULL;
    EekboardContext *context = NULL;
    GBusType bus_type;
    GDBusConnection *connection = NULL;
    GError *error;
    GOptionContext *option_context;
    GMainLoop *loop = NULL;
    gint retval = 0;

    g_type_init ();
    g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

    option_context = g_option_context_new ("eekboard-client");
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

    eekboard = eekboard_eekboard_new (connection, NULL);
    if (eekboard == NULL) {
        g_printerr ("Can't create eekboard proxy\n");
        retval = 1;
        goto out;
    }

    context = eekboard_eekboard_create_context (eekboard,
                                                "eekboard-client",
                                                NULL);
    if (context == NULL) {
        g_printerr ("Can't create context\n");
        retval = 1;
        goto out;
    }

    eekboard_eekboard_push_context (eekboard, context, NULL);

    if (opt_set_keyboard) {
        GFile *file;
        GFileInputStream *input;
        EekLayout *layout;
        EekKeyboard *keyboard;
        guint keyboard_id;

        file = g_file_new_for_path (opt_set_keyboard);

        error = NULL;
        input = g_file_read (file, NULL, &error);
        if (error) {
            g_printerr ("Can't read file %s: %s\n",
                        opt_set_keyboard, error->message);
            retval = 1;
            goto out;
        }

        layout = eek_xml_layout_new (G_INPUT_STREAM(input));
        g_object_unref (input);
        keyboard = eek_keyboard_new (layout, 640, 480);
        g_object_unref (layout);

        keyboard_id = eekboard_context_add_keyboard (context, keyboard, NULL);
        g_object_unref (keyboard);

        eekboard_context_set_keyboard (context, keyboard_id, NULL);
    }

    if (opt_set_group >= 0) {
        eekboard_context_set_group (context, opt_set_group, NULL);
    }

    if (opt_show_keyboard) {
        eekboard_context_show_keyboard (context, NULL);
    }

    if (opt_hide_keyboard) {
        eekboard_context_hide_keyboard (context, NULL);
    }

    if (opt_press_key >= 0) {
        eekboard_context_press_key (context, opt_press_key, NULL);
    }

    if (opt_release_key >= 0) {
        eekboard_context_release_key (context, opt_release_key, NULL);
    }

    if (opt_listen) {
        g_signal_connect (context, "key-pressed",
                          G_CALLBACK(on_key_pressed), NULL);
        g_signal_connect (context, "key-released",
                          G_CALLBACK(on_key_released), NULL);
        loop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (loop);
    }

 out:
    if (context)
        g_object_unref (context);
    if (connection)
        g_object_unref (connection);
    if (loop)
        g_main_loop_unref (loop);

    return retval;
}

