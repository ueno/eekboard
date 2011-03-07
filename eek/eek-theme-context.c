/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; c-basic-offset: 2; -*- */
/*
 * eek-theme-context.c: holds global information about a tree of styled objects
 *
 * Copyright 2009, 2010 Red Hat, Inc.
 * Copyright 2009 Florian Müllner
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

#include "eek-theme.h"
#include "eek-theme-context.h"

struct _EekThemeContext {
  GObject parent;

  double resolution;
  PangoFontDescription *font;
  EekThemeNode *root_node;
  EekTheme *theme;
};

struct _EekThemeContextClass {
  GObjectClass parent_class;
};

#define DEFAULT_RESOLUTION 96.
#define DEFAULT_FONT "sans-serif 10"

enum
{
  CHANGED,

  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (EekThemeContext, eek_theme_context, G_TYPE_OBJECT)

static void
eek_theme_context_finalize (GObject *object)
{
  EekThemeContext *context = EEK_THEME_CONTEXT (object);

  if (context->root_node)
    g_object_unref (context->root_node);
  if (context->theme)
    g_object_unref (context->theme);

  pango_font_description_free (context->font);

  G_OBJECT_CLASS (eek_theme_context_parent_class)->finalize (object);
}

static void
eek_theme_context_class_init (EekThemeContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = eek_theme_context_finalize;

  signals[CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* no default handler slot */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}

static void
eek_theme_context_init (EekThemeContext *context)
{
  context->resolution = DEFAULT_RESOLUTION;
  context->font = pango_font_description_from_string (DEFAULT_FONT);
}

/**
 * eek_theme_context_new:
 *
 * Create a new theme context.
 */
EekThemeContext *
eek_theme_context_new (void)
{
  EekThemeContext *context;

  context = g_object_new (EEK_TYPE_THEME_CONTEXT, NULL);

  return context;
}

static void
eek_theme_context_changed (EekThemeContext *context)
{
  EekThemeNode *old_root = context->root_node;
  context->root_node = NULL;

  g_signal_emit (context, signals[CHANGED], 0);

  if (old_root)
    g_object_unref (old_root);
}

/**
 * eek_theme_context_set_theme:
 * @context: a #EekThemeContext
 *
 * Sets the default set of theme stylesheets for the context. This theme will
 * be used for the root node and for nodes descending from it, unless some other
 * style is explicitely specified.
 */
void
eek_theme_context_set_theme (EekThemeContext          *context,
                            EekTheme                 *theme)
{
  g_return_if_fail (EEK_IS_THEME_CONTEXT (context));
  g_return_if_fail (theme == NULL || EEK_IS_THEME (theme));

  if (context->theme != theme)
    {
      if (context->theme)
        g_object_unref (context->theme);

      context->theme = theme;

      if (context->theme)
        g_object_ref (context->theme);

      eek_theme_context_changed (context);
    }
}

/**
 * eek_theme_context_get_theme:
 * @context: a #EekThemeContext
 *
 * Gets the default theme for the context. See eek_theme_context_set_theme()
 *
 * Return value: (transfer none): the default theme for the context
 */
EekTheme *
eek_theme_context_get_theme (EekThemeContext *context)
{
  g_return_val_if_fail (EEK_IS_THEME_CONTEXT (context), NULL);

  return context->theme;
}

/**
 * eek_theme_context_set_resolution:
 * @context: a #EekThemeContext
 * @resolution: resolution of the context (number of pixels in an "inch")
 *
 * Sets the resolution of the theme context. This is the scale factor
 * used to convert between points and the length units pt, in, and cm.
 * This does not necessarily need to correspond to the actual number
 * resolution of the device. A value of 72. means that points and
 * pixels are identical. The default value is 96.
 */
void
eek_theme_context_set_resolution (EekThemeContext *context,
                                 double          resolution)
{
  g_return_if_fail (EEK_IS_THEME_CONTEXT (context));

  if (resolution == context->resolution)
    return;

  context->resolution = resolution;
  eek_theme_context_changed (context);
}

/**
 * eek_theme_context_set_default_resolution:
 * @context: a #EekThemeContext
 *
 * Sets the resolution of the theme context to the default value of 96.
 * See eek_theme_context_set_resolution().
 */
void
eek_theme_context_set_default_resolution (EekThemeContext *context)
{
  g_return_if_fail (EEK_IS_THEME_CONTEXT (context));

  if (context->resolution == DEFAULT_RESOLUTION)
    return;

  context->resolution = DEFAULT_RESOLUTION;
  eek_theme_context_changed (context);
}

/**
 * eek_theme_context_get_resolution:
 * @context: a #EekThemeContext
 *
 * Gets the current resolution of the theme context.
 * See eek_theme_context_set_resolution().
 *
 * Return value: the resolution (in dots-per-"inch")
 */
double
eek_theme_context_get_resolution (EekThemeContext *context)
{
  g_return_val_if_fail (EEK_IS_THEME_CONTEXT (context), DEFAULT_RESOLUTION);

  return context->resolution;
}

/**
 * eek_theme_context_set_font:
 * @context: a #EekThemeContext
 * @font: the default font for theme context
 *
 * Sets the default font for the theme context. This is the font that
 * is inherited by the root node of the tree of theme nodes. If the
 * font is not overriden, then this font will be used. If the font is
 * partially modified (for example, with 'font-size: 110%', then that
 * modification is based on this font.
 */
void
eek_theme_context_set_font (EekThemeContext             *context,
                           const PangoFontDescription *font)
{
  g_return_if_fail (EEK_IS_THEME_CONTEXT (context));
  g_return_if_fail (font != NULL);

  if (context->font == font ||
      pango_font_description_equal (context->font, font))
    return;

  pango_font_description_free (context->font);
  context->font = pango_font_description_copy (font);
  eek_theme_context_changed (context);
}

/**
 * eek_theme_context_get_font:
 * @context: a #EekThemeContext
 *
 * Gets the default font for the theme context. See eek_theme_context_set_font().
 *
 * Return value: the default font for the theme context.
 */
const PangoFontDescription *
eek_theme_context_get_font (EekThemeContext *context)
{
  g_return_val_if_fail (EEK_IS_THEME_CONTEXT (context), NULL);

  return context->font;
}

/**
 * eek_theme_context_get_root_node:
 * @context: a #EekThemeContext
 *
 * Gets the root node of the tree of theme style nodes that associated with this
 * context. For the node tree associated with a stage, this node represents
 * styles applied to the stage itself.
 *
 * Return value: (transfer none): the root node of the context's style tree
 */
EekThemeNode *
eek_theme_context_get_root_node (EekThemeContext *context)
{
  if (context->root_node == NULL)
    context->root_node = eek_theme_node_new (context, NULL, context->theme,
                                             G_TYPE_NONE, NULL, NULL, NULL, NULL);

  return context->root_node;
}
