/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
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
#ifndef EEK_XKB_LAYOUT_H
#define EEK_XKB_LAYOUT_H 1

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include "eek-layout.h"

G_BEGIN_DECLS

#define EEK_TYPE_XKB_LAYOUT (eek_xkb_layout_get_type())
#define EEK_XKB_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_XKB_LAYOUT, EekXkbLayout))
#define EEK_XKB_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_XKB_LAYOUT, EekXkbLayoutClass))
#define EEK_IS_XKB_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_XKB_LAYOUT))
#define EEK_IS_XKB_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_XKB_LAYOUT))
#define EEK_XKB_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_XKB_LAYOUT, EekXkbLayoutClass))

typedef struct _EekXkbLayout        EekXkbLayout;
typedef struct _EekXkbLayoutClass   EekXkbLayoutClass;
typedef struct _EekXkbLayoutPrivate EekXkbLayoutPrivate;

struct _EekXkbLayout
{
    /*< private >*/
    EekLayout parent;

    EekXkbLayoutPrivate *priv;
};

struct _EekXkbLayoutClass
{
    /*< private >*/
    EekLayoutClass parent_class;

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType                 eek_xkb_layout_get_type  (void) G_GNUC_CONST;
EekLayout            *eek_xkb_layout_new       (void);

gboolean              eek_xkb_layout_set_names (EekXkbLayout         *layout,
                                                XkbComponentNamesRec *names);

gboolean              eek_xkb_layout_set_names_full
                                               (EekXkbLayout         *layout,
                                                ...);
gboolean              eek_xkb_layout_set_names_full_valist
                                               (EekXkbLayout         *layout,
                                                va_list               var_args);
                                        
gboolean              eek_xkb_layout_set_keycodes
                                               (EekXkbLayout         *layout,
                                                const gchar          *keycodes);
gboolean              eek_xkb_layout_set_geometry
                                               (EekXkbLayout         *layout,
                                                const gchar          *geometry);
gboolean              eek_xkb_layout_set_symbols
                                               (EekXkbLayout         *layout,
                                                const gchar          *symbols);

G_CONST_RETURN gchar *eek_xkb_layout_get_keycodes
                                               (EekXkbLayout         *layout);
G_CONST_RETURN gchar *eek_xkb_layout_get_geometry
                                               (EekXkbLayout         *layout);
G_CONST_RETURN gchar *eek_xkb_layout_get_symbols
                                               (EekXkbLayout         *layout);

G_END_DECLS
#endif				/* #ifndef EEK_XKB_LAYOUT_H */
