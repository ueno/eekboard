/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; c-basic-offset: 2; -*- */
/*
 * eek-theme-node.c: style information for one node in a tree of themed objects
 *
 * Copyright 2008-2010 Red Hat, Inc.
 * Copyright 2009 Steve Frécinaux
 * Copyright 2009, 2010 Florian Müllner
 * Copyright 2010 Adel Gadllah
 * Copyright 2010 Giovanni Campagna
 * Copyright 2010-2011 Daiki Ueno
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

struct _EekThemeNode {
  GObject parent;

  EekThemeNode *parent_node;
  EekTheme *theme;

  EekGradientType background_gradient_type;

  EekColor background_color;
  EekColor background_gradient_end;

  EekColor foreground_color;
  EekColor border_color[4];

  int border_width[4];
  int border_radius[4];

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
  guint geometry_computed : 1;
  guint background_computed : 1;
  guint foreground_computed : 1;
};

struct _EekThemeNodeClass {
  GObjectClass parent_class;
};

G_DEFINE_TYPE (EekThemeNode, eek_theme_node, G_TYPE_OBJECT);

#define EEK_THEME_NODE_GET_PRIVATE(obj)                                 \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_THEME_NODE, EekThemeNodePrivate))

static void eek_theme_node_init               (EekThemeNode          *node);
static void eek_theme_node_class_init         (EekThemeNodeClass     *klass);
static void eek_theme_node_dispose           (GObject                 *object);
static void eek_theme_node_finalize           (GObject                 *object);

static const EekColor BLACK_COLOR = { 0, 0, 0, 0xff };
static const EekColor TRANSPARENT_COLOR = { 0, 0, 0, 0 };
static const EekColor DEFAULT_SUCCESS_COLOR = { 0x4e, 0x9a, 0x06, 0xff };
static const EekColor DEFAULT_WARNING_COLOR = { 0xf5, 0x79, 0x3e, 0xff };
static const EekColor DEFAULT_ERROR_COLOR = { 0xcc, 0x00, 0x00, 0xff };

static void
eek_theme_node_init (EekThemeNode *self)
{
}

static void
eek_theme_node_class_init (EekThemeNodeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = eek_theme_node_dispose;
  object_class->finalize = eek_theme_node_finalize;
}

static void
eek_theme_node_dispose (GObject *gobject)
{
  EekThemeNode *node = EEK_THEME_NODE (gobject);

  if (node->theme)
    {
      g_object_unref (node->theme);
      node->theme = NULL;
    }

  if (node->parent_node)
    {
      g_object_unref (node->parent_node);
      node->parent_node = NULL;
    }

  G_OBJECT_CLASS (eek_theme_node_parent_class)->dispose (gobject);
}

static void
eek_theme_node_finalize (GObject *object)
{
  EekThemeNode *node = EEK_THEME_NODE (object);

  g_free (node->element_id);
  g_free (node->element_class);
  g_free (node->pseudo_class);
  g_free (node->inline_style);

  if (node->properties)
    {
      g_free (node->properties);
      node->properties = NULL;
      node->n_properties = 0;
    }

  if (node->inline_properties)
    {
      /* This destroys the list, not just the head of the list */
      cr_declaration_destroy (node->inline_properties);
    }

  G_OBJECT_CLASS (eek_theme_node_parent_class)->finalize (object);
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

  g_return_val_if_fail (parent_node == NULL || EEK_IS_THEME_NODE (parent_node), NULL);

  node = g_object_new (EEK_TYPE_THEME_NODE, NULL);

  if (parent_node != NULL)
    node->parent_node = g_object_ref (parent_node);
  else
    node->parent_node = NULL;

  if (theme == NULL && parent_node != NULL)
    theme = eek_theme_node_get_theme (parent_node);

  if (theme != NULL)
    node->theme = g_object_ref (theme);

  node->element_type = element_type;
  node->element_id = g_strdup (element_id);
  node->element_class = g_strdup (element_class);
  node->pseudo_class = g_strdup (pseudo_class);
  node->inline_style = g_strdup (inline_style);

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
  g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

  return node->parent_node;
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
  g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

  return node->theme;
}

GType
eek_theme_node_get_element_type (EekThemeNode *node)
{
  g_return_val_if_fail (EEK_IS_THEME_NODE (node), G_TYPE_NONE);

  return node->element_type;
}

const char *
eek_theme_node_get_element_id (EekThemeNode *node)
{
  g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

  return node->element_id;
}

const char *
eek_theme_node_get_element_class (EekThemeNode *node)
{
  g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

  return node->element_class;
}

const char *
eek_theme_node_get_pseudo_class (EekThemeNode *node)
{
  g_return_val_if_fail (EEK_IS_THEME_NODE (node), NULL);

  return node->pseudo_class;
}

static void
ensure_properties (EekThemeNode *node)
{
  if (!node->properties_computed)
    {
      GPtrArray *properties = NULL;

      node->properties_computed = TRUE;

      if (node->theme)
        properties = _eek_theme_get_matched_properties (node->theme, node);

      if (node->inline_style)
        {
          CRDeclaration *cur_decl;

          if (!properties)
            properties = g_ptr_array_new ();

          node->inline_properties =
            _eek_theme_parse_declaration_list (node->inline_style);
          for (cur_decl = node->inline_properties;
               cur_decl;
               cur_decl = cur_decl->next)
            g_ptr_array_add (properties, cur_decl);
        }

      if (properties)
        {
          node->n_properties = properties->len;
          node->properties = (CRDeclaration **)g_ptr_array_free (properties,
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
get_color_from_rgba_term (CRTerm   *term,
                          EekColor *color)
{
  CRTerm *arg = term->ext_content.func_param;
  CRNum *num;
  double r = 0, g = 0, b = 0, a = 0;
  int i;

  for (i = 0; i < 4; i++)
    {
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
      if (i < 3)
        {
          if (num->type == NUM_PERCENTAGE)
            value = num->val / 100;
          else if (num->type == NUM_GENERIC)
            value = num->val / 255;
          else
            return VALUE_NOT_FOUND;
        }
      else
        {
          if (num->type != NUM_GENERIC)
            return VALUE_NOT_FOUND;

          value = num->val;
        }

      value = CLAMP (value, 0, 1);

      switch (i)
        {
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

  color->red = CLAMP(r, 0.0, 1.0);
  color->green = CLAMP(g, 0.0, 1.0);
  color->blue = CLAMP(b, 0.0, 1.0);
  color->alpha = CLAMP(a, 0.0, 1.0);

  return VALUE_FOUND;
}

static GetFromTermResult
get_color_from_term (EekThemeNode *node,
                     CRTerm       *term,
                     EekColor     *color)
{
  CRRgb rgb;
  enum CRStatus status;

  /* Since libcroco doesn't know about rgba colors, it can't handle
   * the transparent keyword
   */
  if (term_is_transparent (term))
    {
      *color = TRANSPARENT_COLOR;
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
           g_ascii_strcasecmp (term->content.str->stryng->str, "rgba") == 0)
    {
      return get_color_from_rgba_term (term, color);
    }

  status = cr_rgb_set_from_term (&rgb, term);
  if (status != CR_OK)
    return VALUE_NOT_FOUND;

  if (rgb.inherit)
    return VALUE_INHERIT;

  if (rgb.is_percentage)
    cr_rgb_compute_from_percentage (&rgb);

  color->red = rgb.red / (gdouble)0xff;
  color->green = rgb.green / (gdouble)0xff;
  color->blue = rgb.blue / (gdouble)0xff;
  color->alpha = 1.0;

  return VALUE_FOUND;
}

/**
 * eek_theme_node_lookup_color:
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
eek_theme_node_lookup_color (EekThemeNode *node,
                             const char   *property_name,
                             gboolean      inherit,
                             EekColor     *color)
{
  int i;

  g_return_val_if_fail (EEK_IS_THEME_NODE(node), FALSE);

  ensure_properties (node);

  for (i = node->n_properties - 1; i >= 0; i--)
    {
      CRDeclaration *decl = node->properties[i];

      if (strcmp (decl->property->stryng->str, property_name) == 0)
        {
          GetFromTermResult result = get_color_from_term (node, decl->value, color);
          if (result == VALUE_FOUND)
            {
              return TRUE;
            }
          else if (result == VALUE_INHERIT)
            {
              if (node->parent_node)
                return eek_theme_node_lookup_color (node->parent_node, property_name, inherit, color);
              else
                break;
            }
        }
    }

  if (inherit && node->parent_node)
    return eek_theme_node_lookup_color (node->parent_node, property_name, inherit, color);

  return FALSE;
}

/**
 * eek_theme_node_get_color:
 * @node: a #EekThemeNode
 * @property_name: The name of the color property
 * @color: location to store the color that was determined.
 *
 * Generically looks up a property containing a single color value. When
 * specific getters (like eek_theme_node_get_background_color()) exist, they
 * should be used instead. They are cached, so more efficient, and have
 * handling for shortcut properties and other details of CSS.
 *
 * If @property_name is not found, a warning will be logged and a
 * default color returned.
 *
 * See also eek_theme_node_lookup_color(), which provides more options,
 * and lets you handle the case where the theme does not specify the
 * indicated color.
 */
void
eek_theme_node_get_color (EekThemeNode *node,
                          const char   *property_name,
                          EekColor     *color)
{
  if (!eek_theme_node_lookup_color (node, property_name, FALSE, color))
    {
      g_warning ("Did not find color property '%s'", property_name);
      memset (color, 0, sizeof (EekColor));
    }
}

/**
 * eek_theme_node_lookup_double:
 * @node: a #EekThemeNode
 * @property_name: The name of the numeric property
 * @inherit: if %TRUE, if a value is not found for the property on the
 *   node, then it will be looked up on the parent node, and then on the
 *   parent's parent, and so forth. Note that if the property has a
 *   value of 'inherit' it will be inherited even if %FALSE is passed
 *   in for @inherit; this only affects the default behavior for inheritance.
 * @value: (out): location to store the value that was determined.
 *   If the property is not found, the value in this location
 *   will not be changed.
 *
 * Generically looks up a property containing a single numeric value
 *  without units.
 *
 * See also eek_theme_node_get_double(), which provides a simpler API.
 *
 * Return value: %TRUE if the property was found in the properties for this
 *  theme node (or in the properties of parent nodes when inheriting.)
 */
gboolean
eek_theme_node_lookup_double (EekThemeNode *node,
                              const char   *property_name,
                              gboolean      inherit,
                              double       *value)
{
  gboolean result = FALSE;
  int i;

  g_return_val_if_fail (EEK_IS_THEME_NODE(node), FALSE);

  ensure_properties (node);

  for (i = node->n_properties - 1; i >= 0; i--)
    {
      CRDeclaration *decl = node->properties[i];

      if (strcmp (decl->property->stryng->str, property_name) == 0)
        {
          CRTerm *term = decl->value;

          if (term->type != TERM_NUMBER || term->content.num->type != NUM_GENERIC)
            continue;

          *value = term->content.num->val;
          result = TRUE;
          break;
        }
    }

  if (!result && inherit && node->parent_node)
    result = eek_theme_node_lookup_double (node->parent_node, property_name, inherit, value);

  return result;
}

/**
 * eek_theme_node_get_double:
 * @node: a #EekThemeNode
 * @property_name: The name of the numeric property
 *
 * Generically looks up a property containing a single numeric value
 *  without units.
 *
 * See also eek_theme_node_lookup_double(), which provides more options,
 * and lets you handle the case where the theme does not specify the
 * indicated value.
 *
 * Return value: the value found. If @property_name is not
 *  found, a warning will be logged and 0 will be returned.
 */
gdouble
eek_theme_node_get_double (EekThemeNode *node,
                           const char   *property_name)
{
  gdouble value;

  if (eek_theme_node_lookup_double (node, property_name, FALSE, &value))
    return value;
  else
    {
      g_warning ("Did not find double property '%s'", property_name);
      return 0.0;
    }
}

static GetFromTermResult
get_length_from_term (EekThemeNode *node,
                      CRTerm      *term,
                      gboolean     use_parent_font,
                      gdouble     *length)
{
  CRNum *num;

  enum {
    ABSOLUTE,
    POINTS,
    FONT_RELATIVE,
  } type = ABSOLUTE;

  double multiplier = 1.0;

  if (term->type != TERM_NUMBER)
    {
      g_warning ("Ignoring length property that isn't a number");
      return VALUE_NOT_FOUND;
    }

  num = term->content.num;

  switch (num->type)
    {
    case NUM_LENGTH_PX:
      type = ABSOLUTE;
      multiplier = 1;
      break;
    case NUM_LENGTH_PT:
      type = POINTS;
      multiplier = 1;
      break;
    case NUM_LENGTH_IN:
      type = POINTS;
      multiplier = 72;
      break;
    case NUM_LENGTH_CM:
      type = POINTS;
      multiplier = 72. / 2.54;
      break;
    case NUM_LENGTH_MM:
      type = POINTS;
      multiplier = 72. / 25.4;
      break;
    case NUM_LENGTH_PC:
      type = POINTS;
      multiplier = 12. / 25.4;
      break;
    case NUM_LENGTH_EM:
      {
        type = FONT_RELATIVE;
        multiplier = 1;
        break;
      }
    case NUM_LENGTH_EX:
      {
        /* Doing better would require actually resolving the font description
         * to a specific font, and Pango doesn't have an ex metric anyways,
         * so we'd have to try and synthesize it by complicated means.
         *
         * The 0.5em is the CSS spec suggested thing to use when nothing
         * better is available.
         */
        type = FONT_RELATIVE;
        multiplier = 0.5;
        break;
      }

    case NUM_INHERIT:
      return VALUE_INHERIT;

    case NUM_AUTO:
      g_warning ("'auto' not supported for lengths");
      return VALUE_NOT_FOUND;

    case NUM_GENERIC:
      {
        if (num->val != 0)
          {
            g_warning ("length values must specify a unit");
            return VALUE_NOT_FOUND;
          }
        else
          {
            type = ABSOLUTE;
            multiplier = 0;
          }
        break;
      }

    case NUM_PERCENTAGE:
      g_warning ("percentage lengths not currently supported");
      return VALUE_NOT_FOUND;

    case NUM_ANGLE_DEG:
    case NUM_ANGLE_RAD:
    case NUM_ANGLE_GRAD:
    case NUM_TIME_MS:
    case NUM_TIME_S:
    case NUM_FREQ_HZ:
    case NUM_FREQ_KHZ:
    case NUM_UNKNOWN_TYPE:
    case NB_NUM_TYPE:
      g_warning ("Ignoring invalid type of number of length property");
      return VALUE_NOT_FOUND;
    }

  switch (type)
    {
    case ABSOLUTE:
      *length = num->val * multiplier;
      break;
    case POINTS:
#if 0
      {
        double resolution = eek_theme_context_get_resolution (node->context);
        *length = num->val * multiplier * (resolution / 72.);
      }
#else
      *length = num->val * multiplier;
#endif
      break;
#if 0
    case FONT_RELATIVE:
      {
        const PangoFontDescription *desc;
        double font_size;

        if (use_parent_font)
          desc = get_parent_font (node);
        else
          desc = eek_theme_node_get_font (node);

        font_size = (double)pango_font_description_get_size (desc) / PANGO_SCALE;

        if (pango_font_description_get_size_is_absolute (desc))
          {
            *length = num->val * multiplier * font_size;
          }
        else
          {
            double resolution = eek_theme_context_get_resolution (node->context);
            *length = num->val * multiplier * (resolution / 72.) * font_size;
          }
      }
      break;
#endif
    default:
      g_assert_not_reached ();
    }

  return VALUE_FOUND;
}

static GetFromTermResult
get_length_from_term_int (EekThemeNode *node,
                          CRTerm      *term,
                          gboolean     use_parent_font,
                          gint        *length)
{
  double value;
  GetFromTermResult result;

  result = get_length_from_term (node, term, use_parent_font, &value);
  if (result == VALUE_FOUND)
    *length = (int) (0.5 + value);
  return result;
}

static GetFromTermResult
get_length_internal (EekThemeNode *node,
                     const char  *property_name,
                     const char  *suffixed,
                     gdouble     *length)
{
  int i;

  g_return_val_if_fail (EEK_IS_THEME_NODE(node), FALSE);

  ensure_properties (node);

  for (i = node->n_properties - 1; i >= 0; i--)
    {
      CRDeclaration *decl = node->properties[i];

      if (strcmp (decl->property->stryng->str, property_name) == 0 ||
          (suffixed != NULL && strcmp (decl->property->stryng->str, suffixed) == 0))
        {
          GetFromTermResult result = get_length_from_term (node, decl->value, FALSE, length);
          if (result != VALUE_NOT_FOUND)
            return result;
        }
    }

  return VALUE_NOT_FOUND;
}

/**
 * eek_theme_node_lookup_length:
 * @node: a #EekThemeNode
 * @property_name: The name of the length property
 * @inherit: if %TRUE, if a value is not found for the property on the
 *   node, then it will be looked up on the parent node, and then on the
 *   parent's parent, and so forth. Note that if the property has a
 *   value of 'inherit' it will be inherited even if %FALSE is passed
 *   in for @inherit; this only affects the default behavior for inheritance.
 * @length: (out): location to store the length that was determined.
 *   If the property is not found, the value in this location
 *   will not be changed. The returned length is resolved
 *   to pixels.
 *
 * Generically looks up a property containing a single length value. When
 * specific getters (like eek_theme_node_get_border_width()) exist, they
 * should be used instead. They are cached, so more efficient, and have
 * handling for shortcut properties and other details of CSS.
 *
 * See also eek_theme_node_get_length(), which provides a simpler API.
 *
 * Return value: %TRUE if the property was found in the properties for this
 *  theme node (or in the properties of parent nodes when inheriting.)
 */
gboolean
eek_theme_node_lookup_length (EekThemeNode *node,
                             const char  *property_name,
                             gboolean     inherit,
                             gdouble     *length)
{
  GetFromTermResult result;

  g_return_val_if_fail (EEK_IS_THEME_NODE(node), FALSE);

  result = get_length_internal (node, property_name, NULL, length);
  if (result == VALUE_FOUND)
    return TRUE;
  else if (result == VALUE_INHERIT)
    inherit = TRUE;

  if (inherit && node->parent_node &&
      eek_theme_node_lookup_length (node->parent_node, property_name, inherit, length))
    return TRUE;
  else
    return FALSE;
}

/**
 * eek_theme_node_get_length:
 * @node: a #EekThemeNode
 * @property_name: The name of the length property
 *
 * Generically looks up a property containing a single length value. When
 * specific getters (like eek_theme_node_get_border_width()) exist, they
 * should be used instead. They are cached, so more efficient, and have
 * handling for shortcut properties and other details of CSS.
 *
 * Unlike eek_theme_node_get_color() and eek_theme_node_get_double(),
 * this does not print a warning if the property is not found; it just
 * returns 0.
 *
 * See also eek_theme_node_lookup_length(), which provides more options.
 *
 * Return value: the length, in pixels, or 0 if the property was not found.
 */
gdouble
eek_theme_node_get_length (EekThemeNode *node,
                          const char  *property_name)
{
  gdouble length;

  if (eek_theme_node_lookup_length (node, property_name, FALSE, &length))
    return length;
  else
    return 0.0;
}

static void
do_border_radius_term (EekThemeNode *node,
                       CRTerm      *term,
                       gboolean     topleft,
                       gboolean     topright,
                       gboolean     bottomright,
                       gboolean     bottomleft)
{
  int value;

  g_return_if_fail (EEK_IS_THEME_NODE (node));

  if (get_length_from_term_int (node, term, FALSE, &value) != VALUE_FOUND)
    return;

  if (topleft)
    node->border_radius[EEK_CORNER_TOPLEFT] = value;
  if (topright)
    node->border_radius[EEK_CORNER_TOPRIGHT] = value;
  if (bottomright)
    node->border_radius[EEK_CORNER_BOTTOMRIGHT] = value;
  if (bottomleft)
    node->border_radius[EEK_CORNER_BOTTOMLEFT] = value;
}

static void
do_border_radius (EekThemeNode  *node,
                  CRDeclaration *decl)
{
  const char *property_name = decl->property->stryng->str + 13; /* Skip 'border-radius' */

  if (strcmp (property_name, "") == 0)
    {
      /* Slight deviation ... if we don't understand some of the terms and understand others,
       * then we set the ones we understand and ignore the others instead of ignoring the
       * whole thing
       */
      if (decl->value == NULL) /* 0 values */
        return;
      else if (decl->value->next == NULL) /* 1 value */
        {
          do_border_radius_term (node, decl->value,       TRUE, TRUE, TRUE, TRUE); /* all corners */
          return;
        }
      else if (decl->value->next->next == NULL) /* 2 values */
        {
          do_border_radius_term (node, decl->value,       TRUE,  FALSE,  TRUE,  FALSE);  /* topleft/bottomright */
          do_border_radius_term (node, decl->value->next, FALSE,  TRUE,   FALSE, TRUE);  /* topright/bottomleft */
        }
      else if (decl->value->next->next->next == NULL) /* 3 values */
        {
          do_border_radius_term (node, decl->value,             TRUE,  FALSE, FALSE, FALSE); /* topleft */
          do_border_radius_term (node, decl->value->next,       FALSE, TRUE,  FALSE, TRUE);  /* topright/bottomleft */
          do_border_radius_term (node, decl->value->next->next, FALSE, FALSE, TRUE,  FALSE);  /* bottomright */
        }
      else if (decl->value->next->next->next->next == NULL) /* 4 values */
        {
          do_border_radius_term (node, decl->value,                   TRUE,  FALSE, FALSE, FALSE); /* topleft */
          do_border_radius_term (node, decl->value->next,             FALSE, TRUE,  FALSE, FALSE); /* topright */
          do_border_radius_term (node, decl->value->next->next,       FALSE, FALSE, TRUE,  FALSE); /* bottomright */
          do_border_radius_term (node, decl->value->next->next->next, FALSE, FALSE, FALSE, TRUE);  /* bottomleft */
        }
      else
        {
          g_warning ("Too many values for border-radius property");
          return;
        }
    }
  else
    {
      if (decl->value == NULL || decl->value->next != NULL)
        return;

      if (strcmp (property_name, "-topleft") == 0)
        do_border_radius_term (node, decl->value, TRUE,  FALSE, FALSE, FALSE);
      else if (strcmp (property_name, "-topright") == 0)
        do_border_radius_term (node, decl->value, FALSE, TRUE,  FALSE, FALSE);
      else if (strcmp (property_name, "-bottomright") == 0)
        do_border_radius_term (node, decl->value, FALSE, FALSE, TRUE,  FALSE);
      else if (strcmp (property_name, "-bottomleft") == 0)
        do_border_radius_term (node, decl->value, FALSE, FALSE, FALSE, TRUE);
    }
}

static void
do_border_property (EekThemeNode   *node,
                    CRDeclaration *decl)
{
  const char *property_name = decl->property->stryng->str + 6; /* Skip 'border' */
  EekSide side = (EekSide)-1;
  EekColor color;
  gboolean color_set = FALSE;
  int width = 0; /* suppress warning */
  gboolean width_set = FALSE;
  int j;

  g_return_if_fail (EEK_IS_THEME_NODE (node));

  if (g_str_has_prefix (property_name, "-radius"))
    {
      do_border_radius (node, decl);
      return;
    }

  if (g_str_has_prefix (property_name, "-left"))
    {
      side = EEK_SIDE_LEFT;
      property_name += 5;
    }
  else if (g_str_has_prefix (property_name, "-right"))
    {
      side = EEK_SIDE_RIGHT;
      property_name += 6;
    }
  else if (g_str_has_prefix (property_name, "-top"))
    {
      side = EEK_SIDE_TOP;
      property_name += 4;
    }
  else if (g_str_has_prefix (property_name, "-bottom"))
    {
      side = EEK_SIDE_BOTTOM;
      property_name += 7;
    }

  if (strcmp (property_name, "") == 0)
    {
      /* Set value for width/color/style in any order */
      CRTerm *term;

      for (term = decl->value; term; term = term->next)
        {
          GetFromTermResult result;

          if (term->type == TERM_IDENT)
            {
              const char *ident = term->content.str->stryng->str;
              if (strcmp (ident, "none") == 0 || strcmp (ident, "hidden") == 0)
                {
                  width = 0;
                  width_set = TRUE;
                  continue;
                }
              else if (strcmp (ident, "solid") == 0)
                {
                  /* The only thing we support */
                  continue;
                }
              else if (strcmp (ident, "dotted") == 0 ||
                       strcmp (ident, "dashed") == 0 ||
                       strcmp (ident, "double") == 0 ||
                       strcmp (ident, "groove") == 0 ||
                       strcmp (ident, "ridge") == 0 ||
                       strcmp (ident, "inset") == 0 ||
                       strcmp (ident, "outset") == 0)
                {
                  /* Treat the same as solid */
                  continue;
                }

              /* Presumably a color, fall through */
            }

          if (term->type == TERM_NUMBER)
            {
              result = get_length_from_term_int (node, term, FALSE, &width);
              if (result != VALUE_NOT_FOUND)
                {
                  width_set = result == VALUE_FOUND;
                  continue;
                }
            }

          result = get_color_from_term (node, term, &color);
          if (result != VALUE_NOT_FOUND)
            {
              color_set = result == VALUE_FOUND;
              continue;
            }
        }

    }
  else if (strcmp (property_name, "-color") == 0)
    {
      if (decl->value == NULL || decl->value->next != NULL)
        return;

      if (get_color_from_term (node, decl->value, &color) == VALUE_FOUND)
        /* Ignore inherit */
        color_set = TRUE;
    }
  else if (strcmp (property_name, "-width") == 0)
    {
      if (decl->value == NULL || decl->value->next != NULL)
        return;

      if (get_length_from_term_int (node, decl->value, FALSE, &width) == VALUE_FOUND)
        /* Ignore inherit */
        width_set = TRUE;
    }

  if (side == (EekSide)-1)
    {
      for (j = 0; j < 4; j++)
        {
          if (color_set)
            node->border_color[j] = color;
          if (width_set)
            node->border_width[j] = width;
        }
    }
  else
    {
      if (color_set)
        node->border_color[side] = color;
      if (width_set)
        node->border_width[side] = width;
    }
}

void
_eek_theme_node_ensure_geometry (EekThemeNode *node)
{
  int i, j;

  g_return_if_fail (EEK_IS_THEME_NODE (node));

  if (node->geometry_computed)
    return;

  node->geometry_computed = TRUE;

  ensure_properties (node);

  for (j = 0; j < 4; j++)
    {
      node->border_width[j] = 0;
      node->border_color[j] = TRANSPARENT_COLOR;
    }

  for (i = 0; i < node->n_properties; i++)
    {
      CRDeclaration *decl = node->properties[i];
      const char *property_name = decl->property->stryng->str;

      if (g_str_has_prefix (property_name, "border"))
        do_border_property (node, decl);
    }
}

int
eek_theme_node_get_border_width (EekThemeNode *node,
                                 EekSide       side)
{
  g_return_val_if_fail (EEK_IS_THEME_NODE (node), 0.);
  g_return_val_if_fail (side >= EEK_SIDE_TOP && side <= EEK_SIDE_LEFT, 0.);

  _eek_theme_node_ensure_geometry (node);

  return node->border_width[side];
}

int
eek_theme_node_get_border_radius (EekThemeNode *node,
                                  EekCorner     corner)
{
  g_return_val_if_fail (EEK_IS_THEME_NODE (node), 0.);
  g_return_val_if_fail (corner >= EEK_CORNER_TOPLEFT && corner <= EEK_CORNER_BOTTOMLEFT, 0.);

  _eek_theme_node_ensure_geometry (node);

  return node->border_radius[corner];
}

static GetFromTermResult
get_background_color_from_term (EekThemeNode *node,
                                CRTerm       *term,
                                EekColor     *color)
{
  GetFromTermResult result = get_color_from_term (node, term, color);
  if (result == VALUE_NOT_FOUND)
    {
      if (term_is_transparent (term))
        {
          *color = TRANSPARENT_COLOR;
          return VALUE_FOUND;
        }
    }

  return result;
}

void
_eek_theme_node_ensure_background (EekThemeNode *node)
{
  int i;

  if (node->background_computed)
    return;

  node->background_computed = TRUE;
  node->background_color = TRANSPARENT_COLOR;
  node->background_gradient_type = EEK_GRADIENT_NONE;

  ensure_properties (node);

  for (i = 0; i < node->n_properties; i++)
    {
      CRDeclaration *decl = node->properties[i];
      const char *property_name = decl->property->stryng->str;

      if (g_str_has_prefix (property_name, "background"))
        property_name += 10;
      else
        continue;

      if (strcmp (property_name, "") == 0)
        {
          /* We're very liberal here ... if we recognize any term in the expression we take it, and
           * we ignore the rest. The actual specification is:
           *
           * background: [<'background-color'> || <'background-image'> || <'background-repeat'> || <'background-attachment'> || <'background-position'>] | inherit
           */

          CRTerm *term;
          /* background: property sets all terms to specified or default values */
          node->background_color = TRANSPARENT_COLOR;

          for (term = decl->value; term; term = term->next)
            {
              GetFromTermResult result = get_background_color_from_term (node, term, &node->background_color);
              if (result == VALUE_FOUND)
                {
                  /* color stored in node->background_color */
                }
              else if (result == VALUE_INHERIT)
                {
                  if (node->parent_node)
                    {
                      eek_theme_node_get_background_color (node->parent_node, &node->background_color);
                    }
                }
              else if (term_is_none (term))
                {
                  /* leave node->background_color as transparent */
                }
            }
        }
      else if (strcmp (property_name, "-color") == 0)
        {
          GetFromTermResult result;

          if (decl->value == NULL || decl->value->next != NULL)
            continue;

          result = get_background_color_from_term (node, decl->value, &node->background_color);
          if (result == VALUE_FOUND)
            {
              /* color stored in node->background_color */
            }
          else if (result == VALUE_INHERIT)
            {
              if (node->parent_node)
                eek_theme_node_get_background_color (node->parent_node, &node->background_color);
            }
        }
      else if (strcmp (property_name, "-gradient-direction") == 0)
        {
          CRTerm *term = decl->value;
          if (strcmp (term->content.str->stryng->str, "vertical") == 0)
            {
              node->background_gradient_type = EEK_GRADIENT_VERTICAL;
            }
          else if (strcmp (term->content.str->stryng->str, "horizontal") == 0)
            {
              node->background_gradient_type = EEK_GRADIENT_HORIZONTAL;
            }
          else if (strcmp (term->content.str->stryng->str, "radial") == 0)
            {
              node->background_gradient_type = EEK_GRADIENT_RADIAL;
            }
          else if (strcmp (term->content.str->stryng->str, "none") == 0)
            {
              node->background_gradient_type = EEK_GRADIENT_NONE;
            }
          else
            {
              g_warning ("Unrecognized background-gradient-direction \"%s\"",
                         term->content.str->stryng->str);
            }
        }
      else if (strcmp (property_name, "-gradient-start") == 0)
        {
          get_color_from_term (node, decl->value, &node->background_color);
        }
      else if (strcmp (property_name, "-gradient-end") == 0)
        {
          get_color_from_term (node, decl->value, &node->background_gradient_end);
        }
    }
}

void
eek_theme_node_get_background_color (EekThemeNode *node,
                                     EekColor     *color)
{
  g_return_if_fail (EEK_IS_THEME_NODE (node));

  _eek_theme_node_ensure_background (node);

  *color = node->background_color;
}

void
eek_theme_node_get_border_color (EekThemeNode  *node,
                                 EekSide        side,
                                 EekColor *color)
{
  g_return_if_fail (EEK_IS_THEME_NODE (node));
  g_return_if_fail (side >= EEK_SIDE_TOP && side <= EEK_SIDE_LEFT);

  _eek_theme_node_ensure_geometry (node);

  *color = node->border_color[side];
}

void
eek_theme_node_get_foreground_color (EekThemeNode *node,
                                     EekColor     *color)
{
  g_assert (EEK_IS_THEME_NODE (node));

  if (!node->foreground_computed)
    {
      int i;

      node->foreground_computed = TRUE;

      ensure_properties (node);

      for (i = node->n_properties - 1; i >= 0; i--)
        {
          CRDeclaration *decl = node->properties[i];

          if (strcmp (decl->property->stryng->str, "color") == 0)
            {
              GetFromTermResult result = get_color_from_term (node, decl->value, &node->foreground_color);
              if (result == VALUE_FOUND)
                goto out;
              else if (result == VALUE_INHERIT)
                break;
            }
        }

      if (node->parent_node)
        eek_theme_node_get_foreground_color (node->parent_node, &node->foreground_color);
      else
        node->foreground_color = BLACK_COLOR; /* default to black */
    }

 out:
  *color = node->foreground_color;
}

void
eek_theme_node_get_background_gradient (EekThemeNode    *node,
                                        EekGradientType *type,
                                        EekColor        *start,
                                        EekColor        *end)
{
  g_assert (EEK_IS_THEME_NODE (node));

  _eek_theme_node_ensure_background (node);

  *type = node->background_gradient_type;
  if (*type != EEK_GRADIENT_NONE)
    {
      *start = node->background_color;
      *end = node->background_gradient_end;
    }
}
