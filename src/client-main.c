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

static gchar *opt_set_description = NULL;
static gint opt_set_group = -1;
static gboolean opt_show = FALSE;
static gboolean opt_hide = FALSE;
static gint opt_press_key = -1;
static gint opt_release_key = -1;
static gboolean opt_listen = FALSE;

static const GOptionEntry options[] = {
    {"set-description", '\0', 0, G_OPTION_ARG_STRING, &opt_set_description,
     "Set keyboard description from an XML file"},
    {"set-group", '\0', 0, G_OPTION_ARG_INT, &opt_set_group,
     "Set group of the keyboard"},
    {"show", '\0', 0, G_OPTION_ARG_NONE, &opt_show,
     "Show keyboard"},
    {"hide", '\0', 0, G_OPTION_ARG_NONE, &opt_hide,
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
    EekboardKeyboard *keyboard = NULL;
    GDBusConnection *connection = NULL;
    GError *error;
    GOptionContext *context;
    GMainLoop *loop = NULL;
    gint retval = 0;

    g_type_init ();
    g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

    context = g_option_context_new ("eekboard-client");
    g_option_context_add_main_entries (context, options, NULL);
    g_option_context_parse (context, &argc, &argv, NULL);
    g_option_context_free (context);

    error = NULL;
    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (error) {
        g_printerr ("%s\n", error->message);
        retval = 1;
        goto out;
    }

    error = NULL;
    keyboard = eekboard_keyboard_new ("/com/redhat/eekboard/Keyboard",
                                      connection,
                                      NULL,
                                      &error);
    if (error) {
        g_printerr ("%s\n", error->message);
        retval = 1;
        goto out;
    }

    if (opt_set_description) {
        GFile *file;
        GFileInputStream *input;
        EekLayout *layout;
        EekKeyboard *description;
        GError *error;

        file = g_file_new_for_path (opt_set_description);

        error = NULL;
        input = g_file_read (file, NULL, &error);
        if (error) {
            g_printerr ("Can't read file %s: %s\n",
                        opt_set_description, error->message);
            retval = 1;
            goto out;
        }

        layout = eek_xml_layout_new (G_INPUT_STREAM(input));
        g_object_unref (input);
        description = eek_keyboard_new (layout, 640, 480);
        g_object_unref (layout);
        eekboard_keyboard_set_description (keyboard, description);
        g_object_unref (description);
    }

    if (opt_set_group >= 0) {
        eekboard_keyboard_set_group (keyboard, opt_set_group);
    }

    if (opt_show) {
        eekboard_keyboard_show (keyboard);
    }

    if (opt_hide) {
        eekboard_keyboard_hide (keyboard);
    }

    if (opt_press_key >= 0) {
        eekboard_keyboard_press_key (keyboard, opt_press_key);
    }

    if (opt_release_key >= 0) {
        eekboard_keyboard_release_key (keyboard, opt_release_key);
    }

    if (opt_listen) {
        g_signal_connect (keyboard, "key-pressed",
                          G_CALLBACK(on_key_pressed), NULL);
        g_signal_connect (keyboard, "key-released",
                          G_CALLBACK(on_key_released), NULL);
        loop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (loop);
    }

 out:
    if (keyboard)
        g_object_unref (keyboard);
    if (connection)
        g_object_unref (connection);
    if (loop)
        g_main_loop_unref (loop);

    return retval;
}

