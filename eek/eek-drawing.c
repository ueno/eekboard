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

#include <math.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-keyboard.h"
#include "eek-key.h"
#include "eek-drawing.h"
#include "eek-keysym.h"


void
eek_draw_text_on_layout (PangoLayout *layout,
                         const gchar *text)
{
    /* pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER); */
    pango_layout_set_text (layout, text, -1);
}

struct _GetFontSizeCallbackData
{
    PangoLayout *layout;
    EekKeysymCategory category;
    gint minimum_font_size;
    gint font_size;
};
typedef struct _GetFontSizeCallbackData GetFontSizeCallbackData;

static gint
get_font_size (const gchar *text,
               EekBounds   *bounds,
               PangoLayout *layout)
{
    gdouble scale_x, scale_y;
    const PangoFontDescription *base_font_desc;
    PangoFontDescription *font_desc;
    PangoRectangle logical_rect = { 0, };
    gint font_size;

    layout = pango_layout_copy (layout);
    base_font_desc = pango_layout_get_font_description (layout);
    font_desc = pango_font_description_copy (base_font_desc);

    font_size = eek_bounds_long_side (bounds) * PANGO_SCALE;
    pango_font_description_set_size (font_desc, font_size);
    pango_layout_set_font_description (layout, font_desc);
    pango_font_description_free (font_desc);

    eek_draw_text_on_layout (layout, text);
    pango_layout_get_extents (layout, NULL, &logical_rect);

    scale_x = scale_y = 1.0;
    if (logical_rect.width > bounds->width * PANGO_SCALE)
        scale_x = bounds->width * PANGO_SCALE / logical_rect.width;
    if (logical_rect.height > bounds->height * PANGO_SCALE)
        scale_y = bounds->height * PANGO_SCALE / logical_rect.height;
    g_object_unref (layout);
    return font_size * (scale_x < scale_y ? scale_x : scale_y);
}

static void
egf_key_callback (EekElement *element,
                  gpointer user_data)
{
    EekKey *key = EEK_KEY(element);
    GetFontSizeCallbackData *data = user_data;
    gdouble font_size;
    guint keysym;
    EekBounds bounds;
    gchar *label;

    keysym = eek_key_get_keysym (key);
    if (keysym == EEK_INVALID_KEYSYM ||
        eek_keysym_get_category (keysym) != data->category)
        return;

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    label = eek_keysym_to_string (keysym);
    if (!label)
        return;
    font_size = get_font_size (label, &bounds, data->layout);
    if (font_size < data->font_size && font_size >= data->minimum_font_size)
        data->font_size = font_size;
    g_free (label);
}

static void
egf_section_callback (EekElement *element,
                      gpointer    user_data)
{
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 egf_key_callback,
                                 user_data);
}

static PangoFontDescription *
get_font_for_category (EekKeyboard      *keyboard,
                       EekKeysymCategory category,
                       PangoLayout      *layout,
                       gdouble           minimum_font_size,
                       gdouble           maximum_font_size)
{
    GetFontSizeCallbackData data;
    PangoFontDescription *font_desc;
    const PangoFontDescription *base_font_desc;

    data.layout = layout;
    data.category = category;
    data.minimum_font_size = minimum_font_size;
    data.font_size = maximum_font_size;

    eek_container_foreach_child (EEK_CONTAINER(keyboard),
                                 egf_section_callback,
                                 &data);

    base_font_desc = pango_layout_get_font_description (layout);
    font_desc = pango_font_description_copy (base_font_desc);
    pango_font_description_set_size (font_desc, data.font_size);

    return font_desc;
}

void
eek_get_fonts (EekKeyboard           *keyboard,
               PangoLayout           *layout,
               PangoFontDescription **fonts)
{
    EekBounds bounds;
    PangoFontDescription *font_desc;
    gint font_size;

    /* font for EEK_KEYSYM_CATEGORY_LETTER */
    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);
    font_desc = get_font_for_category (keyboard,
                                       EEK_KEYSYM_CATEGORY_LETTER,
                                       layout,
                                       0,
                                       eek_bounds_long_side (&bounds) *
                                       PANGO_SCALE);
    font_size = pango_font_description_get_size (font_desc);
    fonts[EEK_KEYSYM_CATEGORY_LETTER] = font_desc;

    /* font for EEK_KEYSYM_CATEGORY_FUNCTION */
    font_desc = get_font_for_category (keyboard,
                                       EEK_KEYSYM_CATEGORY_FUNCTION,
                                       layout,
                                       font_size * 0.3,
                                       font_size);
    fonts[EEK_KEYSYM_CATEGORY_FUNCTION] = font_desc;

    /* font for EEK_KEYSYM_CATEGORY_KEYNAME */
    font_desc = get_font_for_category (keyboard,
                                       EEK_KEYSYM_CATEGORY_KEYNAME,
                                       layout,
                                       font_size * 0.3,
                                       font_size);
    fonts[EEK_KEYSYM_CATEGORY_KEYNAME] = font_desc;
}

void
eek_draw_outline (cairo_t        *cr,
                  EekOutline     *outline,
                  EekGradientType gradient_type,
                  EekColor       *gradient_start,
                  EekColor       *gradient_end)
{
    cairo_pattern_t *pat;

    cairo_set_line_width (cr, 1);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

    switch (gradient_type) {
    case EEK_GRADIENT_VERTICAL:
        pat = cairo_pattern_create_linear (0.0, 0.0, 0.0, 256.0);
        break;
    case EEK_GRADIENT_HORIZONTAL:
        pat = cairo_pattern_create_linear (0.0, 0.0, 256.0, 0.0);
        break;
    case EEK_GRADIENT_RADIAL:
        pat = cairo_pattern_create_radial (0.0, 0.0, 0.0,
                                           0.0, 0.0, 256.0);
        break;
    default:
        pat = NULL;
    }

    if (pat) {
        cairo_pattern_add_color_stop_rgba (pat, 0,
                                           gradient_start->red / 255.0,
                                           gradient_start->green / 255.0,
                                           gradient_start->blue / 255.0,
                                           gradient_start->alpha / 255.0);
        cairo_pattern_add_color_stop_rgba (pat, 1,
                                           gradient_end->red / 255.0,
                                           gradient_end->green / 255.0,
                                           gradient_end->blue / 255.0,
                                           gradient_end->alpha / 255.0);
        cairo_set_source (cr, pat);
    }

    eek_draw_rounded_polygon (cr,
                              TRUE,
                              outline->corner_radius,
                              outline->points,
                              outline->num_points);

    if (pat)
        cairo_pattern_destroy (pat);

    cairo_set_source_rgba (cr, 0.3, 0.3, 0.3, 0.5);
    eek_draw_rounded_polygon (cr,
                              FALSE,
                              outline->corner_radius,
                              outline->points,
                              outline->num_points);
}

void
eek_draw_key_label (cairo_t               *cr,
                    EekKey                *key,
                    PangoFontDescription **fonts)
{
    guint keysym;
    EekKeysymCategory category;
    gchar *label;
    PangoLayout *layout;
    PangoRectangle logical_rect = { 0, };
    EekBounds bounds;

    keysym = eek_key_get_keysym (key);
    if (keysym == EEK_INVALID_KEYSYM)
        return;

    category = eek_keysym_get_category (keysym);
    if (category == EEK_KEYSYM_CATEGORY_UNKNOWN)
        return;

    label = eek_keysym_to_string (keysym);
    if (!label)
        return;

    eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
    layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layout, fonts[category]);
    pango_layout_set_width (layout, PANGO_SCALE * bounds.width);
    pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);
    pango_layout_set_text (layout, label, -1);
    pango_layout_get_extents (layout, NULL, &logical_rect);
    cairo_rel_move_to (cr,
                   (bounds.width - logical_rect.width / PANGO_SCALE) / 2,
                   (bounds.height - logical_rect.height / PANGO_SCALE) / 2);
    pango_cairo_show_layout (cr, layout);
    g_free (label);
    g_object_unref (layout);
}

/*
 * The functions below are borrowed from
 * libgnomekbd/gkbd-keyboard-drawing.c.
 * Copyright (C) 2006 Sergey V. Udaltsov <svu@gnome.org>
 *
 * length(), point_line_distance(), normal_form(), inverse(), multiply(),
 * intersect(), rounded_corner(), draw_rounded_polygon()
 */

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
eek_draw_rounded_polygon (cairo_t  *cr,
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
