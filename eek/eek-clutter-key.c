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
 * @short_description: #EekKey implemented as a #ClutterActor
 *
 * The #EekClutterKey class implements the #EekKeyIface interface as a
 * #ClutterActor.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-clutter-key.h"
#include "eek-simple-key.h"
#include "eek-keysym.h"

enum {
    PROP_0,
    PROP_KEYSYMS,
    PROP_COLUMN,
    PROP_ROW,
    PROP_OUTLINE,
    PROP_BOUNDS,
    PROP_GROUP,
    PROP_LEVEL,
    PROP_LAST
};

static void eek_key_iface_init (EekKeyIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekClutterKey, eek_clutter_key,
                         CLUTTER_TYPE_GROUP,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_KEY,
                                                eek_key_iface_init));

#define EEK_CLUTTER_KEY_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_KEY, EekClutterKeyPrivate))

struct _EekClutterKeyPrivate
{
    EekSimpleKey *simple;
};

static void
eek_clutter_key_real_set_keysyms (EekKey *self,
                                  guint  *keysyms,
                                  gint    groups,
                                  gint    levels)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_keysyms (EEK_KEY(priv->simple), keysyms, groups, levels);
}

static gint
eek_clutter_key_real_get_groups (EekKey *self)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, -1);
    return eek_key_get_groups (EEK_KEY(priv->simple));
}

static guint
eek_clutter_key_real_get_keysym (EekKey *self)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, EEK_INVALID_KEYSYM);
    return eek_key_get_keysym (EEK_KEY(priv->simple));
}

static void
eek_clutter_key_real_set_index (EekKey *self,
                                gint    column,
                                gint    row)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_index (EEK_KEY(priv->simple), column, row);
}

static void
eek_clutter_key_real_get_index (EekKey *self,
                                gint   *column,
                                gint   *row)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_get_index (EEK_KEY(priv->simple), column, row);
}

static void
eek_clutter_key_real_set_outline (EekKey *self, EekOutline *outline)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_outline (EEK_KEY(priv->simple), outline);
}

static EekOutline *
eek_clutter_key_real_get_outline (EekKey *self)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, NULL);
    return eek_key_get_outline (EEK_KEY(priv->simple));
}

static void
eek_clutter_key_real_set_bounds (EekKey *self, EekBounds *bounds)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_bounds (EEK_KEY(priv->simple), bounds);

    clutter_actor_set_anchor_point_from_gravity (CLUTTER_ACTOR(self),
                                                 CLUTTER_GRAVITY_CENTER);
    clutter_actor_set_position (CLUTTER_ACTOR(self),
                                bounds->x + bounds->w / 2,
                                bounds->y + bounds->h / 2);
}

static void
eek_clutter_key_real_get_bounds (EekKey    *self,
                                 EekBounds *bounds)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    return eek_key_get_bounds (EEK_KEY(priv->simple), bounds);
}

static void
eek_clutter_key_real_set_keysym_index (EekKey *self,
                                      gint    group,
                                      gint    level)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_index (EEK_KEY(priv->simple), group, level);
}

static void
eek_clutter_key_real_get_keysym_index (EekKey *self, gint *group, gint *level)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    return eek_key_get_keysym_index (EEK_KEY(priv->simple), group, level);
}

static void
eek_key_iface_init (EekKeyIface *iface)
{
    iface->set_keysyms = eek_clutter_key_real_set_keysyms;
    iface->get_groups = eek_clutter_key_real_get_groups;
    iface->get_keysym = eek_clutter_key_real_get_keysym;
    iface->set_index = eek_clutter_key_real_set_index;
    iface->get_index = eek_clutter_key_real_get_index;
    iface->set_outline = eek_clutter_key_real_set_outline;
    iface->get_outline = eek_clutter_key_real_get_outline;
    iface->set_bounds = eek_clutter_key_real_set_bounds;
    iface->get_bounds = eek_clutter_key_real_get_bounds;
    iface->set_keysym_index = eek_clutter_key_real_set_keysym_index;
    iface->get_keysym_index = eek_clutter_key_real_get_keysym_index;
}

static void
eek_clutter_key_dispose (GObject *object)
{
    clutter_group_remove_all (CLUTTER_GROUP(object));
    G_OBJECT_CLASS (eek_clutter_key_parent_class)->dispose (object);
}

static void
eek_clutter_key_finalize (GObject *object)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(object);

    g_object_unref (priv->simple);
    G_OBJECT_CLASS (eek_clutter_key_parent_class)->finalize (object);
}

static void
draw_text_on_layout (PangoLayout *layout,
                     const gchar *text,
                     gdouble      scale)
{
    PangoFontDescription *font_desc;

#define FONT_SIZE (720 * 50)
    /* FIXME: Font should be configurable through a property. */
    font_desc = pango_font_description_from_string ("Sans");
    pango_font_description_set_size (font_desc, FONT_SIZE * scale);
    pango_layout_set_font_description (layout, font_desc);
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    pango_layout_set_text (layout, text, -1);
    pango_font_description_free (font_desc);
}

static void
draw_key_on_layout (EekKey      *key,
                    PangoLayout *layout)
{
    PangoLayout *buffer;
    PangoRectangle logical_rect = { 0, };
    EekBounds bounds;
    guint keysym;
    const gchar *label;
    gdouble scale_x, scale_y;

    eek_key_get_bounds (key, &bounds);
    keysym = eek_key_get_keysym (key);
    if (keysym == EEK_INVALID_KEYSYM)
        return;
    label = eek_keysym_to_string (keysym);
    if (!label)
        label = "";

    /* Compute the layout extents. */
    buffer = pango_layout_copy (layout);
    draw_text_on_layout (buffer, label, 1.0);
    pango_layout_get_extents (buffer, NULL, &logical_rect);
    scale_x = scale_y = 1.0;
    if (PANGO_PIXELS(logical_rect.width) > bounds.w)
        scale_x = bounds.w / PANGO_PIXELS(logical_rect.width);
    if (PANGO_PIXELS(logical_rect.height) > bounds.h)
        scale_y = bounds.h / PANGO_PIXELS(logical_rect.height);
    g_object_unref (buffer);

    /* Actually draw on the layout */
    draw_text_on_layout (layout, label, scale_x < scale_y ? scale_x : scale_y);
}

static void
eek_clutter_key_paint (ClutterActor *self)
{
    PangoLayout *layout;
    PangoRectangle logical_rect = { 0, };
    CoglColor color;
    ClutterGeometry geom;
    EekBounds bounds;
    gfloat width, height;
    gfloat x, y;

    g_return_if_fail (CLUTTER_IS_ACTOR(self));
    g_return_if_fail (EEK_IS_KEY(self));

    /* Draw the background texture first. */
    CLUTTER_ACTOR_CLASS (eek_clutter_key_parent_class)->paint (self);

    /* Draw the label on the key. */
    layout = clutter_actor_create_pango_layout (self, NULL);
    draw_key_on_layout (EEK_KEY(self), layout);
    pango_layout_get_extents (layout, NULL, &logical_rect);

    /* FIXME: Color should be configurable through a property. */
    cogl_color_set_from_4ub (&color, 0x80, 0x00, 0x00, 0xff);
    clutter_actor_get_allocation_geometry (self, &geom);
    cogl_pango_render_layout (layout,
                              (geom.width - logical_rect.width / PANGO_SCALE) / 2,
                              (geom.height - logical_rect.height / PANGO_SCALE) / 2,
                              &color,
                              0);
    g_object_unref (layout);
}

/* FIXME: This is a workaround for the bug
 * http://bugzilla.openedhand.com/show_bug.cgi?id=2137 A developer
 * says this is not a right way to solve the original problem.
 */
static void
eek_clutter_key_get_preferred_width (ClutterActor *self,
                                     gfloat        for_height,
                                     gfloat       *min_width_p,
                                     gfloat       *natural_width_p)
{
    PangoLayout *layout;
    PangoFontDescription *font_desc;
    PangoRectangle logical_rect = { 0, };
    EekBounds bounds;
    guint keysym;
    const gchar *label;
    gdouble scale = 1.0;

    eek_key_get_bounds (EEK_KEY(self), &bounds);
    keysym = eek_key_get_keysym (EEK_KEY(self));
    g_return_if_fail (keysym != EEK_INVALID_KEYSYM);
    label = eek_keysym_to_string (keysym);
    if (!label)
        label = "";

    /* Draw the label on the key. */
    layout = clutter_actor_create_pango_layout (self, NULL);
    draw_key_on_layout (EEK_KEY(self), layout);
    pango_layout_get_extents (layout, NULL, &logical_rect);

    cogl_pango_ensure_glyph_cache_for_layout (layout);
    g_object_unref (layout);

    CLUTTER_ACTOR_CLASS (eek_clutter_key_parent_class)->get_preferred_width
        (self, for_height, min_width_p, natural_width_p);
}

static void
eek_clutter_key_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(object);

    g_return_if_fail (priv);
    switch (prop_id) {
        /* Properties which affect the appearance of the key as a
           ClutterActor. */
    case PROP_BOUNDS:
        eek_key_set_bounds (EEK_KEY(object),
                            g_value_get_boxed (value));
        break;
    case PROP_OUTLINE:
        eek_key_set_outline (EEK_KEY(object),
                             g_value_get_pointer (value));
        break;
        /* Otherwise delegate to priv->simple or the parent. */
    case PROP_KEYSYMS:
    case PROP_COLUMN:
    case PROP_GROUP:
    case PROP_ROW:
    case PROP_LEVEL:
        g_object_set_property (G_OBJECT(priv->simple),
                               g_param_spec_get_name (pspec),
                               value);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_clutter_key_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    EekClutterKeyPrivate *priv = EEK_CLUTTER_KEY_GET_PRIVATE(object);
    EekBounds bounds;

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_BOUNDS:
        eek_key_get_bounds (EEK_KEY(object), &bounds);
        g_value_set_boxed (value, &bounds);
        break;
    case PROP_KEYSYMS:
    case PROP_COLUMN:
    case PROP_ROW:
    case PROP_OUTLINE:
    case PROP_GROUP:
    case PROP_LEVEL:
        g_object_get_property (G_OBJECT(priv->simple),
                               g_param_spec_get_name (pspec),
                               value);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_clutter_key_class_init (EekClutterKeyClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyPrivate));

    actor_class->paint = eek_clutter_key_paint;
    /* FIXME: This is a workaround for the bug
     * http://bugzilla.openedhand.com/show_bug.cgi?id=2137 A developer
     * says this is not a right way to solve the original problem.
     */
    actor_class->get_preferred_width = eek_clutter_key_get_preferred_width;

    gobject_class->set_property = eek_clutter_key_set_property;
    gobject_class->get_property = eek_clutter_key_get_property;
    gobject_class->finalize     = eek_clutter_key_finalize;
    gobject_class->dispose      = eek_clutter_key_dispose;

    g_object_class_override_property (gobject_class,
                                      PROP_KEYSYMS,
                                      "keysyms");
    g_object_class_override_property (gobject_class,
                                      PROP_COLUMN,
                                      "column");
    g_object_class_override_property (gobject_class,
                                      PROP_ROW,
                                      "row");
    g_object_class_override_property (gobject_class,
                                      PROP_OUTLINE,
                                      "outline");
    g_object_class_override_property (gobject_class,
                                      PROP_BOUNDS,
                                      "bounds");
    g_object_class_override_property (gobject_class,
                                      PROP_GROUP,
                                      "group");
    g_object_class_override_property (gobject_class,
                                      PROP_LEVEL,
                                      "level");
}

static void  
on_key_animate_complete (ClutterAnimation *animation,
                         gpointer user_data)
{
    ClutterActor *actor = (ClutterActor*)user_data;

    /* reset after effect */
    clutter_actor_set_opacity (actor, 0xff);
    clutter_actor_set_scale (actor, 1.0, 1.0);
}

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

static gboolean
on_event (ClutterActor *actor,
	  ClutterEvent *event,
	  gpointer      user_data)
{
    g_return_val_if_fail (EEK_IS_KEY(actor), FALSE);
    if (clutter_event_get_source (event) == actor) {
        ClutterActor *section;

        /* Make sure the enlarged key show up on the keys which belong
           to other sections. */
        section = clutter_actor_get_parent (actor);
        clutter_actor_raise_top (section);
        clutter_actor_raise_top (actor);
        if (event->type == CLUTTER_BUTTON_PRESS)
            key_enlarge (actor);
        else if (event->type == CLUTTER_BUTTON_RELEASE)
            key_shrink (actor);
    }
    return FALSE;
}

static void
eek_clutter_key_init (EekClutterKey *self)
{
    EekClutterKeyPrivate *priv;

    priv = self->priv = EEK_CLUTTER_KEY_GET_PRIVATE(self);
    priv->simple = g_object_new (EEK_TYPE_SIMPLE_KEY, NULL);

    clutter_actor_set_reactive (CLUTTER_ACTOR(self), TRUE);
    g_signal_connect (self, "event", G_CALLBACK (on_event), NULL);
}

ClutterActor *
eek_clutter_key_create_texture (EekClutterKey *key)
{
    EekOutline *outline;
    EekBounds bounds;
    ClutterActor *texture;
    cairo_t *cr;
    cairo_pattern_t *pat;

    outline = eek_key_get_outline (EEK_KEY(key));
    eek_key_get_bounds (EEK_KEY(key), &bounds);

    texture = clutter_cairo_texture_new (bounds.w, bounds.h);
    cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE(texture));
    cairo_set_line_width (cr, 1);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    pat = cairo_pattern_create_linear (0.0, 0.0,  0.0, 256.0);
    cairo_pattern_add_color_stop_rgba (pat, 1, 0.5, 0.5, 0.5, 1);
    cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);

    cairo_set_source (cr, pat);

    eek_cairo_draw_rounded_polygon (cr,
                                    TRUE,
                                    outline->corner_radius,
                                    outline->points,
                                    outline->num_points);

    cairo_pattern_destroy (pat);

    cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.5);
    eek_cairo_draw_rounded_polygon (cr,
                                    FALSE,
                                    outline->corner_radius,
                                    outline->points,
                                    outline->num_points);
    cairo_destroy (cr);
    return texture;
}

void
eek_clutter_key_set_texture (EekClutterKey *key,
                             ClutterActor  *texture)
{
    clutter_actor_set_position (texture, 0, 0);
    clutter_container_add_actor (CLUTTER_CONTAINER(key), texture);
}
