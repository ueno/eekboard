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
#ifndef EEK_KEY_H
#define EEK_KEY_H 1

#include <glib-object.h>
#include "eek-types.h"

G_BEGIN_DECLS

#define EEK_TYPE_KEY (eek_key_get_type())
#define EEK_KEY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_KEY, EekKey))
#define EEK_IS_KEY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_KEY))
#define EEK_KEY_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), EEK_TYPE_KEY, EekKeyIface))

typedef struct _EekKeyIface EekKeyIface;

struct _EekKeyIface
{
    /*< private >*/
    GTypeInterface g_iface;

    /*< public >*/
    void        (* set_keysyms)      (EekKey     *self,
                                      guint      *keysyms,
                                      gint        groups,
                                      gint        levels);
    gint        (* get_groups)       (EekKey     *self);
    guint       (* get_keysym)       (EekKey     *self);

    void        (* set_index)        (EekKey     *self,
                                      gint        column,
                                      gint        row);
    void        (* get_index)        (EekKey     *self,
                                      gint       *column,
                                      gint       *row);

    void        (* set_outline)      (EekKey     *self,
                                      EekOutline *outline);
    EekOutline *(* get_outline)      (EekKey     *self);
    void        (* set_bounds)       (EekKey     *self,
                                      EekBounds  *bounds);
    void        (* get_bounds)       (EekKey     *self,
                                      EekBounds  *bounds);

    void        (* set_keysym_index) (EekKey     *self,
                                      gint        group,
                                      gint        level);
    void        (* get_keysym_index) (EekKey     *self,
                                      gint       *group,
                                      gint       *level);
};

GType       eek_key_get_type         (void) G_GNUC_CONST;

void        eek_key_set_keysyms      (EekKey     *key,
                                      guint      *keysyms,
                                      gint        groups,
                                      gint        levels);
gint        eek_key_get_groups       (EekKey     *key);
guint       eek_key_get_keysym       (EekKey     *key);

void        eek_key_set_index        (EekKey     *key,
                                      gint        column,
                                      gint        row);
void        eek_key_get_index        (EekKey     *key,
                                      gint       *column,
                                      gint       *row);

void        eek_key_set_outline      (EekKey     *key,
                                      EekOutline *outline);
EekOutline *eek_key_get_outline      (EekKey     *key);
void        eek_key_set_bounds       (EekKey     *key,
                                      EekBounds  *bounds);
void        eek_key_get_bounds       (EekKey     *key,
                                      EekBounds  *bounds);

void        eek_key_set_keysym_index (EekKey     *key,
                                      gint        group,
                                      gint        level);
void        eek_key_get_keysym_index (EekKey     *key,
                                      gint       *group,
                                      gint       *level);

G_END_DECLS
#endif  /* EEK_KEY_H */
