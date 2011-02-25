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
#include <stdlib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#if HAVE_CLUTTER_GTK
#include <clutter-gtk/clutter-gtk.h>
#endif

#include "server-server.h"
#include "eek/eek.h"

static gboolean opt_system = FALSE;
static gboolean opt_session = FALSE;
static gchar *opt_address = NULL;

static const GOptionEntry options[] = {
    {"system", 'y', 0, G_OPTION_ARG_NONE, &opt_system,
     N_("Connect to the system bus")},
    {"session", 'e', 0, G_OPTION_ARG_NONE, &opt_session,
     N_("Connect to the session bus")},
    {"address", 'a', 0, G_OPTION_ARG_STRING, &opt_address,
     N_("Connect to the given D-Bus address")},
    {NULL}
};

static void
on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
}

static void
on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  exit (1);
}

int
main (int argc, char **argv)
{
    ServerServer *server;
    GBusType bus_type;
    GDBusConnection *connection;
    GError *error;
    GMainLoop *loop;
    guint owner_id;

#if HAVE_CLUTTER_GTK
    if (gtk_clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS) {
        g_printerr ("Can't init GTK with Clutter\n");
        exit (1);
    }
#else
    if (!gtk_init_check (&argc, &argv)) {
        g_printerr ("Can't init GTK\n");
        exit (1);
    }
#endif

    g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

    g_type_class_ref (EEK_TYPE_KEYBOARD);
    g_type_class_ref (EEK_TYPE_SECTION);
    g_type_class_ref (EEK_TYPE_KEY);
    g_type_class_ref (EEK_TYPE_SYMBOL);
    g_type_class_ref (EEK_TYPE_KEYSYM);

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

    server = server_server_new (SERVER_SERVER_PATH, connection);

    if (server == NULL) {
        g_printerr ("Can't create server\n");
        exit (1);
    }

    owner_id = g_bus_own_name_on_connection (connection,
                                             SERVER_SERVER_INTERFACE,
                                             G_BUS_NAME_OWNER_FLAGS_NONE,
                                             on_name_acquired,
                                             on_name_lost,
                                             NULL,
                                             NULL);
    if (owner_id == 0) {
        g_printerr ("Can't own the name\n");
        exit (1);
    }

    loop = g_main_loop_new (NULL, FALSE);
    g_main_loop_run (loop);

    g_bus_unown_name (owner_id);
    g_object_unref (server);
    g_object_unref (connection);
    g_main_loop_unref (loop);

    return 0;
}
