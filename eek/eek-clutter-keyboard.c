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
 * @short_description: #EekKeyboard embedding a #ClutterActor
 */
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-clutter-keyboard.h"
#include "eek-keyboard.h"

G_DEFINE_TYPE (EekClutterKeyboard, eek_clutter_keyboard, EEK_TYPE_KEYBOARD);

#define EEK_CLUTTER_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_KEYBOARD, EekClutterKeyboardPrivate))


struct _EekClutterKeyboardPrivate
{
    ClutterActor *actor;
    EekLayout *layout;
};

static void
eek_clutter_keyboard_real_set_name (EekElement *self,
                                    const gchar *name)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    EEK_ELEMENT_CLASS (eek_clutter_keyboard_parent_class)->
        set_name (self, name);

    g_return_if_fail (priv->actor);

    clutter_actor_set_name (priv->actor, name);
}

static void
eek_clutter_keyboard_real_set_bounds (EekElement *self,
                                     EekBounds  *bounds)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    EEK_ELEMENT_CLASS (eek_clutter_keyboard_parent_class)->
        set_bounds (self, bounds);

    g_return_if_fail (priv->actor);

    clutter_actor_set_position (priv->actor, bounds->x, bounds->y);
    clutter_actor_set_size (priv->actor, bounds->width, bounds->height);
}

static void
key_pressed_event (EekSection  *section,
                   EekKey      *key,
                   EekKeyboard *keyboard)
{
    g_signal_emit_by_name (keyboard, "key-pressed", key);
}

static void
key_released_event (EekSection  *section,
                    EekKey      *key,
                    EekKeyboard *keyboard)
{
    g_signal_emit_by_name (keyboard, "key-released", key);
}

static EekSection *
eek_clutter_keyboard_real_create_section (EekKeyboard *self)
{
    EekSection *section;
    ClutterActor *actor;

    section = g_object_new (EEK_TYPE_CLUTTER_SECTION, NULL);
    g_return_val_if_fail (section, NULL);
    g_object_ref_sink (section);

    g_signal_connect (section, "key-pressed",
                      G_CALLBACK(key_pressed_event), self);
    g_signal_connect (section, "key-released",
                      G_CALLBACK(key_released_event), self);

    EEK_CONTAINER_GET_CLASS(self)->add_child (EEK_CONTAINER(self),
                                              EEK_ELEMENT(section));

    actor = eek_clutter_keyboard_get_actor (EEK_CLUTTER_KEYBOARD(self));
    clutter_container_add_actor
        (CLUTTER_CONTAINER(actor),
         eek_clutter_section_get_actor (EEK_CLUTTER_SECTION(section)));

    return section;
}

static void
eek_clutter_keyboard_real_set_layout (EekKeyboard *self,
                                      EekLayout   *layout)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (EEK_IS_LAYOUT(layout));

    /* Don't apply the layout to keyboard right now, so to delay
       drawing until eek_clutter_keyboard_get_actor. */
    priv->layout = layout;
    g_object_ref_sink (priv->layout);
}

static void
eek_clutter_keyboard_finalize (GObject *object)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(object);

    /* No need for clutter_group_remove_all() since
       ClutterGroup#dispose() unrefs all the children. */
    if (priv->actor)
        g_object_unref (priv->actor);
    if (priv->layout)
        g_object_unref (priv->layout);
    G_OBJECT_CLASS (eek_clutter_keyboard_parent_class)->finalize (object);
}

static void
eek_clutter_keyboard_class_init (EekClutterKeyboardClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    EekElementClass *element_class = EEK_ELEMENT_CLASS (klass);
    EekKeyboardClass *keyboard_class = EEK_KEYBOARD_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyboardPrivate));

    keyboard_class->create_section = eek_clutter_keyboard_real_create_section;
    element_class->set_name = eek_clutter_keyboard_real_set_name;
    element_class->set_bounds = eek_clutter_keyboard_real_set_bounds;
    gobject_class->finalize = eek_clutter_keyboard_finalize;
}

static void
eek_clutter_keyboard_init (EekClutterKeyboard *self)
{
    EekClutterKeyboardPrivate *priv;

    priv = self->priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);
    priv->actor = NULL;
    priv->layout = NULL;
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
    EekKeyboard *keyboard;
    EekBounds bounds;

    keyboard = g_object_new (EEK_TYPE_CLUTTER_KEYBOARD, NULL);
    g_return_val_if_fail (keyboard, NULL);

    /* Can't call set_bounds of this class since it needs priv->actor
       initialized */
    memset (&bounds, 0, sizeof bounds);
    bounds.width = width;
    bounds.height = height;
    EEK_ELEMENT_CLASS (eek_clutter_keyboard_parent_class)->
        set_bounds (EEK_ELEMENT(keyboard), &bounds);
    return keyboard;
}

ClutterActor *
eek_clutter_keyboard_get_actor (EekClutterKeyboard *keyboard)
{
    EekClutterKeyboardPrivate *priv =
        EEK_CLUTTER_KEYBOARD_GET_PRIVATE(keyboard);
    if (!priv->actor)
        priv->actor = clutter_group_new ();
    if (priv->layout)
        eek_layout_apply (priv->layout, EEK_KEYBOARD(keyboard));
    return priv->actor;
}
