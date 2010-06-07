/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#ifndef EEK_GTK_PRIVATE_H
#define EEK_GTK_PRIVATE_H 1

#include <glib/gtypes.h>
#include <gtk/gtk.h>

struct _EekGtkCallbackData {
    GFunc func;
    gpointer user_data;
};
typedef struct _EekGtkCallbackData EekGtkCallbackData;

void eek_gtk_callback (GtkWidget *actor, gpointer user_data);

#endif  /* EEK_GTK_PRIVATE_H */
