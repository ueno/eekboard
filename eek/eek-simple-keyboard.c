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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-simple-keyboard.h"

enum {
    PROP_0,
    PROP_BOUNDS,
    PROP_LAST
};

static void eek_keyboard_iface_init (EekKeyboardIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekSimpleKeyboard, eek_simple_keyboard,
                         G_TYPE_INITIALLY_UNOWNED,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_KEYBOARD,
                                                eek_keyboard_iface_init));

#define EEK_SIMPLE_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_SIMPLE_KEYBOARD, EekSimpleKeyboardPrivate))


struct _EekSimpleKeyboardPrivate
{
    EekBounds bounds;
    GSList *sections;
};

static void
eek_simple_keyboard_real_set_bounds (EekKeyboard *self,
                                     EekBounds *bounds)
{
    EekSimpleKeyboardPrivate *priv = EEK_SIMPLE_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    priv->bounds = *bounds;
}

static void
eek_simple_keyboard_real_get_bounds (EekKeyboard *self,
                                     EekBounds   *bounds)
{
    EekSimpleKeyboardPrivate *priv = EEK_SIMPLE_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_return_if_fail (bounds);
    *bounds = priv->bounds;
}

static EekSection *
eek_simple_keyboard_real_create_section (EekKeyboard *self,
                                          const gchar *name,
                                          gint         angle,
                                          EekBounds   *bounds)
{
    EekSimpleKeyboardPrivate *priv = EEK_SIMPLE_KEYBOARD_GET_PRIVATE(self);
    EekSection *section;

    g_return_val_if_fail (priv, NULL);
    section = g_object_new (EEK_TYPE_SIMPLE_SECTION,
                            "name", name,
                            "angle", angle,
                            "bounds", bounds,
                            NULL);
    g_return_val_if_fail (section, NULL);
    priv->sections = g_slist_prepend (priv->sections, section);
    return section;
}

static void
eek_simple_keyboard_real_foreach_section (EekKeyboard *self,
                                          GFunc        func,
                                          gpointer     user_data)
{
    EekSimpleKeyboardPrivate *priv = EEK_SIMPLE_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_slist_foreach (priv->sections, func, user_data);
}

static void
eek_simple_keyboard_real_set_layout (EekKeyboard *self,
                                     EekLayout *layout)
{
    g_return_if_fail (EEK_IS_KEYBOARD(self));
    g_return_if_fail (EEK_IS_LAYOUT(layout));

    EEK_LAYOUT_GET_CLASS(layout)->apply_to_keyboard (layout, self);
    if (g_object_is_floating (layout))
        g_object_unref (layout);
}

static void
eek_keyboard_iface_init (EekKeyboardIface *iface)
{
    iface->set_bounds = eek_simple_keyboard_real_set_bounds;
    iface->get_bounds = eek_simple_keyboard_real_get_bounds;
    iface->create_section = eek_simple_keyboard_real_create_section;
    iface->foreach_section = eek_simple_keyboard_real_foreach_section;
    iface->set_layout = eek_simple_keyboard_real_set_layout;
}

static void
eek_simple_keyboard_dispose (GObject *object)
{
    EekSimpleKeyboardPrivate *priv = EEK_SIMPLE_KEYBOARD_GET_PRIVATE(object);
    GSList *head;

    for (head = priv->sections; head; head = g_slist_next (head))
        g_object_unref (head->data);
    G_OBJECT_CLASS (eek_simple_keyboard_parent_class)->dispose (object);
}

static void
eek_simple_keyboard_finalize (GObject *object)
{
    EekSimpleKeyboardPrivate *priv = EEK_SIMPLE_KEYBOARD_GET_PRIVATE(object);

    g_slist_free (priv->sections);
    G_OBJECT_CLASS (eek_simple_keyboard_parent_class)->finalize (object);
}

static void
eek_simple_keyboard_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    switch (prop_id) {
    case PROP_BOUNDS:
        eek_keyboard_set_bounds (EEK_KEYBOARD(object),
                                 g_value_get_boxed (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_simple_keyboard_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    EekSimpleKeyboardPrivate *priv = EEK_SIMPLE_KEYBOARD_GET_PRIVATE(object);

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_BOUNDS:
        g_value_set_boxed (value, &priv->bounds);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_simple_keyboard_class_init (EekSimpleKeyboardClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekSimpleKeyboardPrivate));

    gobject_class->set_property = eek_simple_keyboard_set_property;
    gobject_class->get_property = eek_simple_keyboard_get_property;
    gobject_class->finalize     = eek_simple_keyboard_finalize;
    gobject_class->dispose      = eek_simple_keyboard_dispose;

    g_object_class_override_property (gobject_class,
                                      PROP_BOUNDS,
                                      "bounds");
}

static void
eek_simple_keyboard_init (EekSimpleKeyboard *self)
{
    EekSimpleKeyboardPrivate *priv;

    priv = self->priv = EEK_SIMPLE_KEYBOARD_GET_PRIVATE(self);
    priv->sections = NULL;
}

EekKeyboard*
eek_simple_keyboard_new (void)
{
    return g_object_new (EEK_TYPE_SIMPLE_KEYBOARD, NULL);
}
