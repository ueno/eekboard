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
#ifndef EEK_SECTION_H
#define EEK_SECTION_H 1

#include "eek-key.h"

G_BEGIN_DECLS

#define EEK_TYPE_SECTION (eek_section_get_type())
#define EEK_SECTION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_SECTION, EekSection))
#define EEK_IS_SECTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_SECTION))
#define EEK_SECTION_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), EEK_TYPE_SECTION, EekSectionIface))

typedef struct _EekSectionIface EekSectionIface;
typedef struct _EekSection EekSection;

struct _EekSectionIface
{
    /*< private >*/
    GTypeInterface g_iface;

    /*< public >*/
    void    (* set_dimensions) (EekSection  *self,
                                gint         columns,
                                gint         rows);
    void    (* get_dimensions) (EekSection  *self,
                                gint        *columns,
                                gint        *rows);
    void    (* set_angle)      (EekSection  *self,
                                gint         angle);
    gint    (* get_angle)      (EekSection  *self);

    void    (* set_bounds)     (EekSection  *self,
                                EekBounds   *bounds);
    void    (* get_bounds)     (EekSection  *self,
                                EekBounds   *bounds);

    EekKey *(* create_key)     (EekSection  *self,
                                const gchar *name,
                                guint       *keysyms,
                                gint         num_groups,
                                gint         num_levels,
                                gint         column,
                                gint         row,
                                EekOutline  *outline,
                                EekBounds   *bounds);

    void    (* foreach_key)    (EekSection  *self,
                                GFunc        func,
                                gpointer     user_data);
};

GType   eek_section_get_type       (void) G_GNUC_CONST;

void    eek_section_set_dimensions (EekSection  *section,
                                    gint         columns,
                                    gint         rows);
void    eek_section_get_dimensions (EekSection  *section,
                                    gint        *columns,
                                    gint        *rows);
void    eek_section_set_angle      (EekSection  *section,
                                    gint         angle);
gint    eek_section_get_angle      (EekSection  *section);

void    eek_section_set_bounds     (EekSection  *section,
                                    EekBounds   *bounds);
void    eek_section_get_bounds     (EekSection  *section,
                                    EekBounds   *bounds);

EekKey *eek_section_create_key     (EekSection  *section,
                                    const gchar *name,
                                    guint       *keysyms,
                                    gint         num_groups,
                                    gint         num_levels,
                                    gint         column,
                                    gint         row,
                                    EekOutline  *outline,
                                    EekBounds   *bounds);

void    eek_section_foreach_key    (EekSection  *section,
                                    GFunc        func,
                                    gpointer     user_data);

G_END_DECLS
#endif  /* EEK_SECTION_H */
