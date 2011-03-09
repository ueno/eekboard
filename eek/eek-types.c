/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
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
 * SECTION:eek-types
 * @short_description: Miscellaneous types
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <string.h>
#include <math.h>

#include "eek-types.h"

/* EekPoint */
static EekPoint *
eek_point_copy (const EekPoint *point)
{
    return g_slice_dup (EekPoint, point);
}

static void
eek_point_free (EekPoint *point)
{
    g_slice_free (EekPoint, point);
}

GType
eek_point_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0)
        our_type =
            g_boxed_type_register_static ("EekPoint",
                                          (GBoxedCopyFunc)eek_point_copy,
                                          (GBoxedFreeFunc)eek_point_free);
    return our_type;
}

void
eek_point_rotate (EekPoint *point, gint angle)
{
    gdouble r, phi;

    phi = atan2 (point->y, point->x);
    r = sqrt (point->x * point->x + point->y * point->y);
    phi += angle * M_PI / 180;
    point->x = r * cos (phi);
    point->y = r * sin (phi);
}

/* EekBounds */
static EekBounds *
eek_bounds_copy (const EekBounds *bounds)
{
    return g_slice_dup (EekBounds, bounds);
}

static void
eek_bounds_free (EekBounds *bounds)
{
    g_slice_free (EekBounds, bounds);
}

GType
eek_bounds_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0)
        our_type =
            g_boxed_type_register_static ("EekBounds",
                                          (GBoxedCopyFunc)eek_bounds_copy,
                                          (GBoxedFreeFunc)eek_bounds_free);
    return our_type;
}

/* EekOutline */
EekOutline *
eek_outline_copy (const EekOutline *outline)
{
    EekOutline *_outline = g_slice_dup (EekOutline, outline);
    _outline->corner_radius = outline->corner_radius;
    _outline->num_points = outline->num_points;
    _outline->points = g_slice_alloc0 (sizeof (EekPoint) * outline->num_points);
    memcpy (_outline->points, outline->points, sizeof (EekPoint) * outline->num_points);
    return _outline;
}

void
eek_outline_free (EekOutline *outline)
{
    g_slice_free1 (sizeof (EekPoint) * outline->num_points, outline->points);
    g_slice_free (EekOutline, outline);
}

GType
eek_outline_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0)
        our_type =
            g_boxed_type_register_static ("EekOutline",
                                          (GBoxedCopyFunc)eek_outline_copy,
                                          (GBoxedFreeFunc)eek_outline_free);
    return our_type;
}

/* EekColor */
EekColor *
eek_color_copy (const EekColor *color)
{
    return g_slice_dup (EekColor, color);
}

void
eek_color_free (EekColor *color)
{
    g_slice_free (EekColor, color);
}

GType
eek_color_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0)
        our_type =
            g_boxed_type_register_static ("EekColor",
                                          (GBoxedCopyFunc)eek_color_copy,
                                          (GBoxedFreeFunc)eek_color_free);
    return our_type;
}

EekColor *
eek_color_new (gdouble red,
               gdouble green,
               gdouble blue,
               gdouble alpha)
{
    EekColor *color;

    color = g_slice_new (EekColor);
    color->red = red;
    color->green = green;
    color->blue = blue;
    color->alpha = alpha;

    return color;
}
