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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "server.h"
#include "eek/eek.h"

int
main (int argc, char **argv)
{
    EekboardServer *server;
    GDBusConnection *connection;
    GError *error;
    GMainLoop *loop;

    if (!gtk_init_check (&argc, &argv)) {
        g_warning ("Can't init GTK");
        exit (1);
    }

    error = NULL;
    connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);
    if (error) {
        g_printerr ("%s\n", error->message);
        exit (1);
    }

    server = eekboard_server_new (connection);

    loop = g_main_loop_new(NULL, FALSE);

    if (!eekboard_server_start (server)) {
        g_printerr ("Can't start server\n");
        exit (1);
    }
    g_main_loop_run(loop);
    eekboard_server_stop (server);

    g_object_unref(server);
    g_object_unref(connection);
    g_main_loop_unref(loop);

    return 0;
}
