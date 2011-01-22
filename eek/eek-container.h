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
#ifndef EEK_CONTAINER_H
#define EEK_CONTAINER_H 1

#include "eek-element.h"

G_BEGIN_DECLS

#define EEK_TYPE_CONTAINER (eek_container_get_type())
#define EEK_CONTAINER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_CONTAINER, EekContainer))
#define EEK_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_CONTAINER, EekContainerClass))
#define EEK_IS_CONTAINER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_CONTAINER))
#define EEK_IS_CONTAINER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_CONTAINER))
#define EEK_CONTAINER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_CONTAINER, EekContainerClass))

typedef struct _EekContainerClass EekContainerClass;
typedef struct _EekContainerPrivate EekContainerPrivate;

typedef void (*EekCallback) (EekElement *element, gpointer user_data);
typedef gint (*EekCompareFunc) (EekElement *element, gpointer user_data);

struct _EekContainer
{
    /*< private >*/
    EekElement parent;

    EekContainerPrivate *priv;
};

/**
 * EekContainerClass:
 * @foreach_child: virtual function for iterating over the container's children
 * @find: virtual function for looking up a child
 * @child_added: class handler for #EekContainer::child-added
 * @child_removed: class handler for #EekContainer::child-added
 */
struct _EekContainerClass
{
    /*< private >*/
    EekElementClass parent_class;

    void        (* add_child)      (EekContainer      *self,
                                    EekElement        *element);

    void        (* remove_child)   (EekContainer      *self,
                                    EekElement        *element);

    /*< public >*/
    void        (* foreach_child)  (EekContainer      *self,
                                    EekCallback        callback,
                                    gpointer           user_data);
    EekElement *(* find)           (EekContainer      *self,
                                    EekCompareFunc     func,
                                    gpointer           data);

    /* signals */
    void        (* child_added)    (EekContainer      *self,
                                    EekElement        *element);
    void        (* child_removed)  (EekContainer      *self,
                                    EekElement        *element);
    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType       eek_container_get_type       (void) G_GNUC_CONST;

void        eek_container_foreach_child  (EekContainer      *container,
                                          EekCallback        callback,
                                          gpointer           user_data);
EekElement *eek_container_find           (EekContainer      *container,
                                          EekCompareFunc     func,
                                          gpointer           data);

G_END_DECLS
#endif  /* EEK_CONTAINER_H */
