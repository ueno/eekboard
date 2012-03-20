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
 * SECTION:eek-element
 * @short_description: Base class of a keyboard element
 *
 * The #EekElementClass class represents a keyboard element, which
 * shall be used to implement #EekKeyboard, #EekSection, or #EekKey.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <string.h>

#include "eek-element.h"
#include "eek-container.h"
#include "eek-marshalers.h"

enum {
    PROP_0,
    PROP_NAME,
    PROP_BOUNDS,
    PROP_GROUP,
    PROP_LEVEL,
    PROP_LAST
};

enum {
    SYMBOL_INDEX_CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_ABSTRACT_TYPE (EekElement, eek_element, G_TYPE_OBJECT);

#define EEK_ELEMENT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_ELEMENT, EekElementPrivate))


struct _EekElementPrivate
{
    gchar *name;
    EekBounds bounds;
    EekElement *parent;
    gint group;
    gint level;
};

static void
eek_element_real_symbol_index_changed (EekElement *self,
                                       gint        group,
                                       gint        level)
{
    // g_debug ("symbol-index-changed");
}

static void
eek_element_finalize (GObject *object)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(object);

    g_free (priv->name);
    G_OBJECT_CLASS (eek_element_parent_class)->finalize (object);
}

static void
eek_element_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    EekElement *element = EEK_ELEMENT(object);

    switch (prop_id) {
    case PROP_NAME:
        eek_element_set_name (element,
                              g_value_dup_string (value));
        break;
    case PROP_BOUNDS:
        eek_element_set_bounds (element, g_value_get_boxed (value));
        break;
    case PROP_GROUP:
        eek_element_set_group (element, g_value_get_int (value));
        break;
    case PROP_LEVEL:
        eek_element_set_level (element, g_value_get_int (value));
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
eek_element_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    EekElement *element = EEK_ELEMENT(object);
    EekBounds bounds;

    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, eek_element_get_name (element));
        break;
    case PROP_BOUNDS:
        eek_element_get_bounds (element, &bounds);
        g_value_set_boxed (value, &bounds);
        break;
    case PROP_GROUP:
        g_value_set_int (value, eek_element_get_group (element));
        break;
    case PROP_LEVEL:
        g_value_set_int (value, eek_element_get_level (element));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
eek_element_class_init (EekElementClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekElementPrivate));

    /* signals */
    klass->symbol_index_changed = eek_element_real_symbol_index_changed;

    gobject_class->set_property = eek_element_set_property;
    gobject_class->get_property = eek_element_get_property;
    gobject_class->finalize     = eek_element_finalize;

    /**
     * EekElement:name:
     *
     * The name of #EekElement.
     */
    pspec = g_param_spec_string ("name",
                                 "Name",
                                 "Name",
                                 NULL,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_NAME,
                                     pspec);

    /**
     * EekElement:bounds:
     *
     * The bounding box of #EekElement.
     */
    pspec = g_param_spec_boxed ("bounds",
                                "Bounds",
                                "Bounding box of the element",
                                EEK_TYPE_BOUNDS,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_BOUNDS,
                                     pspec);

    /**
     * EekElement:group:
     *
     * The group value of the symbol index of #EekElement.
     */
    pspec = g_param_spec_int ("group",
                              "Group",
                              "Group value of the symbol index",
                              -1, G_MAXINT, -1,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_GROUP,
                                     pspec);

    /**
     * EekElement:level:
     *
     * The level value of the symbol index of #EekElement.
     */
    pspec = g_param_spec_int ("level",
                              "Level",
                              "Level value of the symbol index",
                              -1, G_MAXINT, -1,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_LEVEL,
                                     pspec);

    /**
     * EekElement::symbol-index-changed:
     * @element: an #EekElement
     * @group: row index of the symbol matrix of keys on @element
     * @level: column index of the symbol matrix of keys on @element
     *
     * The ::symbol-index-changed signal is emitted each time the
     * global configuration of group/level index changes.
     */
    signals[SYMBOL_INDEX_CHANGED] =
        g_signal_new (I_("symbol-index-changed"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekElementClass, symbol_index_changed),
                      NULL,
                      NULL,
                      _eek_marshal_VOID__INT_INT,
                      G_TYPE_NONE,
                      2,
                      G_TYPE_INT,
                      G_TYPE_INT);
}

static void
eek_element_init (EekElement *self)
{
    EekElementPrivate *priv;

    priv = self->priv = EEK_ELEMENT_GET_PRIVATE(self);
    priv->group = -1;
    priv->level = -1;
}

/**
 * eek_element_set_parent:
 * @element: an #EekElement
 * @parent: (allow-none): an #EekElement or %NULL
 *
 * Set the parent of @element to @parent.
 */
void
eek_element_set_parent (EekElement *element,
                        EekElement *parent)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    g_return_if_fail (parent == NULL || EEK_IS_ELEMENT(parent));

    if (element->priv->parent == parent)
        return;

    if (element->priv->parent != NULL) {
        /* release self-reference acquired when setting parent */
        g_object_unref (element);
    }

    if (parent != NULL) {
        g_object_ref (element);
    }

    element->priv->parent = parent;
}

/**
 * eek_element_get_parent:
 * @element: an #EekElement
 *
 * Get the parent of @element.
 * Returns: an #EekElement if the parent is set
 */
EekElement *
eek_element_get_parent (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT(element), NULL);
    return element->priv->parent;
}

/**
 * eek_element_set_name:
 * @element: an #EekElement
 * @name: name of @element
 *
 * Set the name of @element to @name.
 */
void
eek_element_set_name (EekElement  *element,
                      const gchar *name)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    g_free (element->priv->name);
    element->priv->name = g_strdup (name);
}

/**
 * eek_element_get_name:
 * @element: an #EekElement
 *
 * Get the name of @element.
 * Returns: the name of @element or NULL when the name is not set
 */
G_CONST_RETURN gchar *
eek_element_get_name (EekElement  *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT(element), NULL);
    return element->priv->name;
}

/**
 * eek_element_set_bounds:
 * @element: an #EekElement
 * @bounds: bounding box of @element
 *
 * Set the bounding box of @element to @bounds.  Note that if @element
 * has parent, X and Y positions of @bounds are relative to the parent
 * position.
 */
void
eek_element_set_bounds (EekElement  *element,
                        EekBounds   *bounds)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    memcpy (&element->priv->bounds, bounds, sizeof(EekBounds));
}

/**
 * eek_element_get_bounds:
 * @element: an #EekElement
 * @bounds: (out): pointer where bounding box of @element will be stored
 *
 * Get the bounding box of @element.  Note that if @element has
 * parent, position of @bounds are relative to the parent.  To obtain
 * the absolute position, use eek_element_get_absolute_position().
 */
void
eek_element_get_bounds (EekElement  *element,
                        EekBounds   *bounds)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    g_return_if_fail (bounds != NULL);
    memcpy (bounds, &element->priv->bounds, sizeof(EekBounds));
}

/**
 * eek_element_get_absolute_position:
 * @element: an #EekElement
 * @x: pointer where the X coordinate of @element will be stored
 * @y: pointer where the Y coordinate of @element will be stored
 *
 * Compute the absolute position of @element.
 */
void
eek_element_get_absolute_position (EekElement *element,
                                   gdouble    *x,
                                   gdouble    *y)
{
    EekBounds bounds;
    gdouble ax = 0.0, ay = 0.0;

    do {
        eek_element_get_bounds (element, &bounds);
        ax += bounds.x;
        ay += bounds.y;
    } while ((element = eek_element_get_parent (element)) != NULL);
    *x = ax;
    *y = ay;
}

/**
 * eek_element_set_position:
 * @element: an #EekElement
 * @x: X coordinate of top left corner
 * @y: Y coordinate of top left corner
 *
 * Set the relative position of @element.
 */
void
eek_element_set_position (EekElement *element,
                          gdouble     x,
                          gdouble     y)
{
    EekBounds bounds;

    eek_element_get_bounds (element, &bounds);
    bounds.x = x;
    bounds.y = y;
    eek_element_set_bounds (element, &bounds);
}

/**
 * eek_element_set_size:
 * @element: an #EekElement
 * @width: width of @element
 * @height: height of @element
 *
 * Set the size of @element.
 */
void
eek_element_set_size (EekElement  *element,
                      gdouble      width,
                      gdouble      height)
{
    EekBounds bounds;

    eek_element_get_bounds (element, &bounds);
    bounds.width = width;
    bounds.height = height;
    eek_element_set_bounds (element, &bounds);
}

/**
 * eek_element_set_symbol_index:
 * @element: an #EekElement
 * @group: row index of the symbol matrix
 * @level: column index of the symbol matrix
 *
 * Set the default index of the symbol matrices of @element.  The
 * setting affects the child, if child does not have the index set, as
 * well as this element.  To unset, pass -1 as group/level.
 */
void
eek_element_set_symbol_index (EekElement *element,
                              gint        group,
                              gint        level)
{
    gboolean emit_signal;

    g_return_if_fail (EEK_IS_ELEMENT(element));

    emit_signal = group != eek_element_get_group (element) ||
        level != eek_element_get_level (element);

    eek_element_set_group (element, group);
    eek_element_set_level (element, level);

    if (emit_signal)
        g_signal_emit (element, signals[SYMBOL_INDEX_CHANGED], 0, group, level);
}

/**
 * eek_element_get_symbol_index:
 * @element: an #EekElement
 * @group: a pointer where the group value of the symbol index will be stored
 * @level: a pointer where the level value of the symbol index will be stored
 *
 * Get the default index of the symbol matrices of @element.
 * If the index is not set, -1 will be returned.
 */
void
eek_element_get_symbol_index (EekElement *element,
                              gint       *group,
                              gint       *level)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    g_return_if_fail (group != NULL || level != NULL);
    if (group != NULL)
        *group = eek_element_get_group (element);
    if (level != NULL)
        *level = eek_element_get_level (element);
}

/**
 * eek_element_set_group:
 * @element: an #EekElement
 * @group: group index of @element
 *
 * Set the group value of the default symbol index of @element.  To
 * unset, pass -1 as @group.
 *
 * See also: eek_element_set_symbol_index()
 */
void
eek_element_set_group (EekElement *element,
                       gint        group)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    if (element->priv->group != group) {
        element->priv->group = group;
        g_object_notify (G_OBJECT(element), "group");
        g_signal_emit (element, signals[SYMBOL_INDEX_CHANGED], 0,
                       group, element->priv->level);
    }
}

/**
 * eek_element_set_level:
 * @element: an #EekElement
 * @level: level index of @element
 *
 * Set the level value of the default symbol index of @element.  To
 * unset, pass -1 as @level.
 *
 * See also: eek_element_set_symbol_index()
 */
void
eek_element_set_level (EekElement *element,
                       gint        level)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    if (element->priv->level != level) {
        element->priv->level = level;
        g_object_notify (G_OBJECT(element), "level");
        g_signal_emit (element, signals[SYMBOL_INDEX_CHANGED], 0,
                       element->priv->group, level);
    }
}

/**
 * eek_element_get_group:
 * @element: an #EekElement
 *
 * Return the group value of the default symbol index of @element.
 * If the value is not set, -1 will be returned.
 *
 * See also: eek_element_get_symbol_index()
 */
gint
eek_element_get_group (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT(element), -1);
    return element->priv->group;
}

/**
 * eek_element_get_level:
 * @element: an #EekElement
 *
 * Return the level value of the default symbol index of @element.
 * If the value is not set, -1 will be returned.
 *
 * See also: eek_element_get_symbol_index()
 */
gint
eek_element_get_level (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT(element), -1);
    return element->priv->level;
}
