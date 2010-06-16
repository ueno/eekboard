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
#ifndef EEK_ELEMENT_H
#define EEK_ELEMENT_H 1

#include <glib-object.h>
#include "eek-types.h"

G_BEGIN_DECLS
#define EEK_TYPE_ELEMENT (eek_element_get_type())
#define EEK_ELEMENT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_ELEMENT, EekElement))
#define EEK_ELEMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_ELEMENT, EekElementClass))
#define EEK_IS_ELEMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_ELEMENT))
#define EEK_IS_ELEMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_ELEMENT))
#define EEK_ELEMENT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_ELEMENT, EekElementClass))

typedef struct _EekElementClass   EekElementClass;
typedef struct _EekElementPrivate EekElementPrivate;

struct _EekElement
{
    /*< private >*/
    GInitiallyUnowned parent;

    EekElementPrivate *priv;
};

struct _EekElementClass
{
    /*< private >*/
    GInitiallyUnownedClass parent_class;

    void                  (* set_name)   (EekElement   *self,
                                          const gchar  *name);

    G_CONST_RETURN gchar *(* get_name)   (EekElement   *self);

    void                  (* set_bounds) (EekElement   *self,
                                          EekBounds    *bounds);

    void                  (* get_bounds) (EekElement   *self,
                                          EekBounds    *bounds);
};

GType                 eek_element_get_type   (void) G_GNUC_CONST;

void                  eek_element_set_name   (EekElement  *element,
                                              const gchar *name);

G_CONST_RETURN gchar *eek_element_get_name   (EekElement  *element);

void                  eek_element_set_bounds (EekElement  *element,
                                              EekBounds   *bounds);

void                  eek_element_get_bounds (EekElement  *element,
                                              EekBounds   *bounds);

G_END_DECLS
#endif  /* EEK_ELEMENT_H */
