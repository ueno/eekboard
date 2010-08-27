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
 * SECTION:eek-element
 * @short_description: Base class of a keyboard element
 *
 * The #EekElementClass class represents a keyboard element, which
 * shall be used to implement #EekKeyboard, #EekSection, or #EekKey.
 */

#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-element.h"
#include "eek-container.h"
#include "eek-theme-node-private.h"

enum {
    PROP_0,
    PROP_NAME,
    PROP_BOUNDS,
    PROP_THEME,
    PROP_PSEUDO_CLASS,
    PROP_STYLE_CLASS,
    PROP_STYLE,
    PROP_LAST
};

enum
{
  STYLE_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_ABSTRACT_TYPE (EekElement, eek_element, G_TYPE_INITIALLY_UNOWNED);

#define EEK_ELEMENT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_ELEMENT, EekElementPrivate))


struct _EekElementPrivate
{
    gchar *name;
    EekBounds bounds;
    EekElement *parent;
    EekTheme *theme;
    EekThemeNode *theme_node;
    gchar *pseudo_class;
    gchar *style_class;
    gchar *inline_style;
};

static void
eek_element_real_set_parent (EekElement *self,
                             EekElement *parent)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    if (!parent) {
        g_return_if_fail (priv->parent);
        /* release self-reference acquired when setting parent */
        g_object_unref (self);
        priv->parent = NULL;
    } else {
        g_return_if_fail (!priv->parent);
        g_object_ref_sink (self);
        priv->parent = parent;
    }
}

static EekElement *
eek_element_real_get_parent (EekElement *self)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    return priv->parent;
}

static void
eek_element_real_set_name (EekElement  *self,
                           const gchar *name)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    g_free (priv->name);
    priv->name = g_strdup (name);

    g_object_notify (G_OBJECT(self), "name");
}

static G_CONST_RETURN gchar *
eek_element_real_get_name (EekElement *self)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    return priv->name;
}

static void
eek_element_real_set_bounds (EekElement *self,
                             EekBounds *bounds)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    priv->bounds = *bounds;

    g_object_notify (G_OBJECT(self), "bounds");
}

static void
eek_element_real_get_bounds (EekElement *self,
                             EekBounds  *bounds)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    g_return_if_fail (bounds);
    *bounds = priv->bounds;
}

static void
eek_element_real_set_theme (EekElement *self,
                            EekTheme   *theme)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    if (theme != priv->theme) {
        if (priv->theme)
            g_object_unref (priv->theme);
        priv->theme = g_object_ref (theme);

        eek_element_style_changed (self);

        g_object_notify (G_OBJECT(self), "theme");
    }
}

static EekTheme *
eek_element_real_get_theme (EekElement *self)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    return priv->theme;
}

static EekThemeNode *
eek_element_real_get_theme_node (EekElement *self)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(self);

    if (priv->theme_node == NULL) {
        EekThemeNode *parent_node = NULL;
        EekElement *parent;

        parent = eek_element_get_parent (self);
        if (parent)
            parent_node = eek_element_get_theme_node (EEK_ELEMENT(parent));
        priv->theme_node = eek_theme_node_new (parent_node,
                                               priv->theme,
                                               G_OBJECT_TYPE(self),
                                               eek_element_get_name (self),
                                               priv->style_class,
                                               priv->pseudo_class,
                                               priv->inline_style);
    }
        
    return priv->theme_node;
}

static void
eek_element_dispose (GObject *object)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(object);

    if (priv->theme) {
        g_object_unref (priv->theme);
        priv->theme = NULL;
    }
    if (priv->theme_node) {
        g_object_unref (priv->theme_node);
        priv->theme_node = NULL;
    }
}

static void
eek_element_finalize (GObject *object)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(object);

    g_free (priv->name);
    g_free (priv->style_class);
    g_free (priv->pseudo_class);

    G_OBJECT_CLASS (eek_element_parent_class)->finalize (object);
}

static void
eek_element_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    g_return_if_fail (EEK_IS_ELEMENT(object));
    switch (prop_id) {
    case PROP_NAME:
        eek_element_set_name (EEK_ELEMENT(object),
                              g_value_get_string (value));
        break;
    case PROP_BOUNDS:
        eek_element_set_bounds (EEK_ELEMENT(object),
                                g_value_get_boxed (value));
        break;
    case PROP_THEME:
        eek_element_set_theme (EEK_ELEMENT(object),
                               g_value_get_object (value));
        break;
    case PROP_PSEUDO_CLASS:
        eek_element_set_style_pseudo_class (EEK_ELEMENT(object),
                                            g_value_get_string (value));
        break;
    case PROP_STYLE_CLASS:
        eek_element_set_style_class_name (EEK_ELEMENT(object),
                                          g_value_get_string (value));
        break;
    case PROP_STYLE:
        eek_element_set_style (EEK_ELEMENT(object), g_value_get_string (value));
        break;
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
    EekBounds  bounds;
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(object);

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, eek_element_get_name (EEK_ELEMENT(object)));
        break;
    case PROP_BOUNDS:
        eek_element_get_bounds (EEK_ELEMENT(object), &bounds);
        g_value_set_boxed (value, &bounds);
        break;
    case PROP_THEME:
        g_value_set_object (value,
                            eek_element_get_theme (EEK_ELEMENT(object)));
        break;
    case PROP_PSEUDO_CLASS:
        g_value_set_string (value, priv->pseudo_class);
        break;
    case PROP_STYLE_CLASS:
        g_value_set_string (value, priv->style_class);
        break;
    case PROP_STYLE:
        g_value_set_string (value, priv->inline_style);
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

    klass->set_parent = eek_element_real_set_parent;
    klass->get_parent = eek_element_real_get_parent;
    klass->set_name = eek_element_real_set_name;
    klass->get_name = eek_element_real_get_name;
    klass->set_bounds = eek_element_real_set_bounds;
    klass->get_bounds = eek_element_real_get_bounds;
    klass->set_theme = eek_element_real_set_theme;
    klass->get_theme = eek_element_real_get_theme;
    klass->get_theme_node = eek_element_real_get_theme_node;

    gobject_class->set_property = eek_element_set_property;
    gobject_class->get_property = eek_element_get_property;
    gobject_class->dispose      = eek_element_dispose;
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
     * EekElement:theme:
     *
     * The theme of #EekElement.
     */
    pspec = g_param_spec_object ("theme",
                                 "Theme",
                                 "Theme of the element",
                                 EEK_TYPE_THEME,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_THEME,
                                     pspec);

    /**
     * EekElement:pseudo-class:
     *
     * The pseudo-class of the actor. Typical values include "hover",
     * "active", "focus".
     */
    pspec = g_param_spec_string ("pseudo-class",
                                 "Pseudo Class",
                                 "Pseudo class for styling",
                                 "",
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_PSEUDO_CLASS,
                                     pspec);

    /**
     * EekElement:style-class:
     *
     * The style-class of the actor for use in styling.
     */
    pspec = g_param_spec_string ("style-class",
                                 "Style Class",
                                 "Style class for styling",
                                 "",
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_STYLE_CLASS,
                                     pspec);

    /**
     * EekElement:style:
     *
     * Inline style information for the actor as a ';'-separated list
     * of CSS properties.
     */
    pspec = g_param_spec_string ("style",
                                 "Style",
                                 "Inline style string",
                                 "",
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_STYLE,
                                     pspec);
    /**
     * EekElement::style-changed:
     *
     * Emitted when the style information that the widget derives from the
     * theme changes
     */
    signals[STYLE_CHANGED] =
        g_signal_new ("style-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (EekElementClass, style_changed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}

static void
eek_element_init (EekElement *self)
{
    EekElementPrivate *priv;

    priv = self->priv = EEK_ELEMENT_GET_PRIVATE(self);
    priv->name = NULL;
    memset (&priv->bounds, 0, sizeof priv->bounds);
}

/**
 * eek_element_set_parent:
 * @element: an #EekElement
 * @parent: an #EekElement
 *
 * Set the parent of @element to @parent.
 */
void
eek_element_set_parent (EekElement *element,
                        EekElement *parent)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    g_return_if_fail (EEK_IS_ELEMENT(parent));
    EEK_ELEMENT_GET_CLASS(element)->set_parent (element, parent);
}

/**
 * eek_element_get_parent:
 * @element: an #EekElement
 *
 * Get the parent of @element.
 * Returns: an #EekElement if the parent is set, otherwise %NULL
 */
EekElement *
eek_element_get_parent (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT(element), NULL);
    return EEK_ELEMENT_GET_CLASS(element)->get_parent (element);
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
    EEK_ELEMENT_GET_CLASS(element)->set_name (element, name);
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
    return EEK_ELEMENT_GET_CLASS(element)->get_name (element);
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
    EEK_ELEMENT_GET_CLASS(element)->set_bounds (element, bounds);
}

/**
 * eek_element_get_bounds:
 * @element: an #EekElement
 * @bounds: pointer where bounding box of @element will be stored
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
    EEK_ELEMENT_GET_CLASS(element)->get_bounds (element, bounds);
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

void
eek_element_set_theme (EekElement *element, EekTheme *theme)
{
    g_return_if_fail (EEK_IS_ELEMENT(element));
    g_return_if_fail (EEK_IS_THEME(theme));
    EEK_ELEMENT_GET_CLASS(element)->set_theme (element, theme);
}

EekTheme *
eek_element_get_theme (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT(element), NULL);
    return EEK_ELEMENT_GET_CLASS(element)->get_theme (element);
}

void
eek_element_style_changed (EekElement  *element)
{
    EekElementPrivate *priv = EEK_ELEMENT_GET_PRIVATE(element);
    EekThemeNode *old_theme_node = NULL;

    g_return_if_fail (priv);
    if (priv->theme_node) {
        old_theme_node = priv->theme_node;
        priv->theme_node = NULL;
    }

    g_signal_emit (element, signals[STYLE_CHANGED], 0);

    if (old_theme_node)
        g_object_unref (old_theme_node);
}

/**
 * eek_element_get_theme_node:
 * @element: an #EekElement
 *
 * Get the theme node of @element.
 * Returns: an #EekThemeNode if the theme node is set, otherwise %NULL
 */
EekThemeNode *
eek_element_get_theme_node (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT(element), NULL);
    return EEK_ELEMENT_GET_CLASS(element)->get_theme_node (element);
}

/* Style management functions below are taken from st-widget.c of
   gnome-shell. */
static const gchar *
find_class_name (const gchar *class_list,
                 const gchar *class_name)
{
  gint len = strlen (class_name);
  const gchar *match;

  if (!class_list)
      return NULL;

  for (match = strstr (class_list, class_name);
       match;
       match = strstr (match + 1, class_name))
      if ((match == class_list || g_ascii_isspace (match[-1])) &&
          (match[len] == '\0' || g_ascii_isspace (match[len])))
          return match;

  return NULL;
}

static gboolean
set_class_list (gchar      **class_list,
                const gchar *new_class_list)
{
    if (g_strcmp0 (*class_list, new_class_list) != 0) {
        g_free (*class_list);
        *class_list = g_strdup (new_class_list);
        return TRUE;
    } else
        return FALSE;
}

static gboolean
add_class_name (gchar       **class_list,
                const gchar  *class_name)
{
    gchar *new_class_list;

    if (*class_list) {
        if (find_class_name (*class_list, class_name))
            return FALSE;

        new_class_list = g_strdup_printf ("%s %s", *class_list, class_name);
        g_free (*class_list);
        *class_list = new_class_list;
    } else
        *class_list = g_strdup (class_name);

  return TRUE;
}

static gboolean
remove_class_name (gchar       **class_list,
                   const gchar  *class_name)
{
    const gchar *match, *end;
    gchar *new_class_list;

    if (!*class_list)
        return FALSE;

    if (strcmp (*class_list, class_name) == 0) {
        g_free (*class_list);
        *class_list = NULL;
        return TRUE;
    }

    match = find_class_name (*class_list, class_name);
    if (!match)
        return FALSE;
    end = match + strlen (class_name);

    /* Adjust either match or end to include a space as well.
     * (One or the other must be possible at this point.)
     */
    if (match != *class_list)
        match--;
    else
        end++;

    new_class_list = g_strdup_printf ("%.*s%s", (int)(match - *class_list),
                                      *class_list, end);
    g_free (*class_list);
    *class_list = new_class_list;

    return TRUE;
}

/**
 * eek_element_set_style_class_name:
 * @element: a #EekElement
 * @style_class_list: (allow-none): a new style class list string
 *
 * Set the style class name list. @style_class_list can either be
 * %NULL, for no classes, or a space-separated list of style class
 * names. See also eek_element_add_style_class_name() and
 * eek_element_remove_style_class_name().
 */
void
eek_element_set_style_class_name (EekElement    *element,
                                  const gchar *style_class_list)
{
    g_return_if_fail (EEK_IS_ELEMENT (element));

    if (set_class_list (&element->priv->style_class, style_class_list)) {
        eek_element_style_changed (element);
        g_object_notify (G_OBJECT (element), "style-class");
    }
}

/**
 * eek_element_add_style_class_name:
 * @element: a #EekElement
 * @style_class: a style class name string
 *
 * Adds @style_class to @element's style class name list, if it is not
 * already present.
 */
void
eek_element_add_style_class_name (EekElement    *element,
                                  const gchar *style_class)
{
    g_return_if_fail (EEK_IS_ELEMENT (element));
    g_return_if_fail (style_class != NULL);

    if (add_class_name (&element->priv->style_class, style_class)) {
        eek_element_style_changed (element);
        g_object_notify (G_OBJECT (element), "style-class");
    }
}

/**
 * eek_element_remove_style_class_name:
 * @element: a #EekElement
 * @style_class: a style class name string
 *
 * Removes @style_class from @element's style class name, if it is
 * present.
 */
void
eek_element_remove_style_class_name (EekElement    *element,
                                     const gchar *style_class)
{
    g_return_if_fail (EEK_IS_ELEMENT (element));
    g_return_if_fail (style_class != NULL);

    if (remove_class_name (&element->priv->style_class, style_class)) {
        eek_element_style_changed (element);
        g_object_notify (G_OBJECT (element), "style-class");
    }
}

/**
 * eek_element_get_style_class_name:
 * @element: a #EekElement
 *
 * Get the current style class name
 *
 * Returns: the class name string. The string is owned by the #EekElement and
 * should not be modified or freed.
 */
const gchar*
eek_element_get_style_class_name (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT (element), NULL);

    return element->priv->style_class;
}

/**
 * eek_element_has_style_class_name:
 * @element: a #EekElement
 * @style_class: a style class string
 *
 * Tests if @element's style class list includes @style_class.
 *
 * Returns: whether or not @element's style class list includes
 * @style_class.
 */
gboolean
eek_element_has_style_class_name (EekElement    *element,
                                const gchar *style_class)
{
    g_return_val_if_fail (EEK_IS_ELEMENT (element), FALSE);

    return find_class_name (element->priv->style_class, style_class) != NULL;
}

/**
 * eek_element_get_style_pseudo_class:
 * @element: a #EekElement
 *
 * Get the current style pseudo class list.
 *
 * Note that an element can have multiple pseudo classes; if you just
 * want to test for the presence of a specific pseudo class, use
 * eek_element_has_style_pseudo_class().
 *
 * Returns: the pseudo class list string. The string is owned by the
 * #EekElement and should not be modified or freed.
 */
const gchar*
eek_element_get_style_pseudo_class (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT (element), NULL);

    return element->priv->pseudo_class;
}

/**
 * eek_element_has_style_pseudo_class:
 * @element: a #EekElement
 * @pseudo_class: a pseudo class string
 *
 * Tests if @element's pseudo class list includes @pseudo_class.
 *
 * Returns: whether or not @element's pseudo class list includes
 * @pseudo_class.
 */
gboolean
eek_element_has_style_pseudo_class (EekElement    *element,
                                    const gchar *pseudo_class)
{
    g_return_val_if_fail (EEK_IS_ELEMENT (element), FALSE);

    return find_class_name (element->priv->pseudo_class, pseudo_class) != NULL;
}

/**
 * eek_element_set_style_pseudo_class:
 * @element: a #EekElement
 * @pseudo_class_list: (allow-none): a new pseudo class list string
 *
 * Set the style pseudo class list. @pseudo_class_list can either be
 * %NULL, for no classes, or a space-separated list of pseudo class
 * names. See also eek_element_add_style_pseudo_class() and
 * eek_element_remove_style_pseudo_class().
 */
void
eek_element_set_style_pseudo_class (EekElement    *element,
                                  const gchar *pseudo_class_list)
{
    g_return_if_fail (EEK_IS_ELEMENT (element));

    if (set_class_list (&element->priv->pseudo_class, pseudo_class_list)) {
        eek_element_style_changed (element);
        g_object_notify (G_OBJECT (element), "pseudo-class");
    }
}

/**
 * eek_element_add_style_pseudo_class:
 * @element: a #EekElement
 * @pseudo_class: a pseudo class string
 *
 * Adds @pseudo_class to @element's pseudo class list, if it is not
 * already present.
 */
void
eek_element_add_style_pseudo_class (EekElement    *element,
                                  const gchar *pseudo_class)
{
    g_return_if_fail (EEK_IS_ELEMENT (element));
    g_return_if_fail (pseudo_class != NULL);

    if (add_class_name (&element->priv->pseudo_class, pseudo_class)) {
        eek_element_style_changed (element);
        g_object_notify (G_OBJECT (element), "pseudo-class");
    }
}

/**
 * eek_element_remove_style_pseudo_class:
 * @element: a #EekElement
 * @pseudo_class: a pseudo class string
 *
 * Removes @pseudo_class from @element's pseudo class, if it is present.
 */
void
eek_element_remove_style_pseudo_class (EekElement    *element,
                                     const gchar *pseudo_class)
{
    g_return_if_fail (EEK_IS_ELEMENT (element));
    g_return_if_fail (pseudo_class != NULL);

    if (remove_class_name (&element->priv->pseudo_class, pseudo_class)) {
        eek_element_style_changed (element);
        g_object_notify (G_OBJECT (element), "pseudo-class");
    }
}

/**
 * eek_element_set_style:
 * @element: a #EekElement
 * @style_class: (allow-none): a inline style string, or %NULL
 *
 * Set the inline style string for this widget. The inline style string is an
 * optional ';'-separated list of CSS properties that override the style as
 * determined from the stylesheets of the current theme.
 */
void
eek_element_set_style (EekElement  *element,
                       const gchar *style)
{
    EekElementPrivate *priv = element->priv;

    g_return_if_fail (EEK_IS_ELEMENT (element));

    priv = element->priv;

    if (g_strcmp0 (style, priv->inline_style)) {
        g_free (priv->inline_style);
        priv->inline_style = g_strdup (style);

        eek_element_style_changed (element);

        g_object_notify (G_OBJECT (element), "style");
    }
}

/**
 * eek_element_get_style:
 * @element: a #EekElement
 *
 * Get the current inline style string. See eek_element_set_style().
 *
 * Returns: The inline style string, or %NULL. The string is owned by the
 * #EekElement and should not be modified or freed.
 */
const gchar*
eek_element_get_style (EekElement *element)
{
    g_return_val_if_fail (EEK_IS_ELEMENT (element), NULL);

    return element->priv->inline_style;
}
