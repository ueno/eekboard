/*
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:eek-serializable
 * @short_description: Interface which provides object serialization
 * into #GVariant
 *
 * The #EekSerializableIface interface defines serialize/deserialize
 * method.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-serializable.h"

GType
eek_serializable_get_type (void)
{
    static GType iface_type = 0;
    if (iface_type == 0) {
        static const GTypeInfo info = {
            sizeof (EekSerializableIface),
            NULL,
            NULL,
        };
        iface_type = g_type_register_static (G_TYPE_INTERFACE,
                                             "EekSerializable",
                                             &info, 0);
    }
    return iface_type;
}

GVariant *
eek_serializable_serialize (EekSerializable *object)
{
    GVariantBuilder builder;

    g_return_val_if_fail (EEK_IS_SERIALIZABLE (object), FALSE);

    g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add (&builder, "s", g_type_name (G_OBJECT_TYPE (object)));
    EEK_SERIALIZABLE_GET_IFACE (object)->serialize (object, &builder);

    return g_variant_builder_end (&builder);
}

EekSerializable *
eek_serializable_deserialize (GVariant *variant)
{
    gchar *type_name = NULL;
    GType type;
    EekSerializable *object;
    gsize index = 0;

    g_return_val_if_fail (variant != NULL, NULL);

    g_variant_get_child (variant, index++, "&s", &type_name);
    type = g_type_from_name (type_name);

    g_return_val_if_fail (g_type_is_a (type, EEK_TYPE_SERIALIZABLE), NULL);

    object = g_object_new (type, NULL);

    index = EEK_SERIALIZABLE_GET_IFACE (object)->deserialize (object,
                                                              variant,
                                                              index);
    if (index < 0) {
        g_object_unref (object);
        g_return_val_if_reached (NULL);
    }

    return object;
}
