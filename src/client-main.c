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

#include "eekboard/eekboard.h"

static gchar *opt_set_keyboard = NULL;
static gint opt_set_group = -1;
static gboolean opt_show_keyboard = FALSE;
static gboolean opt_hide_keyboard = FALSE;
static gint opt_press_key = -1;
static gint opt_release_key = -1;
static gboolean opt_listen = FALSE;

static const GOptionEntry options[] = {
    {"set-keyboard", '\0', 0, G_OPTION_ARG_STRING, &opt_set_keyboard,
     "Set keyboard keyboard from an XML file"},
    {"set-group", '\0', 0, G_OPTION_ARG_INT, &opt_set_group,
     "Set group of the keyboard"},
    {"show-keyboard", '\0', 0, G_OPTION_ARG_NONE, &opt_show_keyboard,
     "Show keyboard"},
    {"hide-keyboard", '\0', 0, G_OPTION_ARG_NONE, &opt_hide_keyboard,
     "Hide keyboard"},
    {"press-key", '\0', 0, G_OPTION_ARG_INT, &opt_press_key,
     "Press key"},
    {"release-key", '\0', 0, G_OPTION_ARG_INT, &opt_release_key,
     "Release key"},
    {"listen", '\0', 0, G_OPTION_ARG_NONE, &opt_listen,
     "Listen events"},
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
    EekboardServer *server = NULL;
    EekboardContext *context = NULL;
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

    error = NULL;
    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (error) {
        g_printerr ("%s\n", error->message);
        retval = 1;
        goto out;
    }

    server = eekboard_server_new (connection, NULL);
    if (!server) {
        g_printerr ("Can't create server\n");
        retval = 1;
        goto out;
    }

    context = eekboard_server_create_context (server,
                                              "eekboard-client",
                                              NULL);
    if (!context) {
        g_printerr ("Can't create context\n");
        retval = 1;
        goto out;
    }

    eekboard_server_push_context (server, context, NULL);

    if (opt_set_keyboard) {
        GFile *file;
        GFileInputStream *input;
        EekLayout *layout;
        EekKeyboard *keyboard;

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

        eekboard_context_set_keyboard (context, keyboard, NULL);
        g_object_unref (keyboard);
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

