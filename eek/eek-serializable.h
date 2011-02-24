/* 
 * Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2011 Red Hat, Inc.
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

#ifndef EEK_SERIALIZABLE_H
#define EEK_SERIALIZABLE_H 1

#include <glib-object.h>

G_BEGIN_DECLS

#define EEK_TYPE_SERIALIZABLE (eek_serializable_get_type())
#define EEK_SERIALIZABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_SERIALIZABLE, EekSerializable))
#define EEK_IS_SERIALIZABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_SERIALIZABLE))
#define EEK_SERIALIZABLE_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), EEK_TYPE_SERIALIZABLE, EekSerializableIface))

typedef struct _EekSerializable EekSerializable;
typedef struct _EekSerializableIface EekSerializableIface;

/**
 * EekSerializableIface:
 *
 * @serialize: virtual function for serializing object into #GVariant
 * @deserialize: virtual function for deserializing object from #GVariant
 */
struct _EekSerializableIface
{
    /*< private >*/
    GTypeInterface parent_iface;

    void  (* serialize)   (EekSerializable       *object,
                           GVariantBuilder       *builder);
    gsize (* deserialize) (EekSerializable       *object,
                           GVariant              *variant,
                           gsize                  index);

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType            eek_serializable_get_type    (void);

GVariant        *eek_serializable_serialize   (EekSerializable *object);
EekSerializable *eek_serializable_deserialize (GVariant        *variant);

G_END_DECLS
#endif  /* EEK_SERIALIZABLE_H */
