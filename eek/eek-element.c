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

/**
 * SECTION:eek-element
 * @short_description: Base class of a keyboard element
 *
 * The #EekElementClass class represents a keyboard element, which
 * shall be used to implement #EekKeyboard, #EekSection, or #EekKey.
 */

#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-element.h"

enum {
    PROP_0,
    PROP_NAME,
    PROP_BOUNDS,
    PROP_LAST
};

G_DEFINE_ABSTRACT_TYPE (EekElement, eek_element, G_TYPE_INITIALLY_UNOWNED);

#define EEK_ELEMENT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_ELEMENT, EekElementPrivate))


struct _EekElementPrivate
{
    gchar *name;
    EekBounds bounds;
};

static void
eek_element_real_set_name (EekElement *self,
                                  const gchar *name)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    priv->name = g_strdup (name);

    g_object_notify (G_OBJECT(self), "name");
}

static G_CONST_RETURN gchar *
eek_element_real_get_name (EekElement *self)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    return priv->name;
}

static void
eek_element_real_set_bounds (EekElement *self,
                             EekBounds *bounds)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    priv->bounds = *bounds;
}

static void
eek_element_real_get_bounds (EekElement *self,
                             EekBounds  *bounds)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    g_return_if_fail (bounds);
    *bounds = priv->bounds;

    g_object_notify (G_OBJECT(self), "bounds");
}

static void
eek_element_finalize (GObject *object)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(object);

    g_free (priv->name);
}

static void
eek_element_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    g_return_if_fail (EEK_IS_ELEMENT(object));
    switch (prop_id) {
    case PROP_NAME:
        eek_element_set_name (EEK_ELEMENT(object),
                              g_value_get_string (value));
        break;
    case PROP_BOUNDS:
        eek_element_set_bounds (EEK_ELEMENT(object),
                                g_value_get_boxed (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
eek_element_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    EekBounds  bounds;

    g_return_if_fail (EEK_IS_ELEMENT(object));
    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, eek_element_get_name (EEK_ELEMENT(object)));
        break;
    case PROP_BOUNDS:
        eek_element_get_bounds (EEK_ELEMENT(object), &bounds);
        g_value_set_boxed (value, &bounds);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
eek_element_class_init (EekElementClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekElementPrivate));

    klass->set_name = eek_element_real_set_name;
    klass->get_name = eek_element_real_get_name;
    klass->set_bounds = eek_element_real_set_bounds;
    klass->get_bounds = eek_element_real_get_bounds;

    gobject_class->set_property = eek_element_set_property;
    gobject_class->get_property = eek_element_get_property;
    gobject_class->finalize     = eek_element_finalize;

    /**
     * EekElement:name:
     *
     * The name of #EekElement.
     */
    pspec = g_param_spec_string ("name",
                                 "Name",
                                 "Name",
                                 NULL,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_NAME,
                                     pspec);

    /**
     * EekElement:bounds:
     *
     * The bounding box of #EekElement.
     */
    pspec = g_param_spec_boxed ("bounds",
                                "Bounds",
                                "Bounding box of the element",
                                EEK_TYPE_BOUNDS,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_BOUNDS,
                                     pspec);
}

static void
eek_element_init (EekElement *self)
{
    EekElementPrivate *priv;

    priv = self->priv = EEK_ELEMENT_GET_PRIVATE(self);
    priv->name = NULL;
    memset (&priv->bounds, 0, sizeof priv->bounds);
}

void
eek_element_set_name (EekElement  *element,
                      const gchar *name)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    EEK_ELEMENT_GET_CLASS(element)->set_name (element, name);
}

G_CONST_RETURN gchar *
eek_element_get_name (EekElement  *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT(element), NULL);
    return EEK_ELEMENT_GET_CLASS(element)->get_name (element);
}

void
eek_element_set_bounds (EekElement  *element,
                        EekBounds   *bounds)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    EEK_ELEMENT_GET_CLASS(element)->set_bounds (element, bounds);
}

void
eek_element_get_bounds (EekElement  *element,
                        EekBounds   *bounds)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    EEK_ELEMENT_GET_CLASS(element)->get_bounds (element, bounds);
}
