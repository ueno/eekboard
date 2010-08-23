#include <stdlib.h>
#include <string.h>
#include <libcroco/libcroco.h>
#include "eek-theme-node-private.h"

static void eek_theme_node_init       (EekThemeNode      *node);
static void eek_theme_node_class_init (EekThemeNodeClass *klass);
static void eek_theme_node_dispose    (GObject           *object);
static void eek_theme_node_finalize   (GObject           *object);

static const EekColor BLACK_COLOR = { 0, 0, 0, 0xff };
static const EekColor TRANSPARENT_COLOR = { 0, 0, 0, 0 };

G_DEFINE_TYPE (EekThemeNode, eek_theme_node, G_TYPE_OBJECT);

static void
eek_theme_node_init (EekThemeNode *node)
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
    theme = parent_node->theme;

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

/**
 * eek_theme_node_equal:
 * @node_a: first #EekThemeNode
 * @node_b: second #EekThemeNode
 *
 * Compare two #EekThemeNodes. Two nodes which compare equal will match
 * the same CSS rules and have the same style properties. However, two
 * nodes that have ended up with identical style properties do not
 * necessarily compare equal.
 * In detail, @node_a and @node_b are considered equal iff
 * <itemizedlist>
 *   <listitem>
 *     <para>they share the same #EekTheme and #EekThemeContext</para>
 *   </listitem>
 *   <listitem>
 *     <para>they have the same parent</para>
 *   </listitem>
 *   <listitem>
 *     <para>they have the same element type</para>
 *   </listitem>
 *   <listitem>
 *     <para>their id, class, pseudo-class and inline-style match</para>
 *   </listitem>
 * </itemizedlist>
 *
 * Returns: %TRUE if @node_a equals @node_b
 */
gboolean
eek_theme_node_equal (EekThemeNode *node_a, EekThemeNode *node_b)
{
  g_return_val_if_fail (EEK_IS_THEME_NODE (node_a), FALSE);
  g_return_val_if_fail (EEK_IS_THEME_NODE (node_b), FALSE);

  return node_a->parent_node == node_b->parent_node &&
         node_a->theme == node_b->theme &&
         node_a->element_type == node_b->element_type &&
         !g_strcmp0 (node_a->element_id, node_b->element_id) &&
         !g_strcmp0 (node_a->element_class, node_b->element_class) &&
         !g_strcmp0 (node_a->pseudo_class, node_b->pseudo_class) &&
         !g_strcmp0 (node_a->inline_style, node_b->inline_style);
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

          node->inline_properties = _eek_theme_parse_declaration_list (node->inline_style);
          for (cur_decl = node->inline_properties; cur_decl; cur_decl = cur_decl->next)
            g_ptr_array_add (properties, cur_decl);
        }

      if (properties)
        {
          node->n_properties = properties->len;
          node->properties = (CRDeclaration **)g_ptr_array_free (properties, FALSE);
        }
    }
}

typedef enum {
  VALUE_FOUND,
  VALUE_NOT_FOUND,
  VALUE_INHERIT
} GetFromTermResult;

static gboolean
term_is_inherit (CRTerm *term)
{
  return (term->type == TERM_IDENT &&
          strcmp (term->content.str->stryng->str, "inherit") == 0);
}

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

static int
color_component_from_double (double component)
{
  /* We want to spread the range 0-1 equally over 0..255, but
   * 1.0 should map to 255 not 256, so we need to special-case it.
   * See http://people.redhat.com/otaylor/pixel-converting.html
   * for (very) detailed discussion of related issues. */
  if (component >= 1.0)
    return 255;
  else
    return (int)(component * 256);
}

static GetFromTermResult
get_color_from_rgba_term (CRTerm       *term,
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

  color->red = color_component_from_double (r);
  color->green = color_component_from_double (g);
  color->blue = color_component_from_double (b);
  color->alpha = color_component_from_double (a);

  return VALUE_FOUND;
}

static GetFromTermResult
get_color_from_term (EekThemeNode  *node,
                     CRTerm       *term,
                     EekColor *color)
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

  color->red = rgb.red;
  color->green = rgb.green;
  color->blue = rgb.blue;
  color->alpha = 0xff;

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
eek_theme_node_get_color (EekThemeNode  *node,
                         const char   *property_name,
                         gboolean      inherit,
                         EekColor *color)
{

  int i;

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
                return eek_theme_node_get_color (node->parent_node, property_name, inherit, color);
              else
                break;
            }
        }
    }

  return FALSE;
}

static GetFromTermResult
get_background_color_from_term (EekThemeNode  *node,
                                CRTerm       *term,
                                EekColor *color)
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
eek_theme_node_get_background_color (EekThemeNode  *node,
                                    EekColor *color)
{
  g_return_if_fail (EEK_IS_THEME_NODE (node));

  _eek_theme_node_ensure_background (node);

  *color = node->background_color;
}

void
eek_theme_node_get_foreground_color (EekThemeNode  *node,
                                    EekColor *color)
{
  g_return_if_fail (EEK_IS_THEME_NODE (node));

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


/**
 * eek_theme_node_get_background_gradient:
 * @node: A #EekThemeNode
 * @type: (out): Type of gradient
 * @start: Color at start of gradient
 * @end: Color at end of gradient
 *
 * The @start and @end arguments will only be set if @type is not #EEK_GRADIENT_NONE.
 */
void
eek_theme_node_get_background_gradient (EekThemeNode    *node,
                                        EekGradientType *type,
                                        EekColor   *start,
                                        EekColor   *end)
{
  g_return_if_fail (EEK_IS_THEME_NODE (node));

  _eek_theme_node_ensure_background (node);

  *type = node->background_gradient_type;
  if (*type != EEK_GRADIENT_NONE)
    {
      *start = node->background_color;
      *end = node->background_gradient_end;
    }
}
