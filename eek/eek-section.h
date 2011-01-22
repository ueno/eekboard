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

#include <glib-object.h>
#include "eek-container.h"
#include "eek-types.h"

G_BEGIN_DECLS

#define EEK_TYPE_SECTION (eek_section_get_type())
#define EEK_SECTION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_SECTION, EekSection))
#define EEK_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_SECTION, EekSectionClass))
#define EEK_IS_SECTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_SECTION))
#define EEK_IS_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_SECTION))
#define EEK_SECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_SECTION, EekSectionClass))

typedef struct _EekSectionClass EekSectionClass;
typedef struct _EekSectionPrivate EekSectionPrivate;

struct _EekSection
{
    /*< private >*/
    EekContainer parent;

    EekSectionPrivate *priv;
};

/**
 * EekSectionClass:
 * @set_angle: virtual function for setting rotation angle of the section
 * @get_angle: virtual function for getting rotation angle of the section
 * @get_n_rows: virtual function for getting the number of rows in the section
 * @add_row: virtual function for adding a new row to the section
 * @get_row: virtual function for accessing a row in the section
 * @create_key: virtual function for creating key in the section
 * @find_key_by_keycode: virtual function for accessing a key in the
 * section by keycode
 */
struct _EekSectionClass
{
    /*< private >*/
    EekContainerClass parent_class;

    /*< public >*/
    void    (* set_angle)           (EekSection     *self,
                                     gint            angle);
    gint    (* get_angle)           (EekSection     *self);

    gint    (* get_n_rows)          (EekSection     *self);
    void    (* add_row)             (EekSection     *self,
                                     gint            num_columns,
                                     EekOrientation  orientation);
    void    (* get_row)             (EekSection     *self,
                                     gint            index,
                                     gint           *num_columns,
                                     EekOrientation *orientation);

    EekKey *(* create_key)          (EekSection     *self,
                                     gint            row,
                                     gint            column);

    EekKey *(* find_key_by_keycode) (EekSection     *self,
                                     guint           keycode);

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType   eek_section_get_type             (void) G_GNUC_CONST;

void    eek_section_set_angle            (EekSection     *section,
                                          gint            angle);
gint    eek_section_get_angle            (EekSection     *section);

gint    eek_section_get_n_rows           (EekSection     *section);
void    eek_section_add_row              (EekSection     *section,
                                          gint            num_columns,
                                          EekOrientation  orientation);
void    eek_section_get_row              (EekSection     *section,
                                          gint            index,
                                          gint           *num_columns,
                                          EekOrientation *orientation);

EekKey *eek_section_create_key           (EekSection     *section,
                                          gint            column,
                                          gint            row);

EekKey *eek_section_find_key_by_keycode  (EekSection     *section,
                                          guint           keycode);

G_END_DECLS
#endif  /* EEK_SECTION_H */
