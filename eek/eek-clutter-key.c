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
 * SECTION:eek-clutter-key
 * @short_description: #EekKey embedding a #ClutterActor
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-clutter-key.h"
#include "eek-clutter-key-actor.h"

G_DEFINE_TYPE (EekClutterKey, eek_clutter_key, EEK_TYPE_KEY);

#define EEK_CLUTTER_KEY_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_KEY, EekClutterKeyPrivate))

struct _EekClutterKeyPrivate
{
    EekClutterDrawingContext *context;
    ClutterActor *actor;
};

static void
eek_clutter_key_real_set_name (EekElement *self,
                               const gchar *name)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    EEK_ELEMENT_CLASS (eek_clutter_key_parent_class)->
        set_name (self, name);

    g_return_if_fail (priv->actor);

    clutter_actor_set_name (CLUTTER_ACTOR(priv->actor), name);
}

static void
eek_clutter_key_real_set_bounds (EekElement *self,
                                 EekBounds *bounds)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    EEK_ELEMENT_CLASS (eek_clutter_key_parent_class)->
        set_bounds (self, bounds);

    g_return_if_fail (priv->actor);

    clutter_actor_set_position (priv->actor, bounds->x, bounds->y);
    clutter_actor_set_size (priv->actor, bounds->width, bounds->height);
}

static void
eek_clutter_key_real_pressed (EekKey *key)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(key);

    if (priv->actor)
        g_signal_emit_by_name (priv->actor, "pressed");
}

static void
eek_clutter_key_real_released (EekKey *key)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(key);

    if (priv->actor)
        g_signal_emit_by_name (priv->actor, "released");
}

static void
eek_clutter_key_dispose (GObject *object)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(object);

    if (priv->context) {
        g_object_unref (priv->context);
        priv->context = NULL;
    }
    if (priv->actor) {
        g_object_unref (priv->actor);
        priv->actor = NULL;
    }
    G_OBJECT_CLASS (eek_clutter_key_parent_class)->dispose (object);
}

static void
eek_clutter_key_class_init (EekClutterKeyClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    EekElementClass *element_class = EEK_ELEMENT_CLASS (klass);
    EekKeyClass *key_class = EEK_KEY_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyPrivate));

    element_class->set_name = eek_clutter_key_real_set_name;
    element_class->set_bounds = eek_clutter_key_real_set_bounds;
    gobject_class->dispose = eek_clutter_key_dispose;

    /* signals */
    key_class->pressed = eek_clutter_key_real_pressed;
    key_class->released = eek_clutter_key_real_released;
}

static void
eek_clutter_key_init (EekClutterKey *self)
{
    EekClutterKeyPrivate *priv;
    priv = self->priv = EEK_CLUTTER_KEY_GET_PRIVATE (self);
    priv->actor = NULL;
}

ClutterActor *
eek_clutter_key_get_actor (EekClutterKey *key)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(key);

    if (!priv->actor) {
        g_return_val_if_fail (priv->context, NULL);
        priv->actor = eek_clutter_key_actor_new (priv->context, EEK_KEY(key));
        g_object_ref_sink (priv->actor);
    }
    return priv->actor;
}

EekKey *
eek_clutter_key_new (EekClutterDrawingContext *context, gint column, gint row)
{
    EekClutterKey *key;

    g_return_val_if_fail (context, NULL);
    key = g_object_new (EEK_TYPE_CLUTTER_KEY,
                        "column", column,
                        "row", row,
                        NULL);
    key->priv->context = context;
    g_object_ref_sink (key->priv->context);
    return EEK_KEY(key);
}
