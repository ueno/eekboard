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
 * SECTION:eek-clutter-keyboard
 * @short_description: #EekKeyboard implemented as a #ClutterActor
 *
 * The #EekClutterKeyboard class implements the #EekKeyboardIface
 * interface as a #ClutterActor.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-clutter-keyboard.h"
#include "eek-clutter-private.h"
#include "eek-simple-keyboard.h"

enum {
    PROP_0,
    PROP_BOUNDS,
    PROP_LAST
};

static void eek_keyboard_iface_init (EekKeyboardIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekClutterKeyboard, eek_clutter_keyboard,
                         CLUTTER_TYPE_GROUP,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_KEYBOARD,
                                                eek_keyboard_iface_init));

#define EEK_CLUTTER_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_KEYBOARD, EekClutterKeyboardPrivate))


struct _EekClutterKeyboardPrivate
{
    EekSimpleKeyboard *simple;
    gint group;
    gint level;
};

static void
eek_clutter_keyboard_real_set_bounds (EekKeyboard *self,
                                      EekBounds *bounds)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_keyboard_set_bounds (EEK_KEYBOARD(priv->simple), bounds);
    clutter_actor_set_position (CLUTTER_ACTOR(self), bounds->x, bounds->y);
    clutter_actor_set_size (CLUTTER_ACTOR(self), bounds->width, bounds->height);
}

static void
eek_clutter_keyboard_real_get_bounds (EekKeyboard *self,
                                      EekBounds   *bounds)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    return eek_keyboard_get_bounds (EEK_KEYBOARD(priv->simple), bounds);
}

static void
eek_clutter_keyboard_real_set_keysym_index (EekKeyboard *self,
                                            gint         group,
                                            gint         level)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);
    gint num_sections, num_keys, i, j;

    g_return_if_fail (priv);
    priv->group = group;
    priv->level = level;
    num_sections = clutter_group_get_n_children (CLUTTER_GROUP(self));
    for (i = 0; i < num_sections; i++) {
        ClutterActor *section;

        section = clutter_group_get_nth_child (CLUTTER_GROUP(self), i);
        g_return_if_fail (EEK_IS_CLUTTER_SECTION(section));
        num_keys = clutter_group_get_n_children (CLUTTER_GROUP(section));
        for (j = 0; j < num_keys; j++) {
            ClutterActor *key;

            key = clutter_group_get_nth_child (CLUTTER_GROUP(section), j);
            g_return_if_fail (EEK_IS_CLUTTER_KEY(key));
            eek_key_set_keysym_index (EEK_KEY(key), group, level);
        }
    }
}

static void
eek_clutter_keyboard_real_get_keysym_index (EekKeyboard *self,
                                            gint        *group,
                                            gint        *level)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv);
    *group = priv->group;
    *level = priv->level;
}

static EekSection *
eek_clutter_keyboard_real_create_section (EekKeyboard *self,
                                          const gchar *name,
                                          gint         angle,
                                          EekBounds   *bounds)
{
    EekSection *section;

    g_return_if_fail (EEK_IS_CLUTTER_KEYBOARD(self));

    section = g_object_new (EEK_TYPE_CLUTTER_SECTION,
                            "name", name,
                            "angle", angle,
                            "bounds", bounds,
                            NULL);

    clutter_container_add_actor (CLUTTER_CONTAINER(self),
                                 CLUTTER_ACTOR(section));

    return section;
}

static void
eek_clutter_keyboard_real_foreach_section (EekKeyboard *self,
                                           GFunc        func,
                                           gpointer     user_data)
{
    EekClutterCallbackData data;

    g_return_if_fail (EEK_IS_CLUTTER_KEYBOARD(self));

    data.func = func;
    data.user_data = user_data;

    clutter_container_foreach (CLUTTER_CONTAINER(self),
                               eek_clutter_callback,
                               &data);
}

static void
eek_clutter_keyboard_real_set_layout (EekKeyboard *self,
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
    iface->set_bounds = eek_clutter_keyboard_real_set_bounds;
    iface->get_bounds = eek_clutter_keyboard_real_get_bounds;
    iface->set_keysym_index = eek_clutter_keyboard_real_set_keysym_index;
    iface->get_keysym_index = eek_clutter_keyboard_real_get_keysym_index;
    iface->create_section = eek_clutter_keyboard_real_create_section;
    iface->foreach_section = eek_clutter_keyboard_real_foreach_section;
    iface->set_layout = eek_clutter_keyboard_real_set_layout;
}

static void
eek_clutter_keyboard_dispose (GObject *object)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(object);

    if (priv->simple) {
        g_object_unref (priv->simple);
        priv->simple = NULL;
    }
    clutter_group_remove_all (CLUTTER_GROUP(object));
    G_OBJECT_CLASS (eek_clutter_keyboard_parent_class)->dispose (object);
}

static void
eek_clutter_keyboard_finalize (GObject *object)
{
    G_OBJECT_CLASS (eek_clutter_keyboard_parent_class)->finalize (object);
}

static void
eek_clutter_keyboard_set_property (GObject *object,
                                   guint prop_id,
                                   const GValue *value,
                                   GParamSpec *pspec)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(object);

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
eek_clutter_keyboard_get_property (GObject *object,
                                   guint prop_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(object);

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
eek_clutter_keyboard_paint (ClutterActor *self)
{
    CLUTTER_ACTOR_CLASS (eek_clutter_keyboard_parent_class)->paint (self);
}

static void
eek_clutter_keyboard_class_init (EekClutterKeyboardClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyboardPrivate));

    gobject_class->set_property = eek_clutter_keyboard_set_property;
    gobject_class->get_property = eek_clutter_keyboard_get_property;
    gobject_class->finalize     = eek_clutter_keyboard_finalize;
    gobject_class->dispose      = eek_clutter_keyboard_dispose;

    actor_class->paint = eek_clutter_keyboard_paint;

    g_object_class_override_property (gobject_class,
                                      PROP_BOUNDS,
                                      "bounds");
}

static gboolean
on_event (ClutterActor *actor,
	  ClutterEvent *event,
	  gpointer      user_data)
{
    EekKeyboard *keyboard;
    EekKey *key;
    ClutterActor *source;

    g_return_val_if_fail (EEK_IS_KEYBOARD(user_data), FALSE);
    keyboard = EEK_KEYBOARD(user_data);

    source = clutter_event_get_source (event);
    if (!EEK_IS_KEY(source))
        return FALSE;
    key = EEK_KEY(source);
    if (event->type == CLUTTER_BUTTON_PRESS) {
        guint keysym;
        gint group, level;

        keysym = eek_key_get_keysym (EEK_KEY(source));
        if (keysym == 0xFFE1 || keysym == 0xFFE2) {
            eek_keyboard_get_keysym_index (keyboard, &group, &level);
            if (level == 0)
                eek_keyboard_set_keysym_index (keyboard, group, 1);
            else
                eek_keyboard_set_keysym_index (keyboard, group, 0);
        }
    }
    return FALSE;
}

static void
eek_clutter_keyboard_init (EekClutterKeyboard *self)
{
    EekClutterKeyboardPrivate *priv;

    priv = self->priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);
    priv->simple = g_object_new (EEK_TYPE_SIMPLE_KEYBOARD, NULL);
    priv->group = priv->level = 0;

    clutter_actor_set_reactive (CLUTTER_ACTOR(self), TRUE);
    g_signal_connect (self, "event", G_CALLBACK(on_event), self);
}

/**
 * eek_clutter_keyboard_new:
 * @width: max width of the area where the keyboard to be drawn
 * @height: max height of the area where the keyboard to be drawn
 *
 * Create a new #EekClutterKeyboard.
 */
EekKeyboard*
eek_clutter_keyboard_new (gfloat width,
                          gfloat height)
{
    EekBounds bounds;

    bounds.x = bounds.y = 0;
    bounds.width = width;
    bounds.height = height;
    return g_object_new (EEK_TYPE_CLUTTER_KEYBOARD, "bounds", &bounds, NULL);
}
