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
 * SECTION:eek-clutter-key-actor
 * @short_description: Custom #ClutterActor drawing a key shape
 */

#include <math.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-clutter-key-actor.h"
#include "eek-keysym.h"
#include <cogl/cogl.h>
#include <cogl/cogl-pango.h>

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
    EekKey *key;
    ClutterActor *texture;
};

static struct {
    /* outline pointer -> ClutterTexture */
    GHashTable *outline_textures;
    gint outline_textures_ref_count;
} texture_cache;

static ClutterActor *get_texture        (EekClutterKeyActor *actor);
static void          draw_key_on_layout (EekKey             *key,
                                         PangoLayout        *layout);
static void          key_enlarge        (ClutterActor       *actor);
static void          key_shrink         (ClutterActor       *actor);

static void
eek_clutter_key_actor_real_paint (ClutterActor *self)
{
    EekClutterKeyActorPrivate *priv = EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE (self);
    PangoLayout *layout;
    PangoRectangle logical_rect = { 0, };
    CoglColor color;
    ClutterGeometry geom;
    EekBounds bounds;

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
    draw_key_on_layout (priv->key, layout);
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
eek_clutter_key_actor_real_get_preferred_width (ClutterActor *self,
                                                gfloat        for_height,
                                                gfloat       *min_width_p,
                                                gfloat       *natural_width_p)
{
    EekClutterKeyActorPrivate *priv = EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE (self);
    PangoLayout *layout;

    /* Draw the label on the key - just to validate the glyph cache. */
    layout = clutter_actor_create_pango_layout (self, NULL);
    draw_key_on_layout (priv->key, layout);
    cogl_pango_ensure_glyph_cache_for_layout (layout);
    g_object_unref (layout);

    CLUTTER_ACTOR_CLASS (eek_clutter_key_actor_parent_class)->
        get_preferred_width (self, for_height, min_width_p, natural_width_p);
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
eek_clutter_key_actor_finalize (GObject *object)
{
    EekClutterKeyActorPrivate *priv = EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE(object);

    g_object_unref (priv->key);
    if (priv->texture && --texture_cache.outline_textures_ref_count <= 0) {
        g_hash_table_unref (texture_cache.outline_textures);
        texture_cache.outline_textures = NULL;
    }
    G_OBJECT_CLASS (eek_clutter_key_actor_parent_class)->finalize (object);
}

static void
eek_clutter_key_actor_class_init (EekClutterKeyActorClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyActorPrivate));

    actor_class->paint = eek_clutter_key_actor_real_paint;
    /* FIXME: This is a workaround for the bug
     * http://bugzilla.openedhand.com/show_bug.cgi?id=2137 A developer
     * says this is not a right way to solve the original problem.
     */
    actor_class->get_preferred_width =
        eek_clutter_key_actor_real_get_preferred_width;

    gobject_class->finalize = eek_clutter_key_actor_finalize;

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

    /* priv->key will send back PRESSED event of actor. */
    g_signal_emit_by_name (priv->key, "pressed");
}

static void
on_button_release_event (ClutterActor *actor,
                         ClutterEvent *event,
                         gpointer      user_data)
{
    EekClutterKeyActorPrivate *priv =
        EEK_CLUTTER_KEY_ACTOR_GET_PRIVATE(actor);

    /* priv->key will send back RELEASED event of actor. */
    g_signal_emit_by_name (priv->key, "released");
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
}

ClutterActor *
eek_clutter_key_actor_new (EekKey *key)
{
    EekClutterKeyActor *actor;

    actor = g_object_new (EEK_TYPE_CLUTTER_KEY_ACTOR, NULL);
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

static gdouble
length (gdouble x, gdouble y)
{
    return sqrt (x * x + y * y);
}

static gdouble
point_line_distance (gdouble ax, gdouble ay, gdouble nx, gdouble ny)
{
    return ax * nx + ay * ny;
}

static void
normal_form (gdouble ax, gdouble ay,
	     gdouble bx, gdouble by,
	     gdouble * nx, gdouble * ny, gdouble * d)
{
    gdouble l;

    *nx = by - ay;
    *ny = ax - bx;

    l = length (*nx, *ny);

    *nx /= l;
    *ny /= l;

    *d = point_line_distance (ax, ay, *nx, *ny);
}

static void
inverse (gdouble a, gdouble b, gdouble c, gdouble d,
	 gdouble * e, gdouble * f, gdouble * g, gdouble * h)
{
    gdouble det;

    det = a * d - b * c;

    *e = d / det;
    *f = -b / det;
    *g = -c / det;
    *h = a / det;
}

static void
multiply (gdouble a, gdouble b, gdouble c, gdouble d,
	  gdouble e, gdouble f, gdouble * x, gdouble * y)
{
    *x = a * e + b * f;
    *y = c * e + d * f;
}

static void
intersect (gdouble n1x, gdouble n1y, gdouble d1,
	   gdouble n2x, gdouble n2y, gdouble d2, gdouble * x, gdouble * y)
{
    gdouble e, f, g, h;

    inverse (n1x, n1y, n2x, n2y, &e, &f, &g, &h);
    multiply (e, f, g, h, d1, d2, x, y);
}


/* draw an angle from the current point to b and then to c,
 * with a rounded corner of the given radius.
 */
static void
rounded_corner (cairo_t * cr,
		gdouble bx, gdouble by,
		gdouble cx, gdouble cy, gdouble radius)
{
    gdouble ax, ay;
    gdouble n1x, n1y, d1;
    gdouble n2x, n2y, d2;
    gdouble pd1, pd2;
    gdouble ix, iy;
    gdouble dist1, dist2;
    gdouble nx, ny, d;
    gdouble a1x, a1y, c1x, c1y;
    gdouble phi1, phi2;

    cairo_get_current_point (cr, &ax, &ay);
#ifdef KBDRAW_DEBUG
    printf ("        current point: (%f, %f), radius %f:\n", ax, ay,
            radius);
#endif

    /* make sure radius is not too large */
    dist1 = length (bx - ax, by - ay);
    dist2 = length (cx - bx, cy - by);

    radius = MIN (radius, MIN (dist1, dist2));

    /* construct normal forms of the lines */
    normal_form (ax, ay, bx, by, &n1x, &n1y, &d1);
    normal_form (bx, by, cx, cy, &n2x, &n2y, &d2);

    /* find which side of the line a,b the point c is on */
    if (point_line_distance (cx, cy, n1x, n1y) < d1)
        pd1 = d1 - radius;
    else
        pd1 = d1 + radius;

    /* find which side of the line b,c the point a is on */
    if (point_line_distance (ax, ay, n2x, n2y) < d2)
        pd2 = d2 - radius;
    else
        pd2 = d2 + radius;

    /* intersect the parallels to find the center of the arc */
    intersect (n1x, n1y, pd1, n2x, n2y, pd2, &ix, &iy);

    nx = (bx - ax) / dist1;
    ny = (by - ay) / dist1;
    d = point_line_distance (ix, iy, nx, ny);

    /* a1 is the point on the line a-b where the arc starts */
    intersect (n1x, n1y, d1, nx, ny, d, &a1x, &a1y);

    nx = (cx - bx) / dist2;
    ny = (cy - by) / dist2;
    d = point_line_distance (ix, iy, nx, ny);

    /* c1 is the point on the line b-c where the arc ends */
    intersect (n2x, n2y, d2, nx, ny, d, &c1x, &c1y);

    /* determine the first angle */
    if (a1x - ix == 0)
        phi1 = (a1y - iy > 0) ? M_PI_2 : 3 * M_PI_2;
    else if (a1x - ix > 0)
        phi1 = atan ((a1y - iy) / (a1x - ix));
    else
        phi1 = M_PI + atan ((a1y - iy) / (a1x - ix));

    /* determine the second angle */
    if (c1x - ix == 0)
        phi2 = (c1y - iy > 0) ? M_PI_2 : 3 * M_PI_2;
    else if (c1x - ix > 0)
        phi2 = atan ((c1y - iy) / (c1x - ix));
    else
        phi2 = M_PI + atan ((c1y - iy) / (c1x - ix));

    /* compute the difference between phi2 and phi1 mod 2pi */
    d = phi2 - phi1;
    while (d < 0)
        d += 2 * M_PI;
    while (d > 2 * M_PI)
        d -= 2 * M_PI;

#ifdef KBDRAW_DEBUG
    printf ("        line 1 to: (%f, %f):\n", a1x, a1y);
#endif
    if (!(isnan (a1x) || isnan (a1y)))
        cairo_line_to (cr, a1x, a1y);

    /* pick the short arc from phi1 to phi2 */
    if (d < M_PI)
        cairo_arc (cr, ix, iy, radius, phi1, phi2);
    else
        cairo_arc_negative (cr, ix, iy, radius, phi1, phi2);

#ifdef KBDRAW_DEBUG
    printf ("        line 2 to: (%f, %f):\n", cx, cy);
#endif
    cairo_line_to (cr, cx, cy);
}

void
draw_rounded_polygon (cairo_t  *cr,
                      gboolean  filled,
                      gdouble   radius,
                      EekPoint *points,
                      gint      num_points)
{
    gint i, j;

    cairo_move_to (cr,
                   (gdouble) (points[num_points - 1].x +
                              points[0].x) / 2,
                   (gdouble) (points[num_points - 1].y +
                              points[0].y) / 2);


#ifdef KBDRAW_DEBUG
    printf ("    rounded polygon of radius %f:\n", radius);
#endif
    for (i = 0; i < num_points; i++) {
        j = (i + 1) % num_points;
        rounded_corner (cr, (gdouble) points[i].x,
                        (gdouble) points[i].y,
                        (gdouble) (points[i].x + points[j].x) / 2,
                        (gdouble) (points[i].y + points[j].y) / 2,
                        radius);
#ifdef KBDRAW_DEBUG
        printf ("      corner (%d, %d) -> (%d, %d):\n",
                points[i].x, points[i].y, points[j].x,
                points[j].y);
#endif
    };
    cairo_close_path (cr);

    if (filled)
        cairo_fill (cr);
    else
        cairo_stroke (cr);
}

static ClutterActor *
create_texture_for_key (EekKey *key)
{
    ClutterActor *texture;
    cairo_t *cr;
    cairo_pattern_t *pat;
    EekOutline *outline;
    EekBounds bounds;

    outline = eek_key_get_outline (EEK_KEY(key));
    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
 
    texture = clutter_cairo_texture_new (bounds.width, bounds.height);
    cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE(texture));
    cairo_set_line_width (cr, 1);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    pat = cairo_pattern_create_linear (0.0, 0.0,  0.0, 256.0);
    cairo_pattern_add_color_stop_rgba (pat, 1, 0.5, 0.5, 0.5, 1);
    cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);

    cairo_set_source (cr, pat);

    draw_rounded_polygon (cr,
                          TRUE,
                          outline->corner_radius,
                          outline->points,
                          outline->num_points);

    cairo_pattern_destroy (pat);

    cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.5);
    draw_rounded_polygon (cr,
                          FALSE,
                          outline->corner_radius,
                          outline->points,
                          outline->num_points);
    cairo_destroy (cr);
    return texture;
}

static ClutterActor *
get_texture (EekClutterKeyActor *actor)
{
    ClutterActor *texture;
    EekOutline *outline;

    if (!texture_cache.outline_textures)
        texture_cache.outline_textures = g_hash_table_new_full (g_direct_hash,
                                                                g_direct_equal,
                                                                NULL,
                                                                g_free);
    outline = eek_key_get_outline (actor->priv->key);
    texture = g_hash_table_lookup (texture_cache.outline_textures, outline);
    if (texture == NULL) {
        texture = create_texture_for_key (actor->priv->key);
        g_hash_table_insert (texture_cache.outline_textures, outline, texture);
    } else
        texture = clutter_clone_new (texture);
    texture_cache.outline_textures_ref_count++;
    return texture;
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
    const gchar *label, *empty_label = "";
    gdouble scale_x, scale_y;

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    keysym = eek_key_get_keysym (key);
    if (keysym == EEK_INVALID_KEYSYM)
        return;
    label = eek_keysym_to_string (keysym);
    if (!label)
        label = empty_label;

    /* Compute the layout extents. */
    buffer = pango_layout_copy (layout);
    draw_text_on_layout (buffer, label, 1.0);
    pango_layout_get_extents (buffer, NULL, &logical_rect);
    scale_x = scale_y = 1.0;
    if (PANGO_PIXELS(logical_rect.width) > bounds.width)
        scale_x = bounds.width / PANGO_PIXELS(logical_rect.width);
    if (PANGO_PIXELS(logical_rect.height) > bounds.height)
        scale_y = bounds.height / PANGO_PIXELS(logical_rect.height);
    g_object_unref (buffer);

    /* Actually draw on the layout */
    draw_text_on_layout (layout,
                         label,
                         (scale_x < scale_y ? scale_x : scale_y) * 0.8);
    if (label != empty_label)
        g_free ((gpointer)label);
}
