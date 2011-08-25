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
 * @title: Miscellaneous Types
 * @short_description: Miscellaneous types used in Libeek
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <string.h>
#include <math.h>

#include "eek-types.h"

/* EekPoint */
G_DEFINE_BOXED_TYPE(EekPoint, eek_point, eek_point_copy, eek_point_free);

EekPoint *
eek_point_copy (const EekPoint *point)
{
    return g_slice_dup (EekPoint, point);
}

void
eek_point_free (EekPoint *point)
{
    g_slice_free (EekPoint, point);
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
G_DEFINE_BOXED_TYPE(EekBounds, eek_bounds, eek_bounds_copy, eek_bounds_free);

EekBounds *
eek_bounds_copy (const EekBounds *bounds)
{
    return g_slice_dup (EekBounds, bounds);
}

void
eek_bounds_free (EekBounds *bounds)
{
    g_slice_free (EekBounds, bounds);
}

/* EekOutline */
G_DEFINE_BOXED_TYPE(EekOutline, eek_outline,
                    eek_outline_copy, eek_outline_free);

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

/* EekColor */
G_DEFINE_BOXED_TYPE(EekColor, eek_color, eek_color_copy, eek_color_free);

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
