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
#ifndef EEK_XKL_LAYOUT_H
#define EEK_XKL_LAYOUT_H 1

#include <libxklavier/xklavier.h>
#include "eek-xkb-layout.h"

G_BEGIN_DECLS

#define EEK_TYPE_XKL_LAYOUT (eek_xkl_layout_get_type())
#define EEK_XKL_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_XKL_LAYOUT, EekXklLayout))
#define EEK_XKL_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_XKL_LAYOUT, EekXklLayoutClass))
#define EEK_IS_XKL_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_XKL_LAYOUT))
#define EEK_IS_XKL_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_XKL_LAYOUT))
#define EEK_XKL_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_XKL_LAYOUT, EekXklLayoutClass))

typedef struct _EekXklLayout        EekXklLayout;
typedef struct _EekXklLayoutClass   EekXklLayoutClass;
typedef struct _EekXklLayoutPrivate EekXklLayoutPrivate;

struct _EekXklLayout
{
    /*< private >*/
    EekXkbLayout parent;

    EekXklLayoutPrivate *priv;
};

struct _EekXklLayoutClass
{
    /*< private >*/
    EekXkbLayoutClass parent_class;

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType      eek_xkl_layout_get_type        (void) G_GNUC_CONST;

EekLayout *eek_xkl_layout_new             (void);

gboolean   eek_xkl_layout_set_config      (EekXklLayout *layout,
                                           XklConfigRec *config);

gboolean   eek_xkl_layout_set_config_full (EekXklLayout *layout,
                                           gchar        *model,
                                           gchar       **layouts,
                                           gchar       **variants,
                                           gchar       **options);

gboolean   eek_xkl_layout_set_model       (EekXklLayout *layout,
                                           const gchar  *model);
gboolean   eek_xkl_layout_set_layouts     (EekXklLayout *layout,
                                           gchar       **layouts);
gboolean   eek_xkl_layout_set_variants    (EekXklLayout *layout,
                                           gchar       **variants);
gboolean   eek_xkl_layout_set_options     (EekXklLayout *layout,
                                           gchar       **options);
gboolean   eek_xkl_layout_enable_option   (EekXklLayout *layout,
                                           const gchar  *option);
gboolean   eek_xkl_layout_disable_option  (EekXklLayout *layout,
                                           const gchar  *option);

gchar     *eek_xkl_layout_get_model       (EekXklLayout *layout);
gchar    **eek_xkl_layout_get_layouts     (EekXklLayout *layout);
gchar    **eek_xkl_layout_get_variants    (EekXklLayout *layout);
gchar    **eek_xkl_layout_get_options     (EekXklLayout *layout);
gboolean   eek_xkl_layout_get_option      (EekXklLayout *layout,
                                           const gchar  *option);

G_END_DECLS
#endif				/* #ifndef EEK_XKL_LAYOUT_H */
