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

#include "eek-clutter-key.h"

G_DEFINE_TYPE (EekClutterKey, eek_clutter_key, CLUTTER_TYPE_ACTOR);

#define EEK_CLUTTER_KEY_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_KEY, EekClutterKeyPrivate))

struct _EekClutterKeyPrivate
{
    EekKey *key;
    EekClutterRenderer *renderer;
};

static void
eek_clutter_key_dispose (GObject *object)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(object);

    if (priv->renderer) {
        g_object_unref (priv->renderer);
        priv->renderer = NULL;
    }

    if (priv->key && g_object_is_floating (priv->key)) {
        g_object_unref (priv->key);
        priv->key = NULL;
    }

    G_OBJECT_CLASS (eek_clutter_key_parent_class)->dispose (object);
}

static void
eek_clutter_key_real_paint (ClutterActor *self)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    eek_clutter_renderer_render_key (priv->renderer, self, priv->key);
}

static void
eek_clutter_key_real_get_preferred_width (ClutterActor *self,
                                          gfloat        for_height,
                                          gfloat       *min_width_p,
                                          gfloat       *natural_width_p)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);
    EekBounds bounds;
    gdouble scale;

    scale = eek_renderer_get_scale (EEK_RENDERER(priv->renderer));
    eek_element_get_bounds (EEK_ELEMENT(priv->key), &bounds);
    *min_width_p = 0.0f;
    *natural_width_p = bounds.width * scale;
}

static void
eek_clutter_key_real_get_preferred_height (ClutterActor *self,
                                           gfloat        for_width,
                                           gfloat       *min_height_p,
                                           gfloat       *natural_height_p)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);
    EekBounds bounds;
    gdouble scale;

    scale = eek_renderer_get_scale (EEK_RENDERER(priv->renderer));
    eek_element_get_bounds (EEK_ELEMENT(priv->key), &bounds);
    *min_height_p = 0.0f;
    *natural_height_p = bounds.height * scale;
}

static void
eek_clutter_key_real_allocate (ClutterActor          *self,
                               const ClutterActorBox *box,
                               ClutterAllocationFlags flags)
{
    CLUTTER_ACTOR_CLASS (eek_clutter_key_parent_class)->
        allocate (self, box, flags);
}

static gboolean
eek_clutter_key_real_button_press_event (ClutterActor       *self,
                                         ClutterButtonEvent *event)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_signal_emit_by_name (priv->key, "pressed");

    return TRUE;
}

static gboolean
eek_clutter_key_real_button_release_event (ClutterActor       *self,
                                           ClutterButtonEvent *event)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_signal_emit_by_name (priv->key, "released");

    return TRUE;
}

static gboolean
eek_clutter_key_real_leave_event (ClutterActor         *self,
                                  ClutterCrossingEvent *event)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    if (eek_key_is_pressed (priv->key))
        g_signal_emit_by_name (priv->key, "released");

    return TRUE;
}

static void
eek_clutter_key_class_init (EekClutterKeyClass *klass)
{
    ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyPrivate));

    actor_class->paint = eek_clutter_key_real_paint;
    actor_class->get_preferred_width =
        eek_clutter_key_real_get_preferred_width;
    actor_class->get_preferred_height =
        eek_clutter_key_real_get_preferred_height;
    actor_class->allocate = eek_clutter_key_real_allocate;

    /* signals */
    actor_class->button_press_event =
        eek_clutter_key_real_button_press_event;
    actor_class->button_release_event =
        eek_clutter_key_real_button_release_event;
    actor_class->leave_event =
        eek_clutter_key_real_leave_event;

    gobject_class->dispose = eek_clutter_key_dispose;
}

static void
eek_clutter_key_init (EekClutterKey *self)
{
    EekClutterKeyPrivate *priv;
    priv = self->priv = EEK_CLUTTER_KEY_GET_PRIVATE (self);
    priv->key = NULL;
    priv->renderer = NULL;
}

static void
on_pressed (EekKey *key, gpointer user_data)
{
    ClutterActor *actor = user_data, *parent;

    parent = clutter_actor_get_parent (actor);
    clutter_actor_raise_top (parent);
    clutter_actor_raise_top (actor);
    clutter_actor_set_scale_with_gravity (actor,
                                          1.0,
                                          1.0,
                                          CLUTTER_GRAVITY_CENTER);

    clutter_actor_animate (actor, CLUTTER_EASE_IN_SINE, 150,
                           "scale-x", 1.5,
                           "scale-y", 1.5,
                           NULL);
}

static void
on_released (EekKey *key, gpointer user_data)
{
    ClutterActor *actor = user_data, *parent;

    parent = clutter_actor_get_parent (actor);
    clutter_actor_raise_top (parent);
    clutter_actor_raise_top (actor);
    clutter_actor_set_scale_with_gravity (actor,
                                          1.5,
                                          1.5,
                                          CLUTTER_GRAVITY_CENTER);
    clutter_actor_animate (actor, CLUTTER_EASE_OUT_SINE, 150,
                           "scale-x", 1.0,
                           "scale-y", 1.0,
                           NULL);
}

ClutterActor *
eek_clutter_key_new (EekKey *key, EekClutterRenderer *renderer)
{
    ClutterActor *actor;
    EekClutterKeyPrivate *priv;
    EekBounds bounds;
    gdouble scale;

    actor = g_object_new (EEK_TYPE_CLUTTER_KEY, NULL);
    priv = EEK_CLUTTER_KEY_GET_PRIVATE(actor);
    priv->key = g_object_ref_sink (key);
    priv->renderer = g_object_ref (renderer);

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    scale = eek_renderer_get_scale (EEK_RENDERER(priv->renderer));

    clutter_actor_set_position (actor,
                                bounds.x * scale,
                                bounds.y * scale);

    clutter_actor_set_reactive (actor, TRUE);

    g_signal_connect (priv->key, "pressed",
                      G_CALLBACK(on_pressed), actor);
    g_signal_connect (priv->key, "released",
                      G_CALLBACK(on_released), actor);

    return actor;
}
