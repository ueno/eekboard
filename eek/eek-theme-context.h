/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
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

#ifndef __EEK_THEME_CONTEXT_H__
#define __EEK_THEME_CONTEXT_H__

#include <pango/pango.h>
#include "eek-theme-node.h"

G_BEGIN_DECLS

/**
 * SECTION:EekThemeContext
 * @short_description: holds global information about a tree of styled objects
 *
 * #EekThemeContext is responsible for managing information global to
 * a tree of styled objects, such as the set of stylesheets or the
 * default font.
 */

typedef struct _EekThemeContextClass EekThemeContextClass;

#define EEK_TYPE_THEME_CONTEXT             (eek_theme_context_get_type ())
#define EEK_THEME_CONTEXT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), EEK_TYPE_THEME_CONTEXT, EekThemeContext))
#define EEK_THEME_CONTEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_THEME_CONTEXT, EekThemeContextClass))
#define EEK_IS_THEME_CONTEXT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object), EEK_TYPE_THEME_CONTEXT))
#define EEK_IS_THEME_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_THEME_CONTEXT))
#define EEK_THEME_CONTEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_THEME_CONTEXT, EekThemeContextClass))

GType                       eek_theme_context_get_type
                            (void) G_GNUC_CONST;

EekThemeContext            *eek_theme_context_new
                            (void);

void                        eek_theme_context_set_theme
                            (EekThemeContext            *context,
                             EekTheme                   *theme);
EekTheme *                  eek_theme_context_get_theme
                            (EekThemeContext            *context);

void                        eek_theme_context_set_resolution
                            (EekThemeContext            *context,
                             gdouble                     resolution);
void                        eek_theme_context_set_default_resolution
                            (EekThemeContext            *context);
double                      eek_theme_context_get_resolution
                            (EekThemeContext            *context);
void                        eek_theme_context_set_font
                            (EekThemeContext            *context,
                             const PangoFontDescription *font);
const PangoFontDescription *eek_theme_context_get_font
                            (EekThemeContext            *context);

EekThemeNode *              eek_theme_context_get_root_node
                            (EekThemeContext            *context);

G_END_DECLS

#endif /* __EEK_THEME_CONTEXT_H__ */
