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
#ifndef EEK_TYPES_H
#define EEK_TYPES_H 1

#include <glib-object.h>

G_BEGIN_DECLS

typedef enum {
    EEK_ORIENTATION_VERTICAL,
    EEK_ORIENTATION_HORIZONTAL,
    EEK_ORIENTATION_INVALID = -1
} EekOrientation;

typedef struct _EekKeyboard EekKeyboard;

/**
 * EekKeysymMatrix:
 * @data: array of keysyms
 * @num_groups: the number of groups (rows)
 * @num_levels: the number of levels (columns)
 *
 * Symbol matrix of a key.
 */
struct _EekKeysymMatrix
{
    guint *data;
    gint num_groups;
    gint num_levels;
};
typedef struct _EekKeysymMatrix EekKeysymMatrix;

#define EEK_TYPE_KEYSYM_MATRIX (eek_keysym_matrix_get_type ())
GType eek_keysym_matrix_get_type (void) G_GNUC_CONST;

/**
 * EekPoint:
 * @x: X coordinate of the point
 * @y: Y coordinate of the point
 *
 * 2D vertex
 */
struct _EekPoint
{
    gdouble x;
    gdouble y;
};
typedef struct _EekPoint EekPoint;

#define EEK_TYPE_POINT (eek_point_get_type ())
GType eek_point_get_type (void) G_GNUC_CONST;

/**
 * EekBounds:
 * @x: X coordinate of the top left point
 * @y: Y coordinate of the top left point
 * @width: width of the box
 * @height: height of the box
 *
 * 2D bounding box
 */
struct _EekBounds
{
    gdouble x;
    gdouble y;
    gdouble width;
    gdouble height;
};
typedef struct _EekBounds EekBounds;

#define EEK_TYPE_BOUNDS (eek_bounds_get_type ())
GType eek_bounds_get_type (void) G_GNUC_CONST;

/**
 * EekOutline:
 * @corner_radius: radius of corners of rounded polygon
 * @points: an array of points represents a polygon
 * @num_points: the length of @points
 *
 * 2D rounded polygon used to draw key shape
 */
struct _EekOutline
{
    gdouble corner_radius;
    EekPoint *points;
    gint num_points;
};
typedef struct _EekOutline EekOutline;

#define EEK_TYPE_OUTLINE (eek_outline_get_type ())
GType eek_outline_get_type (void) G_GNUC_CONST;

G_END_DECLS
#endif  /* EEK_TYPES_H */
