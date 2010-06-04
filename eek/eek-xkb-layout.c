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

/**
 * SECTION:eek-xkb-layout
 * @short_description: Layout engine using XKB configuration
 *
 * The #EekXkbLayout inherits #EekLayout class and arranges keyboard
 * elements using XKB.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <gdk/gdkx.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBgeom.h>
#include <string.h>
#include "eek-xkb-layout.h"
#include "eek-keyboard.h"

#define noKBDRAW_DEBUG

G_DEFINE_TYPE (EekXkbLayout, eek_xkb_layout, EEK_TYPE_LAYOUT);

#define EEK_XKB_LAYOUT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_XKB_LAYOUT, EekXkbLayoutPrivate))

enum {
    PROP_0,
    PROP_KEYCODES,
    PROP_GEOMETRY,
    PROP_SYMBOLS,
    PROP_LAST
};

struct _EekXkbLayoutPrivate
{
    /* Configuration names that should synch'ed to the symbolic names
       in priv->xkb->names.  Since we use GLib's memory allocator,
       don't store any address returned from the X server here. */
    XkbComponentNamesRec names;

    Display *display;

    /* Actual XKB configuration of DISPLAY. */
    XkbDescRec *xkb;

    /* Hash table to cache outlines by shape address. */
    GHashTable *outline_hash;

    gint scale_numerator;
    gint scale_denominator;
};

#define INVALID_KEYCODE ((guint)(-1))

static guint
find_keycode (EekXkbLayout *layout, gchar *key_name);

static void
get_keyboard (EekXkbLayout *layout);

static void
get_names (EekXkbLayout *layout);

static void
setup_scaling (EekXkbLayout *layout,
               gdouble       width,
               gdouble       height);

G_INLINE_FUNC gint
xkb_to_pixmap_coord (EekXkbLayout *layout,
                     gint          n)
{
    EekXkbLayoutPrivate *priv = layout->priv;
    return n * priv->scale_numerator / priv->scale_denominator;
}

G_INLINE_FUNC gdouble
xkb_to_pixmap_double (EekXkbLayout *layout,
                     gdouble       d)
{
    EekXkbLayoutPrivate *priv = layout->priv;
    return d * priv->scale_numerator / priv->scale_denominator;
}

static EekKey *
create_key (EekXkbLayout *layout,
            EekSection   *section,
            gint          column,
            gint          row,
            gdouble       x,
            gdouble       y,
            XkbKeyRec    *xkbkey)
{
    XkbGeometryRec *xkbgeometry;
    XkbBoundsRec *xkbbounds;
    XkbShapeRec *xkbshape;
    XkbOutlineRec *xkboutline;
    EekXkbLayoutPrivate *priv = layout->priv;
    EekKey *key;
    EekBounds bounds;
    guint *keysyms;
    gchar name[XkbKeyNameLength + 1];
    EekOutline *outline;
    KeyCode keycode;
    gint num_groups, num_levels;

    xkbgeometry = priv->xkb->geom;
    xkbshape = &xkbgeometry->shapes[xkbkey->shape_ndx];
    outline = g_hash_table_lookup (priv->outline_hash, xkbshape);
    if (outline == NULL) {
        xkboutline = xkbshape->primary == NULL ? &xkbshape->outlines[0] :
            xkbshape->primary;

        outline = g_new0 (EekOutline, 1);
        outline->corner_radius = xkb_to_pixmap_coord(layout, xkboutline->corner_radius);

        if (xkboutline->num_points <= 2) { /* rectangular */
            gdouble x1, y1, x2, y2;

            outline->num_points = 4;
            outline->points = g_new0 (EekPoint, outline->num_points);
            if (xkboutline->num_points == 1) {
                x1 = xkb_to_pixmap_coord(layout, xkbshape->bounds.x1);
                y1 = xkb_to_pixmap_coord(layout, xkbshape->bounds.y1);
                x2 = xkb_to_pixmap_coord(layout, xkboutline->points[0].x);
                y2 = xkb_to_pixmap_coord(layout, xkboutline->points[0].y);
            } else {
                x1 = xkb_to_pixmap_coord(layout, xkboutline->points[0].x);
                y1 = xkb_to_pixmap_coord(layout, xkboutline->points[0].y);
                x2 = xkb_to_pixmap_coord(layout, xkboutline->points[1].x);
                y2 = xkb_to_pixmap_coord(layout, xkboutline->points[1].y);
            }
            outline->points[0].x = outline->points[3].x = x1;
            outline->points[0].y = outline->points[1].y = y1;
            outline->points[1].x = outline->points[2].x = x2;
            outline->points[2].y = outline->points[3].y = y2;
        } else {                /* polygon */
            gint i;

            outline->num_points = xkboutline->num_points;
            outline->points = g_new0 (EekPoint, outline->num_points);
            for (i = 0; i < xkboutline->num_points; i++) {
                outline->points[i].x = xkb_to_pixmap_coord(layout, xkboutline->points[i].x);
                outline->points[i].y = xkb_to_pixmap_coord(layout, xkboutline->points[i].y);
            }
        }
        g_hash_table_insert (priv->outline_hash, xkbshape, outline);
    }

    memset (name, 0, sizeof name);
    memcpy (name, xkbkey->name.name, sizeof name - 1);

    xkbbounds = &xkbgeometry->shapes[xkbkey->shape_ndx].bounds;
    bounds.x = xkb_to_pixmap_coord(layout, xkbbounds->x1 + x);
    bounds.y = xkb_to_pixmap_coord(layout, xkbbounds->y1 + y);
    bounds.width = xkb_to_pixmap_coord(layout, xkbbounds->x2 - xkbbounds->x1);
    bounds.height = xkb_to_pixmap_coord(layout, xkbbounds->y2 - xkbbounds->y1);

    keycode = find_keycode (layout, name);
    if (keycode == INVALID_KEYCODE)
        num_groups = num_levels = 0;
    else {
        KeySym keysym;
        gint num_keysyms, i, j;

        num_groups = XkbKeyNumGroups (priv->xkb, keycode);
        num_levels = XkbKeyGroupsWidth (priv->xkb, keycode);
        num_keysyms = num_groups * num_levels;
        keysyms = g_malloc0 ((num_keysyms) * sizeof(guint));
        for (i = 0; i < num_groups; i++)
            for (j = 0; j < num_levels; j++) {
                keysym = XkbKeySymEntry (priv->xkb, keycode, j, i);
                keysyms[i * num_levels + j] = keysym;
            }
    }

    eek_section_create_key (section,
                            name,
                            keysyms,
                            num_groups,
                            num_levels,
                            column,
                            row,
                            outline,
                            &bounds);
}

static void
create_section (EekXkbLayout  *layout,
                EekKeyboard   *keyboard,
                XkbSectionRec *xkbsection)
{
    XkbGeometryRec *xkbgeometry;
    EekXkbLayoutPrivate *priv;
    EekSection *section;
    EekBounds bounds;
    const gchar *name;
    gfloat left, top;
    gint i, j, columns;

    bounds.x = xkb_to_pixmap_coord(layout, xkbsection->left);
    bounds.y = xkb_to_pixmap_coord(layout, xkbsection->top);
    bounds.width = xkb_to_pixmap_coord(layout, xkbsection->width);
    bounds.height = xkb_to_pixmap_coord(layout, xkbsection->height);

    priv = layout->priv;
    xkbgeometry = priv->xkb->geom;
    name = XGetAtomName (priv->display, xkbsection->name);
    section = eek_keyboard_create_section (keyboard,
                                           name,
                                           /* angle is in tenth of degree */
                                           xkbsection->angle / 10,
                                           &bounds);

    for (columns = 0, i = 0; i < xkbsection->num_rows; i++) {
        XkbRowRec *xkbrow;

        xkbrow = &xkbsection->rows[i];
        if (xkbrow->num_keys > columns)
            columns = xkbrow->num_keys;
    }
    eek_section_set_dimensions (section, columns, xkbsection->num_rows);

    for (i = 0; i < xkbsection->num_rows; i++) {
        XkbRowRec *xkbrow;

        xkbrow = &xkbsection->rows[i];
        left = xkbrow->left;
        top = xkbrow->top;
        for (j = 0; j < xkbrow->num_keys; j++) {
            XkbKeyRec *xkbkey;
            XkbBoundsRec *xkbbounds;

            xkbkey = &xkbrow->keys[j];
            if (xkbrow->vertical)
                top += xkbkey->gap;
            else
                left += xkbkey->gap;
            create_key (layout, section, j, i, left, top, xkbkey);
            xkbbounds = &xkbgeometry->shapes[xkbkey->shape_ndx].bounds;
            if (xkbrow->vertical)
                top += xkbbounds->y2 - xkbbounds->y1;
            else
                left += xkbbounds->x2 - xkbbounds->x1;
        }
    }
}

static void
create_keyboard (EekXkbLayout *layout, EekKeyboard *keyboard)
{
    EekXkbLayoutPrivate *priv = layout->priv;
    XkbGeometryRec *xkbgeometry;
    EekBounds bounds;
    gint i;

    g_return_if_fail (priv->xkb);
    g_return_if_fail (priv->xkb->geom);

    xkbgeometry = priv->xkb->geom;

    eek_keyboard_get_bounds (keyboard, &bounds);
    setup_scaling (EEK_XKB_LAYOUT(layout), bounds.width, bounds.height);

    bounds.x = bounds.y = 0;
    bounds.width = xkb_to_pixmap_coord(layout, xkbgeometry->width_mm);
    bounds.height = xkb_to_pixmap_coord(layout, xkbgeometry->height_mm);
    eek_keyboard_set_bounds (keyboard, &bounds);

    for (i = 0; i < xkbgeometry->num_sections; i++) {
        XkbSectionRec *xkbsection;
        EekSection *section;

        xkbsection = &xkbgeometry->sections[i];
        create_section (layout, keyboard, xkbsection);
    }
}

static void
eek_xkb_layout_apply_to_keyboard (EekLayout *layout, EekKeyboard *keyboard)
{
    g_return_if_fail (EEK_IS_XKB_LAYOUT(layout));
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));
    create_keyboard (EEK_XKB_LAYOUT(layout), keyboard);
    if (g_object_is_floating (keyboard))
        g_object_unref (keyboard);
}

static void
eek_xkb_layout_dispose (GObject *object)
{
    G_OBJECT_CLASS (eek_xkb_layout_parent_class)->dispose (object);
}

static void
eek_xkb_layout_finalize (GObject *object)
{
    EekXkbLayoutPrivate *priv = EEK_XKB_LAYOUT_GET_PRIVATE (object);

    g_free (priv->names.keycodes);
    g_free (priv->names.geometry);
    g_free (priv->names.symbols);
    g_hash_table_unref (priv->outline_hash);
    XkbFreeKeyboard (priv->xkb, 0, TRUE);	/* free_all = TRUE */
    G_OBJECT_CLASS (eek_xkb_layout_parent_class)->finalize (object);
}

static void 
eek_xkb_layout_set_property (GObject      *object, 
                               guint         prop_id,
                               const GValue *value, 
                               GParamSpec   *pspec)
{
    const gchar *name;

    switch (prop_id) 
        {
        case PROP_KEYCODES:
            name = g_value_get_string (value);
            eek_xkb_layout_set_keycodes (EEK_XKB_LAYOUT(object), name);
            break;
        case PROP_GEOMETRY:
            name = g_value_get_string (value);
            eek_xkb_layout_set_geometry (EEK_XKB_LAYOUT(object), name);
            break;
        case PROP_SYMBOLS:
            name = g_value_get_string (value);
            eek_xkb_layout_set_symbols (EEK_XKB_LAYOUT(object), name);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
        }
}

static void 
eek_xkb_layout_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    const gchar *name;

    switch (prop_id) 
        {
        case PROP_KEYCODES:
            name = eek_xkb_layout_get_keycodes (EEK_XKB_LAYOUT(object));
            g_value_set_string (value, name);
            break;
        case PROP_GEOMETRY:
            name = eek_xkb_layout_get_geometry (EEK_XKB_LAYOUT(object));
            g_value_set_string (value, name);
            break;
        case PROP_SYMBOLS:
            name = eek_xkb_layout_get_symbols (EEK_XKB_LAYOUT(object));
            g_value_set_string (value, name);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
        }
}

static void
eek_xkb_layout_class_init (EekXkbLayoutClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;
    EekLayoutClass    *layout_class = EEK_LAYOUT_CLASS (klass);

    g_type_class_add_private (gobject_class, sizeof (EekXkbLayoutPrivate));

    gobject_class->finalize     = eek_xkb_layout_finalize;
    gobject_class->dispose      = eek_xkb_layout_dispose;
    gobject_class->set_property = eek_xkb_layout_set_property;
    gobject_class->get_property = eek_xkb_layout_get_property;

    layout_class->apply_to_keyboard = eek_xkb_layout_apply_to_keyboard;

    pspec = g_param_spec_string ("keycodes",
				 "Keycodes",
				 "XKB keycodes component name",
				 NULL,
				 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_KEYCODES, pspec);

    pspec = g_param_spec_string ("geometry",
                                 "Geometry",
                                 "XKB geometry component name",
                                 NULL,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GEOMETRY, pspec);

    pspec = g_param_spec_string ("symbols",
                                 "Symbols",
                                 "XKB symbols component name",
                                 NULL,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_SYMBOLS, pspec);
}

static void
eek_xkb_layout_init (EekXkbLayout *self)
{
    EekXkbLayoutPrivate *priv;

    priv = self->priv = EEK_XKB_LAYOUT_GET_PRIVATE (self);

    memset (&priv->names, 0, sizeof priv->names);
    priv->display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());

    /* XXX: XkbClientMapMask | XkbIndicatorMapMask | XkbNamesMask |
       XkbGeometryMask */
    priv->xkb = XkbGetKeyboard (priv->display,
                                XkbGBN_GeometryMask |
                                XkbGBN_KeyNamesMask |
                                XkbGBN_OtherNamesMask |
                                XkbGBN_SymbolsMask |
                                XkbGBN_IndicatorMapMask,
                                XkbUseCoreKbd);

    priv->outline_hash = g_hash_table_new_full (g_direct_hash,
                                                g_direct_equal,
                                                NULL,
                                                g_free);
    if (priv->xkb == NULL) {
        g_critical ("XkbGetKeyboard failed to get keyboard from the server!");
        return;
    }
    get_names (self);
}

static void
get_names (EekXkbLayout *layout)
{
    EekXkbLayoutPrivate *priv = layout->priv;
    gchar *name;

    XkbGetNames (priv->display, XkbAllNamesMask, priv->xkb);

    if (priv->xkb->names->keycodes <= 0)
        g_warning ("XKB keycodes setting is not loaded properly");
    else {
        name = XGetAtomName (priv->display, priv->xkb->names->keycodes);
        if (!name)
            g_warning ("Can't get the name of keycodes");
        else if (!priv->names.keycodes ||
                 g_strcmp0 (name, priv->names.keycodes)) {
            g_free (priv->names.keycodes);
            priv->names.keycodes = g_strdup (name);
            XFree (name);
        }
    }

    if (priv->xkb->names->geometry <= 0)
        g_warning ("XKB geometry setting is not loaded");
    else {
        name = XGetAtomName (priv->display, priv->xkb->names->geometry);
        if (!name)
            g_warning ("Can't get the name of geometry");
        else if (!priv->names.geometry ||
                 g_strcmp0 (name, priv->names.geometry)) {
            g_free (priv->names.geometry);
            priv->names.geometry = g_strdup (name);
            XFree (name);
        }
    }

    if (priv->xkb->names->symbols <= 0)
        g_warning ("XKB symbols setting is not loaded");
    else {
        name = XGetAtomName (priv->display, priv->xkb->names->symbols);
        if (!name)
            g_warning ("Can't get the name of symbols");
        else if (!priv->names.symbols ||
                 g_strcmp0 (name, priv->names.symbols)) {
            g_free (priv->names.symbols);
            priv->names.symbols = g_strdup (name);
            XFree (name);
        }
    }
}

/**
 * eek_xkb_layout_new:
 * @keycodes: component name for keycodes
 * @geometry: component name for geometry
 * @symbols: component name for symbols
 *
 * Create a new #EekXkbLayout.
 */
EekLayout *
eek_xkb_layout_new (const gchar *keycodes,
                    const gchar *geometry,
                    const gchar *symbols)
{
    EekXkbLayout *layout = g_object_new (EEK_TYPE_XKB_LAYOUT, NULL);
    EekXkbLayoutPrivate *priv = layout->priv;

    g_return_val_if_fail (layout, NULL);
    if (keycodes)
        priv->names.keycodes = g_strdup (keycodes);
    if (geometry)
        priv->names.geometry = g_strdup (geometry);
    if (symbols)
        priv->names.symbols = g_strdup (symbols);

    get_keyboard (layout);
    return EEK_LAYOUT(layout);
}

/**
 * eek_xkb_layout_set_keycodes:
 * @layout: an #EekXkbLayout
 * @keycodes: component name for keycodes
 *
 * Set the keycodes component (in the XKB terminology).
 */
void
eek_xkb_layout_set_keycodes (EekXkbLayout *layout, const gchar *keycodes)
{
    EekXkbLayoutPrivate *priv = layout->priv;

    g_free (priv->names.keycodes);
    priv->names.keycodes = g_strdup (keycodes);
    get_keyboard (layout);
}

/**
 * eek_xkb_layout_set_geometry:
 * @layout: an #EekXkbLayout
 * @geometry: component name for geometry
 *
 * Set the keycodes component (in the XKB terminology).
 */
void
eek_xkb_layout_set_geometry (EekXkbLayout *layout, const gchar *geometry)
{
    EekXkbLayoutPrivate *priv = layout->priv;

    g_free (priv->names.geometry);
    priv->names.geometry = g_strdup (geometry);
    get_keyboard (layout);
}

/**
 * eek_xkb_layout_set_symbols:
 * @layout: an #EekXkbLayout
 * @symbols: component name for symbols
 *
 * Set the symbols component (in the XKB terminology).
 */
void
eek_xkb_layout_set_symbols (EekXkbLayout *layout, const gchar *symbols)
{
    EekXkbLayoutPrivate *priv = layout->priv;

    g_free (priv->names.symbols);
    priv->names.symbols = g_strdup (symbols);
    get_keyboard (layout);
}

/**
 * eek_xkb_layout_get_keycodes:
 * @layout: an #EekXkbLayout
 *
 * Get the keycodes component name (in the XKB terminology).
 */
G_CONST_RETURN gchar *
eek_xkb_layout_get_keycodes (EekXkbLayout *layout)
{
    EekXkbLayoutPrivate *priv = layout->priv;

    return priv->names.keycodes;
}

/**
 * eek_xkb_layout_get_geometry:
 * @layout: an #EekXkbLayout
 *
 * Get the geometry component name (in the XKB terminology).
 */
G_CONST_RETURN gchar *
eek_xkb_layout_get_geometry (EekXkbLayout *layout)
{
    EekXkbLayoutPrivate *priv = layout->priv;
    char *name;

    return priv->names.geometry;
}

/**
 * eek_xkb_layout_get_symbols:
 * @layout: an #EekXkbLayout
 *
 * Get the symbols component name (in the XKB terminology).
 */
G_CONST_RETURN gchar *
eek_xkb_layout_get_symbols (EekXkbLayout *layout)
{
    EekXkbLayoutPrivate *priv = layout->priv;

    return priv->names.symbols;
}

static void
get_keyboard (EekXkbLayout *layout)
{
    EekXkbLayoutPrivate *priv = layout->priv;

    if (priv->xkb)
        XkbFreeKeyboard (priv->xkb, 0, TRUE);	/* free_all = TRUE */
    priv->xkb = NULL;

    if (priv->names.keycodes &&
        priv->names.geometry &&
        priv->names.symbols) {
        priv->xkb = XkbGetKeyboardByName (priv->display, XkbUseCoreKbd,
                                          &priv->names, 0,
                                          XkbGBN_GeometryMask |
                                          XkbGBN_KeyNamesMask |
                                          XkbGBN_OtherNamesMask |
                                          XkbGBN_ClientSymbolsMask |
                                          XkbGBN_IndicatorMapMask, FALSE);
    } else {
        priv->xkb = XkbGetKeyboard (priv->display,
                                    XkbGBN_GeometryMask |
                                    XkbGBN_KeyNamesMask |
                                    XkbGBN_OtherNamesMask |
                                    XkbGBN_SymbolsMask |
                                    XkbGBN_IndicatorMapMask,
                                    XkbUseCoreKbd);
        get_names (layout);
    }

    if (priv->xkb == NULL) {
        g_free (priv->names.keycodes);
        priv->names.keycodes = NULL;
        g_free (priv->names.geometry);
        priv->names.geometry = NULL;
        g_free (priv->names.symbols);
        priv->names.symbols = NULL;
    }
}


static guint
find_keycode (EekXkbLayout *layout, gchar *key_name)
{
#define KEYSYM_NAME_MAX_LENGTH 4
    guint keycode;
    gint i, j;
    XkbKeyNamePtr pkey;
    XkbKeyAliasPtr palias;
    guint is_name_matched;
    gchar *src, *dst;
    EekXkbLayoutPrivate *priv = layout->priv;

    if (!priv->xkb)
        return INVALID_KEYCODE;

#ifdef KBDRAW_DEBUG
    printf ("    looking for keycode for (%c%c%c%c)\n",
            key_name[0], key_name[1], key_name[2], key_name[3]);
#endif

    pkey = priv->xkb->names->keys + priv->xkb->min_key_code;
    for (keycode = priv->xkb->min_key_code;
         keycode <= priv->xkb->max_key_code; keycode++) {
        is_name_matched = 1;
        src = key_name;
        dst = pkey->name;
        for (i = KEYSYM_NAME_MAX_LENGTH; --i >= 0;) {
            if ('\0' == *src)
                break;
            if (*src++ != *dst++) {
                is_name_matched = 0;
                break;
            }
        }
        if (is_name_matched) {
#ifdef KBDRAW_DEBUG
            printf ("      found keycode %u\n", keycode);
#endif
            return keycode;
        }
        pkey++;
    }

    palias = priv->xkb->names->key_aliases;
    for (j = priv->xkb->names->num_key_aliases; --j >= 0;) {
        is_name_matched = 1;
        src = key_name;
        dst = palias->alias;
        for (i = KEYSYM_NAME_MAX_LENGTH; --i >= 0;) {
            if ('\0' == *src)
                break;
            if (*src++ != *dst++) {
                is_name_matched = 0;
                break;
            }
        }

        if (is_name_matched) {
            keycode = find_keycode (layout, palias->real);
#ifdef KBDRAW_DEBUG
            printf ("found alias keycode %u\n", keycode);
#endif
            return keycode;
        }
        palias++;
    }

    return INVALID_KEYCODE;
}

static void
setup_scaling (EekXkbLayout *layout,
               gdouble       width,
               gdouble       height)
{
    EekXkbLayoutPrivate *priv = layout->priv;

    g_return_if_fail (priv->xkb);

    g_return_if_fail (priv->xkb->geom->width_mm > 0);
    g_return_if_fail (priv->xkb->geom->height_mm > 0);

    if (width * priv->xkb->geom->height_mm <
        height * priv->xkb->geom->width_mm) {
        priv->scale_numerator = width;
        priv->scale_denominator = priv->xkb->geom->width_mm;
    } else {
        priv->scale_numerator = height;
        priv->scale_denominator = priv->xkb->geom->height_mm;
    }
}
