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

#include "proxy.h"

static gchar *opt_set_keyboard = NULL;
static gboolean opt_show = FALSE;
static gboolean opt_hide = FALSE;
static gboolean opt_listen = FALSE;

static const GOptionEntry options[] = {
    {"set-keyboard", '\0', 0, G_OPTION_ARG_STRING, &opt_set_keyboard,
     "Set keyboard from an XML file"},
    {"show", '\0', 0, G_OPTION_ARG_NONE, &opt_show,
     "Show keyboard"},
    {"hide", '\0', 0, G_OPTION_ARG_NONE, &opt_hide,
     "Hide keyboard"},
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
    EekboardProxy *proxy = NULL;
    GDBusConnection *connection = NULL;
    GError *error;
    GOptionContext *context;
    GMainLoop *loop = NULL;
    gint retval = 0;

    g_type_init ();

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
    proxy = eekboard_proxy_new ("/com/redhat/eekboard/Keyboard",
                                 connection,
                                 NULL,
                                 &error);
    if (error) {
        g_printerr ("%s\n", error->message);
        retval = 1;
        goto out;
    }

    if (opt_set_keyboard) {
        GFile *file;
        GFileInputStream *input;
        EekLayout *layout;
        EekKeyboard *keyboard;
        GError *error;

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
        eekboard_proxy_set_keyboard (proxy, keyboard);
        g_object_unref (keyboard);
    }

    if (opt_show) {
        eekboard_proxy_show (proxy);
    }

    if (opt_hide) {
        eekboard_proxy_hide (proxy);
    }

    if (opt_listen) {
        g_signal_connect (proxy, "key-pressed",
                          G_CALLBACK(on_key_pressed), NULL);
        g_signal_connect (proxy, "key-released",
                          G_CALLBACK(on_key_released), NULL);
        loop = g_main_loop_new (NULL, FALSE);
        g_main_loop_run (loop);
    }

 out:
    if (proxy)
        g_object_unref (proxy);
    if (connection)
        g_object_unref (connection);
    if (loop)
        g_main_loop_unref (loop);

    return retval;
}

