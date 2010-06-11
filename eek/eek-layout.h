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

#ifndef EEK_LAYOUT_H
#define EEK_LAYOUT_H 1

#include <glib-object.h>
#include "eek-types.h"

G_BEGIN_DECLS

#define EEK_TYPE_LAYOUT (eek_layout_get_type())
#define EEK_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_LAYOUT, EekLayout))
#define EEK_IS_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_LAYOUT))
#define EEK_LAYOUT_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), EEK_TYPE_LAYOUT, EekLayoutIface))

typedef struct _EekLayoutIface EekLayoutIface;
typedef struct _EekLayout EekLayout;

struct _EekLayoutIface
{
    /*< private >*/
    GTypeInterface parent_iface;

    /*< public >*/
    void (* apply)         (EekLayout   *self,
                            EekKeyboard *keyboard);
    gint (* get_group)     (EekLayout   *self);

    /* signals */
    void (* group_changed) (EekLayout   *self,
                            gint         group);
    void (* changed)       (EekLayout   *self);
};

GType eek_layout_get_type  (void) G_GNUC_CONST;
void  eek_layout_apply     (EekLayout   *layout,
                            EekKeyboard *keyboard);
gint  eek_layout_get_group (EekLayout   *layout);

G_END_DECLS
#endif  /* EEK_LAYOUT_H */
