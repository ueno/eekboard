/*
 * st-theme-node.c: style information for one node in a tree of themed objects
 *
 * Copyright 2008-2010 Red Hat, Inc.
 * Copyright 2009 Steve Frécinaux
 * Copyright 2009, 2010 Florian Müllner
 * Copyright 2010 Adel Gadllah
 * Copyright 2010 Giovanni Campagna
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <libcroco/libcroco.h>

#include "eek-theme-node.h"
#include "eek-theme-private.h"

G_DEFINE_TYPE (EekThemeNode, eek_theme_node, G_TYPE_OBJECT);

#define EEK_THEME_NODE_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_THEME_NODE, EekThemeNodePrivate))

struct _EekThemeNodePrivate
{
    EekThemeNode *parent_node;
    EekTheme *theme;

    EekColor *transparent_color;
    EekColor *black_color;

    EekGradientType background_gradient_type;

    EekColor *background_color;
    EekColor *background_gradient_end;
    EekColor *foreground_color;

    GType element_type;
    char *element_id;
    char *element_class;
    char *pseudo_class;
    char *inline_style;

    CRDeclaration **properties;
    int n_properties;

    /* We hold onto these separately so we can destroy them on finalize */
    CRDeclaration *inline_properties;

    guint properties_computed : 1;
    guint background_computed : 1;
    guint foreground_computed : 1;
};

static void
eek_theme_node_dispose (GObject *gobject)
{
    EekThemeNodePrivate *priv = EEK_THEME_NODE_GET_PRIVATE (gobject);

    if (priv->theme) {
        g_object_unref (priv->theme);
        priv->theme = NULL;
    }

    if (priv->parent_node) {
        g_object_unref (priv->parent_node);
        priv->parent_node = NULL;
    }

    G_OBJECT_CLASS (eek_theme_node_parent_class)->dispose (gobject);
}

static void
eek_theme_node_finalize (GObject *object)
{
    EekThemeNodePrivate *priv = EEK_THEME_NODE_GET_PRIVATE (object);

    eek_color_free (priv->transparent_color);
    eek_color_free (priv->black_color);
    eek_color_free (priv->background_color);
    eek_color_free (priv->background_gradient_end);
    eek_color_free (priv->foreground_color);

    g_free (priv->element_id);
    g_free (priv->element_class);
    g_free (priv->pseudo_class);
    g_free (priv->inline_style);

    if (priv->properties) {
        g_free (priv->properties);
        priv->properties = NULL;
        priv->n_properties = 0;
    }

    if (priv->inline_properties) {
        /* This destroys the list, not just the head of the list */
        cr_declaration_destroy (priv->inline_properties);
    }

    G_OBJECT_CLASS (eek_theme_node_parent_class)->finalize (object);
}

static void
eek_theme_node_class_init (EekThemeNodeClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekThemeNodePrivate));

    gobject_class->dispose = eek_theme_node_dispose;
    gobject_class->finalize = eek_theme_node_finalize;
}

static void
eek_theme_node_init (EekThemeNode *self)
{
    EekThemeNodePrivate *priv;

    priv = self->priv = EEK_THEME_NODE_GET_PRIVATE(self);

    priv->transparent_color = eek_color_new (0.0, 0.0, 0.0, 0.0);
    priv->black_color = eek_color_new (0.0, 0.0, 0.0, 1.0);
    priv->background_color = eek_color_copy (priv->transparent_color);
    priv->background_gradient_end = eek_color_copy (priv->transparent_color);
    priv->foreground_color = eek_color_copy (priv->black_color);
    priv->background_gradient_type = EEK_GRADIENT_NONE;
}

/**
 * eek_theme_node_new:
 * @parent_node: (allow-none): the parent node of this node
 * @theme: (allow-none): a theme (stylesheet set) that overrides the
 *   theme inherited from the parent node
 * @element_type: the type of the GObject represented by this node
 *  in the tree (corresponding to an element if we were theming an XML
 *  document. %G_TYPE_NONE means this style was created for the stage
 * actor and matches a selector element name of 'stage'.
 * @element_id: (allow-none): the ID to match CSS rules against
 * @element_class: (allow-none): a whitespace-separated list of classes
 *   to match CSS rules against
 * @pseudo_class: (allow-none): a whitespace-separated list of pseudo-classes
 *   (like 'hover' or 'visited') to match CSS rules against
 *
 * Creates a new #EekThemeNode. Once created, a node is immutable. Of any
 * of the attributes of the node (like the @element_class) change the node
 * and its child nodes must be destroyed and recreated.
 *
 * Return value: (transfer full): the theme node
 */
EekThemeNode *
eek_theme_node_new (EekThemeNode *parent_node,
                    EekTheme     *theme,
                    GType         element_type,
                    const char   *element_id,
                    const char   *element_class,
                    const char   *pseudo_class,
                    const char   *inline_style)
{
    EekThemeNode *node;
    EekThemeNodePrivate *priv;

    g_return_val_if_fail (parent_node == NULL || EEK_IS_THEME_NODE (parent_node), NULL);

    node = g_object_new (EEK_TYPE_THEME_NODE, NULL);
    priv = EEK_THEME_NODE_GET_PRIVATE(node);

    if (parent_node != NULL)
        priv->parent_node = g_object_ref (parent_node);
    else
        priv->parent_node = NULL;

    if (theme == NULL && parent_node != NULL)
        theme = eek_theme_node_get_theme (parent_node);

    if (theme != NULL)
        priv->theme = g_object_ref (theme);

    priv->element_type = element_type;
    priv->element_id = g_strdup (element_id);
    priv->element_class = g_strdup (element_class);
    priv->pseudo_class = g_strdup (pseudo_class);
    priv->inline_style = g_strdup (inline_style);

    return node;
}

/**
 * eek_theme_node_get_parent:
 * @node: a #EekThemeNode
 *
 * Gets the parent themed element node.
 *
 * Return value: (transfer none): the parent #EekThemeNode, or %NULL if this
 *  is the root node of the tree of theme elements.
 */
EekThemeNode *
eek_theme_node_get_parent (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

    priv = EEK_THEME_NODE_GET_PRIVATE(node);
    return priv->parent_node;
}

/**
 * eek_theme_node_get_theme:
 * @node: a #EekThemeNode
 *
 * Gets the theme stylesheet set that styles this node
 *
 * Return value: (transfer none): the theme stylesheet set
 */
EekTheme *
eek_theme_node_get_theme (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

    priv = EEK_THEME_NODE_GET_PRIVATE(node);
    return priv->theme;
}

GType
eek_theme_node_get_element_type (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_return_val_if_fail (EEK_IS_THEME_NODE (node), G_TYPE_NONE);

    priv = EEK_THEME_NODE_GET_PRIVATE(node);
    return priv->element_type;
}

const char *
eek_theme_node_get_element_id (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

    priv = EEK_THEME_NODE_GET_PRIVATE(node);
    return priv->element_id;
}

const char *
eek_theme_node_get_element_class (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

    priv = EEK_THEME_NODE_GET_PRIVATE(node);
    return priv->element_class;
}

void
eek_theme_node_set_pseudo_class (EekThemeNode *node,
                                 const gchar  *pseudo_class)
{
    EekThemeNodePrivate *priv;

    g_return_if_fail (EEK_IS_THEME_NODE (node));

    priv = EEK_THEME_NODE_GET_PRIVATE(node);

    if (g_strcmp0 (pseudo_class, priv->pseudo_class)) {
        g_free (priv->pseudo_class);
        priv->pseudo_class = g_strdup (pseudo_class);
        priv->properties_computed = 0;
        priv->background_computed = 0;
        priv->foreground_computed = 0;
    }
}

const char *
eek_theme_node_get_pseudo_class (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

    priv = EEK_THEME_NODE_GET_PRIVATE(node);
    return priv->pseudo_class;
}

static void
ensure_properties (EekThemeNode *node)
{
    EekThemeNodePrivate *priv = EEK_THEME_NODE_GET_PRIVATE(node);

    if (!priv->properties_computed) {
        GPtrArray *properties = NULL;

        priv->properties_computed = TRUE;

        if (priv->theme)
            properties = _eek_theme_get_matched_properties (priv->theme, node);

        if (priv->inline_style) {
            CRDeclaration *cur_decl;

            if (!properties)
                properties = g_ptr_array_new ();

            priv->inline_properties =
                _eek_theme_parse_declaration_list (priv->inline_style);
            for (cur_decl = priv->inline_properties;
                 cur_decl;
                 cur_decl = cur_decl->next)
                g_ptr_array_add (properties, cur_decl);
        }

        if (properties) {
            priv->n_properties = properties->len;
            priv->properties = (CRDeclaration **)g_ptr_array_free (properties,
                                                                   FALSE);
        }
    }
}

typedef enum {
    VALUE_FOUND,
    VALUE_NOT_FOUND,
    VALUE_INHERIT
} GetFromTermResult;

static gboolean
term_is_none (CRTerm *term)
{
    return (term->type == TERM_IDENT &&
            strcmp (term->content.str->stryng->str, "none") == 0);
}

static gboolean
term_is_transparent (CRTerm *term)
{
    return (term->type == TERM_IDENT &&
            strcmp (term->content.str->stryng->str, "transparent") == 0);
}

static GetFromTermResult
get_color_from_rgba_term (CRTerm    *term,
                          EekColor **color)
{
    CRTerm *arg = term->ext_content.func_param;
    CRNum *num;
    double r = 0, g = 0, b = 0, a = 0;
    int i;

    for (i = 0; i < 4; i++) {
        double value;

        if (arg == NULL)
            return VALUE_NOT_FOUND;

        if ((i == 0 && arg->the_operator != NO_OP) ||
            (i > 0 && arg->the_operator != COMMA))
            return VALUE_NOT_FOUND;

        if (arg->type != TERM_NUMBER)
            return VALUE_NOT_FOUND;

        num = arg->content.num;

        /* For simplicity, we convert a,r,g,b to [0,1.0] floats and then
         * convert them back below. Then when we set them on a cairo content
         * we convert them back to floats, and then cairo converts them
         * back to integers to pass them to X, and so forth...
         */
        if (i < 3) {
            if (num->type == NUM_PERCENTAGE)
                value = num->val / 100;
            else if (num->type == NUM_GENERIC)
                value = num->val / 255;
            else
                return VALUE_NOT_FOUND;
        } else {
            if (num->type != NUM_GENERIC)
                return VALUE_NOT_FOUND;

            value = num->val;
        }

        value = CLAMP (value, 0, 1);

        switch (i) {
        case 0:
            r = value;
            break;
        case 1:
            g = value;
            break;
        case 2:
            b = value;
            break;
        case 3:
            a = value;
            break;
        }

        arg = arg->next;
    }

    *color = eek_color_new (CLAMP(r, 0.0, 1.0),
                            CLAMP(g, 0.0, 1.0),
                            CLAMP(b, 0.0, 1.0),
                            CLAMP(a, 0.0, 1.0));

    return VALUE_FOUND;
}

static GetFromTermResult
get_color_from_term (EekThemeNode *node,
                     CRTerm       *term,
                     EekColor    **color)
{
    EekThemeNodePrivate *priv = EEK_THEME_NODE_GET_PRIVATE(node);
    CRRgb rgb;
    enum CRStatus status;

    /* Since libcroco doesn't know about rgba colors, it can't handle
     * the transparent keyword
     */
    if (term_is_transparent (term)) {
        *color = eek_color_copy (priv->transparent_color);
        return VALUE_FOUND;
    }
    /* rgba () colors - a CSS3 addition, are not supported by libcroco,
     * but they are parsed as a "function", so we can emulate the
     * functionality.
     *
     * libcroco < 0.6.2 has a bug where functions starting with 'r' are
     * misparsed. We workaround this by pre-converting 'rgba' to 'RGBA'
     * before parsing the stylesheet. Since libcroco isn't
     * case-insensitive (a bug), it's fine with functions starting with
     * 'R'. (In theory, we should be doing a case-insensitive compare
     * everywhere, not just here, but that doesn't make much sense when
     * the built-in parsing of libcroco is case-sensitive and things
     * like 10PX don't work.)
     */
    else if (term->type == TERM_FUNCTION &&
             term->content.str &&
             term->content.str->stryng &&
             term->content.str->stryng->str &&
             g_ascii_strcasecmp (term->content.str->stryng->str, "rgba") == 0) {
        return get_color_from_rgba_term (term, color);
    }

    status = cr_rgb_set_from_term (&rgb, term);
    if (status != CR_OK)
        return VALUE_NOT_FOUND;

    if (rgb.inherit)
        return VALUE_INHERIT;

    if (rgb.is_percentage)
        cr_rgb_compute_from_percentage (&rgb);

    *color = eek_color_new (rgb.red / (gdouble)0xff,
                            rgb.green / (gdouble)0xff,
                            rgb.blue / (gdouble)0xff,
                            1.0);

    return VALUE_FOUND;
}

/**
 * eek_theme_node_get_color:
 * @node: a #EekThemeNode
 * @property_name: The name of the color property
 * @inherit: if %TRUE, if a value is not found for the property on the
 *   node, then it will be looked up on the parent node, and then on the
 *   parent's parent, and so forth. Note that if the property has a
 *   value of 'inherit' it will be inherited even if %FALSE is passed
 *   in for @inherit; this only affects the default behavior for inheritance.
 * @color: location to store the color that was determined.
 *   If the property is not found, the value in this location
 *   will not be changed.
 *
 * Generically looks up a property containing a single color value. When
 * specific getters (like eek_theme_node_get_background_color()) exist, they
 * should be used instead. They are cached, so more efficient, and have
 * handling for shortcut properties and other details of CSS.
 *
 * Return value: %TRUE if the property was found in the properties for this
 *  theme node (or in the properties of parent nodes when inheriting.)
 */
gboolean
eek_theme_node_get_color (EekThemeNode *node,
                          const char   *property_name,
                          gboolean      inherit,
                          EekColor    **color)
{
    EekThemeNodePrivate *priv;
    int i;

    g_return_val_if_fail (EEK_IS_THEME_NODE(node), FALSE);

    priv = EEK_THEME_NODE_GET_PRIVATE(node);

    ensure_properties (node);

    for (i = priv->n_properties - 1; i >= 0; i--) {
        CRDeclaration *decl = priv->properties[i];

        if (strcmp (decl->property->stryng->str, property_name) == 0) {
            GetFromTermResult result = get_color_from_term (node, decl->value, color);
            if (result == VALUE_FOUND) {
                return TRUE;
            } else if (result == VALUE_INHERIT) {
                if (priv->parent_node)
                    return eek_theme_node_get_color (priv->parent_node, property_name, inherit, color);
                else
                    break;
            }
        }
    }

    return FALSE;
}

static GetFromTermResult
get_background_color_from_term (EekThemeNode *node,
                                CRTerm       *term,
                                EekColor    **color)
{
    EekThemeNodePrivate *priv = EEK_THEME_NODE_GET_PRIVATE(node);
    GetFromTermResult result = get_color_from_term (node, term, color);
    if (result == VALUE_NOT_FOUND) {
        if (term_is_transparent (term)) {
            *color = eek_color_copy (priv->transparent_color);
            return VALUE_FOUND;
        }
    }

    return result;
}

void
_eek_theme_node_ensure_background (EekThemeNode *node)
{
    EekThemeNodePrivate *priv = EEK_THEME_NODE_GET_PRIVATE(node);
    int i;

    if (priv->background_computed)
        return;

    priv->background_computed = TRUE;

    eek_color_free (priv->background_color);
    priv->background_color = eek_color_copy (priv->transparent_color);
    eek_color_free (priv->background_gradient_end);
    priv->background_gradient_end = eek_color_copy (priv->background_color);
    priv->background_gradient_type = EEK_GRADIENT_NONE;

    ensure_properties (node);

    for (i = 0; i < priv->n_properties; i++) {
        CRDeclaration *decl = priv->properties[i];
        const char *property_name = decl->property->stryng->str;
        GetFromTermResult result;
        EekColor *color;

        if (g_str_has_prefix (property_name, "background"))
            property_name += 10;
        else
            continue;

        if (strcmp (property_name, "") == 0) {
            /* We're very liberal here ... if we recognize any term in
             * the expression we take it, and we ignore the rest. The
             * actual specification is:
             *
             * background: [<'background-color'> ||
             * <'background-image'> || <'background-repeat'> ||
             * <'background-attachment'> || <'background-position'>] |
             * inherit
             */

            CRTerm *term;
            /* background: property sets all terms to specified or
               default values */
            eek_color_free (priv->background_color);
            priv->background_color = eek_color_copy (priv->transparent_color);

            for (term = decl->value; term; term = term->next) {
                result = get_background_color_from_term (node, term, &color);
                if (result == VALUE_FOUND) {
                    eek_color_free (priv->background_color);
                    priv->background_color = color;
                } else if (result == VALUE_INHERIT) {
                    if (priv->parent_node) {
                        color = eek_theme_node_get_background_color (priv->parent_node);
                        if (color) {
                            eek_color_free (priv->background_color);
                            priv->background_color = color;
                        }
                    }
                } else if (term_is_none (term)) {
                    /* leave priv->background_color as transparent */
                }
            }
        } else if (strcmp (property_name, "-color") == 0) {
            if (decl->value == NULL || decl->value->next != NULL)
                continue;

            result = get_background_color_from_term (node, decl->value, &color);
            if (result == VALUE_FOUND) {
                eek_color_free (priv->background_color);
                priv->background_color = color;
            } else if (result == VALUE_INHERIT) {
                if (priv->parent_node) {
                    color =
                        eek_theme_node_get_background_color (priv->parent_node);
                    if (color) {
                        eek_color_free (priv->background_color);
                        priv->background_color = color;
                    }
                }
            }
        } else if (strcmp (property_name, "-gradient-direction") == 0) {
            CRTerm *term = decl->value;
            if (strcmp (term->content.str->stryng->str, "vertical") == 0) {
                priv->background_gradient_type = EEK_GRADIENT_VERTICAL;
            } else if (strcmp (term->content.str->stryng->str, "horizontal") == 0) {
                priv->background_gradient_type = EEK_GRADIENT_HORIZONTAL;
            } else if (strcmp (term->content.str->stryng->str, "radial") == 0) {
                priv->background_gradient_type = EEK_GRADIENT_RADIAL;
            } else if (strcmp (term->content.str->stryng->str, "none") == 0) {
                priv->background_gradient_type = EEK_GRADIENT_NONE;
            } else {
                g_warning ("Unrecognized background-gradient-direction \"%s\"",
                           term->content.str->stryng->str);
            }
        } else if (strcmp (property_name, "-gradient-start") == 0) {
            result = get_color_from_term (node, decl->value, &color);
            if (result == VALUE_FOUND) {
                eek_color_free (priv->background_color);
                priv->background_color = color;
            }
        } else if (strcmp (property_name, "-gradient-end") == 0) {
            result = get_color_from_term (node, decl->value, &color);
            if (result == VALUE_FOUND) {
                eek_color_free (priv->background_gradient_end);
                priv->background_gradient_end = color;
            }
        }
    }
}

EekColor *
eek_theme_node_get_background_color (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_assert (EEK_IS_THEME_NODE (node));

    priv = EEK_THEME_NODE_GET_PRIVATE(node);

    _eek_theme_node_ensure_background (node);

    if (priv->background_gradient_type == EEK_GRADIENT_NONE)
        return eek_color_copy (priv->background_color);
    return NULL;
}

EekColor *
eek_theme_node_get_foreground_color (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_assert (EEK_IS_THEME_NODE (node));

    priv = EEK_THEME_NODE_GET_PRIVATE(node);

    if (!priv->foreground_computed) {
        int i;
        EekColor *color;

        priv->foreground_computed = TRUE;

        ensure_properties (node);

        for (i = priv->n_properties - 1; i >= 0; i--) {
            CRDeclaration *decl = priv->properties[i];

            if (strcmp (decl->property->stryng->str, "color") == 0) {
                GetFromTermResult result =
                    get_color_from_term (node, decl->value, &color);
                if (result == VALUE_FOUND) {
                    eek_color_free (priv->foreground_color);
                    priv->foreground_color = color;
                    goto out;
                }
                if (result == VALUE_INHERIT)
                    break;
            }
        }

        if (priv->parent_node) {
            color = eek_theme_node_get_foreground_color (priv->parent_node);
            if (color) {
                eek_color_free (priv->foreground_color);
                priv->foreground_color = color;
            }
        } else {
            /* default to black */
            eek_color_free (priv->foreground_color);
            priv->foreground_color = eek_color_copy (priv->black_color);
        }
    }

 out:
    return eek_color_copy (priv->foreground_color);
}

/**
 * eek_theme_node_get_background_gradient:
 * @node: A #EekThemeNode
 *
 * Get the background gradient of @node.  If the node does not have
 * gradient property, returns %NULL.
 *
 * Returns: an #EekGradient, which should be freed with
 * eek_gradient_free() or %NULL.
 */
EekGradient *
eek_theme_node_get_background_gradient (EekThemeNode *node)
{
    EekThemeNodePrivate *priv;

    g_assert (EEK_IS_THEME_NODE (node));

    priv = EEK_THEME_NODE_GET_PRIVATE(node);

    _eek_theme_node_ensure_background (node);

    if (priv->background_gradient_type == EEK_GRADIENT_NONE)
        return NULL;

    return eek_gradient_new (priv->background_gradient_type,
                             priv->background_color,
                             priv->background_gradient_end);
}
