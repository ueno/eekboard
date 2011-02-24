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
#ifndef EEK_TYPES_H
#define EEK_TYPES_H 1

#include <glib-object.h>

G_BEGIN_DECLS

#define I_(string) g_intern_static_string (string)

#define EEK_TYPE_SYMBOL_MATRIX (eek_symbol_matrix_get_type ())
#define EEK_TYPE_POINT (eek_point_get_type ())
#define EEK_TYPE_BOUNDS (eek_bounds_get_type ())
#define EEK_TYPE_OUTLINE (eek_outline_get_type ())
#define EEK_TYPE_COLOR (eek_color_get_type ())


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

/**
 * EekModifierBehavior:
 * @EEK_MODIFIER_BEHAVIOR_NONE: do nothing when a modifier key is pressed
 * @EEK_MODIFIER_BEHAVIOR_LOCK: toggle the modifier status each time a
 * modifier key are pressed
 * @EEK_MODIFIER_BEHAVIOR_LATCH: enable the modifier when a modifier
 * key is pressed and keep it enabled until any key is pressed.
 *
 * Modifier handling mode.
 */
typedef enum {
    EEK_MODIFIER_BEHAVIOR_NONE,
    EEK_MODIFIER_BEHAVIOR_LOCK,
    EEK_MODIFIER_BEHAVIOR_LATCH
} EekModifierBehavior;

/**
 * EekModifierType:
 * @EEK_SHIFT_MASK: the Shift key.
 * @EEK_LOCK_MASK: a Lock key (depending on the modifier mapping of the
 *  X server this may either be CapsLock or ShiftLock).
 * @EEK_CONTROL_MASK: the Control key.
 * @EEK_MOD1_MASK: the fourth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier, but
 *  normally it is the Alt key).
 * @EEK_MOD2_MASK: the fifth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @EEK_MOD3_MASK: the sixth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @EEK_MOD4_MASK: the seventh modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @EEK_MOD5_MASK: the eighth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @EEK_BUTTON1_MASK: the first mouse button.
 * @EEK_BUTTON2_MASK: the second mouse button.
 * @EEK_BUTTON3_MASK: the third mouse button.
 * @EEK_BUTTON4_MASK: the fourth mouse button.
 * @EEK_BUTTON5_MASK: the fifth mouse button.
 * @EEK_SUPER_MASK: the Super modifier. Since 2.10
 * @EEK_HYPER_MASK: the Hyper modifier. Since 2.10
 * @EEK_META_MASK: the Meta modifier. Since 2.10
 * @EEK_RELEASE_MASK: not used in EEK itself. GTK+ uses it to differentiate
 *  between (keyval, modifiers) pairs from key press and release events.
 * @EEK_MODIFIER_MASK: a mask covering all modifier types.
 */
typedef enum
{
  EEK_SHIFT_MASK    = 1 << 0,
  EEK_LOCK_MASK	    = 1 << 1,
  EEK_CONTROL_MASK  = 1 << 2,
  EEK_MOD1_MASK	    = 1 << 3,
  EEK_MOD2_MASK	    = 1 << 4,
  EEK_MOD3_MASK	    = 1 << 5,
  EEK_MOD4_MASK	    = 1 << 6,
  EEK_MOD5_MASK	    = 1 << 7,
  EEK_BUTTON1_MASK  = 1 << 8,
  EEK_BUTTON2_MASK  = 1 << 9,
  EEK_BUTTON3_MASK  = 1 << 10,
  EEK_BUTTON4_MASK  = 1 << 11,
  EEK_BUTTON5_MASK  = 1 << 12,

  /* The next few modifiers are used by XKB, so we skip to the end.
   * Bits 15 - 25 are currently unused. Bit 29 is used internally.
   */
  
  EEK_SUPER_MASK    = 1 << 26,
  EEK_HYPER_MASK    = 1 << 27,
  EEK_META_MASK     = 1 << 28,
  
  EEK_RELEASE_MASK  = 1 << 30,

  EEK_MODIFIER_MASK = 0x5c001fff
} EekModifierType;

/**
 * EEK_INVALID_KEYCODE:
 *
 * Pseudo keycode used for error reporting.
 */
#define EEK_INVALID_KEYCODE (0)
    
typedef struct _EekElement EekElement;
typedef struct _EekContainer EekContainer;
typedef struct _EekKey EekKey;
typedef struct _EekSection EekSection;
typedef struct _EekKeyboard EekKeyboard;
typedef struct _EekSymbol EekSymbol;
typedef struct _EekKeysym EekKeysym;

typedef struct _EekSymbolMatrix EekSymbolMatrix;
typedef struct _EekPoint EekPoint;
typedef struct _EekBounds EekBounds;
typedef struct _EekOutline EekOutline;
typedef struct _EekColor EekColor;

/**
 * EekSymbolMatrix:
 * @data: array of symbols
 * @num_groups: the number of groups (rows)
 * @num_levels: the number of levels (columns)
 *
 * Symbol matrix of a key.
 */
struct _EekSymbolMatrix
{
    gint num_groups;
    gint num_levels;
    EekSymbol **data;
};

GType             eek_symbol_matrix_get_type
                                         (void) G_GNUC_CONST;
EekSymbolMatrix * eek_symbol_matrix_new  (gint                   num_groups,
                                          gint                   num_levels);
EekSymbolMatrix  *eek_symbol_matrix_copy (const EekSymbolMatrix *matrix);
void              eek_symbol_matrix_free (EekSymbolMatrix       *matrix);

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
 * The rectangle containing an element's bounding box.
 */
struct _EekBounds
{
    /*< public >*/
    gdouble x;
    gdouble y;
    gdouble width;
    gdouble height;
};

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

GType       eek_outline_get_type (void) G_GNUC_CONST;
EekOutline *eek_outline_copy     (const EekOutline *outline);
void        eek_outline_free     (EekOutline       *outline);

/**
 * EekColor:
 * @red: red component of color, between 0.0 and 1.0
 * @green: green component of color, between 0.0 and 1.0
 * @blue: blue component of color, between 0.0 and 1.0
 * @alpha: alpha component of color, between 0.0 and 1.0
 *
 * Color used for drawing.
 */
struct _EekColor
{
    gdouble red;
    gdouble green;
    gdouble blue;
    gdouble alpha;
};

GType     eek_color_get_type (void) G_GNUC_CONST;

EekColor *eek_color_new      (gdouble red,
                              gdouble green,
                              gdouble blue,
                              gdouble alpha);

G_END_DECLS
#endif  /* EEK_TYPES_H */
