/* 
 * Copyright (C) 2006 Sergey V. Udaltsov <svu@gnome.org>
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

#include <cogl/cogl.h>
#include <cogl/cogl-pango.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-clutter-key-actor.h"
#include "eek-keysym.h"
#include "eek-drawing.h"
#include "eek-section.h"
#include "eek-keyboard.h"

#define noKBDRAW_DEBUG

enum {
    PRESSED,
    RELEASED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (EekClutterKeyActor, eek_clutter_key_actor,
               CLUTTER_TYPE_GROUP);

#define EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_KEY_ACTOR, EekClutterKeyActorPrivate))

struct _EekClutterKeyActorPrivate
{
    EekClutterDrawingContext *context;
    EekClutterKey *key;
    ClutterActor *texture;
    gboolean is_pressed;
};

static ClutterActor *get_texture          (EekClutterKeyActor *actor);
static void          draw_key_on_layout   (EekClutterKeyActor *actor,
                                           PangoLayout        *layout);
static void          key_enlarge          (ClutterActor       *actor);
static void          key_shrink           (ClutterActor       *actor);

static void
eek_clutter_key_actor_real_paint (ClutterActor *self)
{
    EekClutterKeyActorPrivate *priv = EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE (self);
    PangoLayout *layout;
    PangoRectangle logical_rect = { 0, };
    CoglColor color;
    ClutterGeometry geom;
    EekBounds bounds;
    EekThemeNode *tnode;

    eek_element_get_bounds (EEK_ELEMENT(priv->key), &bounds);
    clutter_actor_set_anchor_point_from_gravity (self,
                                                 CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_position (self,
                                bounds.x + bounds.width / 2,
                                bounds.y + bounds.height / 2);

    if (!priv->texture) {
        priv->texture = get_texture (EEK_CLUTTER_KEY_ACTOR(self));
        clutter_actor_set_position (priv->texture, 0, 0);
        clutter_container_add_actor (CLUTTER_CONTAINER(self), priv->texture);
    }

    CLUTTER_ACTOR_CLASS (eek_clutter_key_actor_parent_class)->
        paint (self);

    /* Draw the label on the key. */
    layout = clutter_actor_create_pango_layout (self, NULL);
    draw_key_on_layout (EEK_CLUTTER_KEY_ACTOR(self), layout);
    pango_layout_get_extents (layout, NULL, &logical_rect);

    tnode = eek_clutter_key_get_theme_node (priv->key);
    if (tnode) {
        EekColor c;
        eek_theme_node_get_foreground_color (tnode, &c);
        cogl_color_set_from_4ub (&color, c.red, c.green, c.blue, c.alpha);
    } else
        cogl_color_set_from_4ub (&color, 0x80, 0x00, 0x00, 0xff);
    clutter_actor_get_allocation_geometry (self, &geom);
    cogl_pango_render_layout
        (layout,
         (geom.width - logical_rect.width / PANGO_SCALE) / 2,
         (geom.height - logical_rect.height / PANGO_SCALE) / 2,
         &color,
         0);
    g_object_unref (layout);
}

static void
eek_clutter_key_actor_real_pressed (EekClutterKeyActor *self)
{
    ClutterActor *actor, *section;

    actor = CLUTTER_ACTOR(self);

    /* Make sure the enlarged key show up on the keys which belong
       to other sections. */
    section = clutter_actor_get_parent (actor);
    clutter_actor_raise_top (section);
    clutter_actor_raise_top (actor);
    key_enlarge (actor);
}

static void
eek_clutter_key_actor_real_released (EekClutterKeyActor *self)
{
    ClutterActor *actor, *section;

    actor = CLUTTER_ACTOR(self);

    /* Make sure the enlarged key show up on the keys which belong
       to other sections. */
    section = clutter_actor_get_parent (actor);
    clutter_actor_raise_top (section);
    clutter_actor_raise_top (actor);
    key_shrink (actor);
}

static void
eek_clutter_key_actor_dispose (GObject *object)
{
    EekClutterKeyActorPrivate *priv = EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE(object);

    if (priv->context) {
        g_object_unref (priv->context);
        priv->context = NULL;
    }
    if (priv->key) {
        g_object_unref (priv->key);
        priv->key = NULL;
    }
    G_OBJECT_CLASS (eek_clutter_key_actor_parent_class)->dispose (object);
}

static void
eek_clutter_key_actor_class_init (EekClutterKeyActorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyActorPrivate));

    actor_class->paint = eek_clutter_key_actor_real_paint;

    gobject_class->dispose = eek_clutter_key_actor_dispose;

    /* signals */
    klass->pressed = eek_clutter_key_actor_real_pressed;
    klass->released = eek_clutter_key_actor_real_released;

    signals[PRESSED] =
        g_signal_new ("pressed",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekClutterKeyActorClass, pressed),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[RELEASED] =
        g_signal_new ("released",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekClutterKeyActorClass, released),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}

static void
on_button_press_event (ClutterActor *actor,
                       ClutterEvent *event,
                       gpointer user_data)
{
    EekClutterKeyActorPrivate *priv =
        EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE(actor);

    if (!priv->is_pressed) {
        priv->is_pressed = TRUE;
        /* priv->key will send back PRESSED event of actor. */
        g_signal_emit_by_name (priv->key, "pressed");
    }
}

static void
on_button_release_event (ClutterActor *actor,
                         ClutterEvent *event,
                         gpointer      user_data)
{
    EekClutterKeyActorPrivate *priv =
        EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE(actor);

    if (priv->is_pressed) {
        priv->is_pressed = FALSE;
        /* priv->key will send back RELEASED event of actor. */
        g_signal_emit_by_name (priv->key, "released");
    }
}

static gboolean
on_leave_event (ClutterActor *actor,
                ClutterEvent *event,
                gpointer      user_data)
{
    EekClutterKeyActorPrivate *priv =
        EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE(actor);

    if (priv->is_pressed) {
        priv->is_pressed = FALSE;
        /* priv->key will send back RELEASED event of actor. */
        g_signal_emit_by_name (priv->key, "released");
    }
    return FALSE;
}

static void
eek_clutter_key_actor_init (EekClutterKeyActor *self)
{
    EekClutterKeyActorPrivate *priv;

    priv = self->priv = EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE(self);
    priv->key = NULL;
    priv->texture = NULL;

    clutter_actor_set_reactive (CLUTTER_ACTOR(self), TRUE);

    g_signal_connect (self, "button-press-event",
                      G_CALLBACK (on_button_press_event), NULL);
    g_signal_connect (self, "button-release-event",
                      G_CALLBACK (on_button_release_event), NULL);
    g_signal_connect (self, "leave-event",
                      G_CALLBACK (on_leave_event), NULL);
}

ClutterActor *
eek_clutter_key_actor_new (EekClutterDrawingContext *context,
                           EekClutterKey *key)
{
    EekClutterKeyActor *actor;

    actor = g_object_new (EEK_TYPE_CLUTTER_KEY_ACTOR, NULL);
    actor->priv->context = context;
    g_object_ref_sink (actor->priv->context);
    actor->priv->key = key;
    g_object_ref_sink (actor->priv->key);
    return CLUTTER_ACTOR(actor);
}

#if 0
static void  
on_key_animate_complete (ClutterAnimation *animation,
                         gpointer user_data)
{
    ClutterActor *actor = (ClutterActor*)user_data;

    /* reset after effect */
    clutter_actor_set_opacity (actor, 0xff);
    clutter_actor_set_scale (actor, 1.0, 1.0);
}
#endif

static void
key_enlarge (ClutterActor *actor)
{
    clutter_actor_set_scale (actor, 1.0, 1.0);
    clutter_actor_animate (actor, CLUTTER_EASE_IN_SINE, 150,
                           "scale-x", 1.5,
                           "scale-y", 1.5,
                           NULL);
}

static void
key_shrink (ClutterActor *actor)
{
    clutter_actor_set_scale (actor, 1.5, 1.5);
    clutter_actor_animate (actor, CLUTTER_EASE_OUT_SINE, 150,
                           "scale-x", 1.0,
                           "scale-y", 1.0,
                           NULL);
}


static ClutterActor *
create_texture_for_key (EekClutterKey *key)
{
    ClutterActor *texture;
    cairo_t *cr;
    EekOutline *outline;
    EekBounds bounds;
    EekThemeNode *tnode;
    EekGradientType gradient_type = EEK_GRADIENT_VERTICAL;
    EekColor gradient_start = {0xFF, 0xFF, 0xFF, 0xFF},
        gradient_end = {0x80, 0x80, 0x80, 0xFF};

    outline = eek_key_get_outline (EEK_KEY(key));
    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
 
    texture = clutter_cairo_texture_new (bounds.width, bounds.height);
    cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE(texture));

    tnode = eek_clutter_key_get_theme_node (key);
    if (tnode)
        eek_theme_node_get_background_gradient (tnode,
                                                &gradient_type,
                                                &gradient_start,
                                                &gradient_end);
    eek_draw_outline (cr,
                      &bounds,
                      outline,
                      gradient_type,
                      &gradient_start,
                      &gradient_end);
    cairo_destroy (cr);
    return texture;
}

static ClutterActor *
get_texture (EekClutterKeyActor *actor)
{
    ClutterActor *texture;
    EekOutline *outline;

    outline = eek_key_get_outline (EEK_KEY(actor->priv->key));
    texture =
        eek_clutter_drawing_context_get_outline_texture (actor->priv->context,
                                                         outline);
    if (texture == NULL) {
        texture = create_texture_for_key (actor->priv->key);
        eek_clutter_drawing_context_set_outline_texture (actor->priv->context,
                                                         outline,
                                                         texture);
    } else
        texture = clutter_clone_new (texture);
    return texture;
}

static void
draw_key_on_layout (EekClutterKeyActor *self,
                    PangoLayout *layout)
{
    EekClutterKeyActorPrivate *priv = EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE (self);
    guint keysym;
    const gchar *label, *empty_label = "";
    EekKeysymCategory category;
    EekBounds bounds;
    PangoFontDescription *font;

    keysym = eek_key_get_keysym (EEK_KEY(priv->key));
    if (keysym == EEK_INVALID_KEYSYM)
        return;
    category = eek_keysym_get_category (keysym);
    if (category == EEK_KEYSYM_CATEGORY_UNKNOWN)
        return;

    font = eek_clutter_drawing_context_get_category_font (priv->context,
                                                          category);
    pango_layout_set_font_description (layout, font);

    eek_element_get_bounds (EEK_ELEMENT(priv->key), &bounds);
    pango_layout_set_width (layout, PANGO_SCALE * bounds.width);
    pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);

    label = eek_keysym_to_string (keysym);
    if (!label)
        label = empty_label;
    eek_draw_text_on_layout (layout, label);
    if (label != empty_label)
        g_free ((gpointer)label);
}
