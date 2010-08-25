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

/**
 * EekOrientation:
 * @EEK_ORIENTATION_VERTICAL: the elements will be arranged vertically
 * @EEK_ORIENTATION_HORIZONTAL: the elements will be arranged horizontally
 * @EEK_ORIENTATION_INVALID: used for error reporting
 *
 * Orientation of rows in sections.  Elements in a row will be
 * arranged with the specified orientation.
 */
typedef enum {
    EEK_ORIENTATION_VERTICAL,
    EEK_ORIENTATION_HORIZONTAL,
    EEK_ORIENTATION_INVALID = -1
} EekOrientation;

typedef struct _EekElement EekElement;
typedef struct _EekContainer EekContainer;
typedef struct _EekKey EekKey;
typedef struct _EekSection EekSection;
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
void  eek_point_rotate   (EekPoint *point,
                          gint      angle);

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

G_INLINE_FUNC gdouble
eek_bounds_long_side (EekBounds *bounds)
{
    return bounds->width > bounds->height ? bounds->width : bounds->height;
}

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

/**
 * EekColor:
 * @red: red component of the RGB value
 * @green: green component of the RGB value
 * @blue: blue component of the RGB value
 * @alpha: alpha component of the RGB value
 *
 * RGB color value
 */
struct _EekColor
{
    guint8 red;
    guint8 green;
    guint8 blue;
    guint8 alpha;
};
typedef struct _EekColor EekColor;

#define EEK_TYPE_COLOR (eek_color_get_type ())
GType eek_color_get_type (void) G_GNUC_CONST;

typedef enum {
  EEK_GRADIENT_NONE,
  EEK_GRADIENT_VERTICAL,
  EEK_GRADIENT_HORIZONTAL,
  EEK_GRADIENT_RADIAL
} EekGradientType;

G_END_DECLS
#endif  /* EEK_TYPES_H */
