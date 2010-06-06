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
 * SECTION:eek-gtk-keyboard
 * @short_description: #EekKeyboard implemented as a #GtkWidget
 *
 * The #EekGtkKeyboard class implements the #EekKeyboardIface
 * interface as a #GtkWidget.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-gtk-keyboard.h"
#include "eek-gtk-private.h"
#include "eek-simple-keyboard.h"

enum {
    PROP_0,
    PROP_BOUNDS,
    PROP_LAST
};

static void eek_keyboard_iface_init (EekKeyboardIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekGtkKeyboard, eek_gtk_keyboard,
                         GTK_TYPE_VBOX,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_KEYBOARD,
                                                eek_keyboard_iface_init));

#define EEK_GTK_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_GTK_KEYBOARD, EekGtkKeyboardPrivate))


struct _EekGtkKeyboardPrivate
{
    EekSimpleKeyboard *simple;
    gint group;
    gint level;
};

static void
eek_gtk_keyboard_real_set_bounds (EekKeyboard *self,
                                      EekBounds *bounds)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_keyboard_set_bounds (EEK_KEYBOARD(priv->simple), bounds);
}

static void
eek_gtk_keyboard_real_get_bounds (EekKeyboard *self,
                                      EekBounds   *bounds)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    return eek_keyboard_get_bounds (EEK_KEYBOARD(priv->simple), bounds);
}

struct keysym_index {
    gint group;
    gint level;
};

static void
key_set_keysym_index (gpointer self, gpointer user_data)
{
    struct keysym_index *ki;

    g_return_if_fail (EEK_IS_KEY(self));
    eek_key_set_keysym_index (self, ki->group, ki->level);
}

static void
section_set_keysym_index (gpointer self, gpointer user_data)
{
    EekGtkCallbackData data;

    data.func = key_set_keysym_index;
    data.user_data = user_data;

    g_return_if_fail (EEK_IS_GTK_SECTION(self));
    gtk_container_foreach (GTK_CONTAINER(self), eek_gtk_callback, &data);
}

static void
eek_gtk_keyboard_real_set_keysym_index (EekKeyboard *self,
                                        gint         group,
                                        gint         level)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    EekGtkCallbackData data;
    struct keysym_index ki;

    g_return_if_fail (priv);
    priv->group = group;
    priv->level = level;

    ki.group = group;
    ki.level = level;

    data.func = section_set_keysym_index;
    data.user_data = &ki;

    gtk_container_foreach (GTK_CONTAINER(self), eek_gtk_callback, &data);
}

static void
eek_gtk_keyboard_real_get_keysym_index (EekKeyboard *self,
                                            gint        *group,
                                            gint        *level)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    *group = priv->group;
    *level = priv->level;
}

static EekSection *
eek_gtk_keyboard_real_create_section (EekKeyboard *self,
                                      const gchar *name,
                                      gint         angle,
                                      EekBounds   *bounds)
{
    EekSection *section;

    g_return_if_fail (EEK_IS_GTK_KEYBOARD(self));

    section = g_object_new (EEK_TYPE_GTK_SECTION,
                            "name", name,
                            "angle", angle,
                            "bounds", bounds,
                            NULL);

    gtk_box_pack_start (GTK_BOX(self), GTK_WIDGET(section), FALSE, FALSE, 0);
    return section;
}

static void
eek_gtk_keyboard_real_foreach_section (EekKeyboard *self,
                                       GFunc        func,
                                       gpointer     user_data)
{
    EekGtkCallbackData data;

    g_return_if_fail (EEK_IS_GTK_KEYBOARD(self));

    data.func = func;
    data.user_data = user_data;

    gtk_container_foreach (GTK_CONTAINER(self),
                           eek_gtk_callback,
                           &data);
}

static void
eek_gtk_keyboard_real_set_layout (EekKeyboard *self,
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
    iface->set_bounds = eek_gtk_keyboard_real_set_bounds;
    iface->get_bounds = eek_gtk_keyboard_real_get_bounds;
    iface->set_keysym_index = eek_gtk_keyboard_real_set_keysym_index;
    iface->get_keysym_index = eek_gtk_keyboard_real_get_keysym_index;
    iface->create_section = eek_gtk_keyboard_real_create_section;
    iface->foreach_section = eek_gtk_keyboard_real_foreach_section;
    iface->set_layout = eek_gtk_keyboard_real_set_layout;
}

static void
eek_gtk_keyboard_dispose (GObject *object)
{
    G_OBJECT_CLASS (eek_gtk_keyboard_parent_class)->dispose (object);
}

static void
eek_gtk_keyboard_finalize (GObject *object)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(object);

    g_object_unref (priv->simple);
    G_OBJECT_CLASS (eek_gtk_keyboard_parent_class)->finalize (object);
}

static void
eek_gtk_keyboard_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(object);

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_BOUNDS:
        eek_keyboard_set_bounds (EEK_KEYBOARD(object),
                                 g_value_get_boxed (value));
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_gtk_keyboard_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
    EekGtkKeyboardPrivate *priv = EEK_GTK_KEYBOARD_GET_PRIVATE(object);

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_BOUNDS:
        eek_keyboard_set_bounds (EEK_KEYBOARD(object),
                                 g_value_get_boxed (value));
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_gtk_keyboard_class_init (EekGtkKeyboardClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekGtkKeyboardPrivate));

    gobject_class->set_property = eek_gtk_keyboard_set_property;
    gobject_class->get_property = eek_gtk_keyboard_get_property;
    gobject_class->finalize     = eek_gtk_keyboard_finalize;
    gobject_class->dispose      = eek_gtk_keyboard_dispose;

    g_object_class_override_property (gobject_class,
                                      PROP_BOUNDS,
                                      "bounds");
}

static void
eek_gtk_keyboard_init (EekGtkKeyboard *self)
{
    EekGtkKeyboardPrivate *priv;

    priv = self->priv = EEK_GTK_KEYBOARD_GET_PRIVATE(self);
    priv->simple = g_object_new (EEK_TYPE_SIMPLE_KEYBOARD, NULL);
    priv->group = priv->level = 0;
}

/**
 * eek_gtk_keyboard_new:
 *
 * Create a new #EekGtkKeyboard.
 */
EekKeyboard*
eek_gtk_keyboard_new (void)
{
    return g_object_new (EEK_TYPE_GTK_KEYBOARD, NULL);
}
