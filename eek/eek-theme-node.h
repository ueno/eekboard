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
#ifndef __EEK_THEME_NODE_H__
#define __EEK_THEME_NODE_H__

#include "eek-types.h"

G_BEGIN_DECLS

/**
 * SECTION:EekThemeNode
 * @short_description: style information for one node in a tree of
 * themed objects
 *
 * A #EekThemeNode represents the CSS style information (the set of
 * CSS properties) for one node in a tree of themed objects. In
 * typical usage, it represents the style information for a single
 * #EekElement. A #EekThemeNode is immutable: attributes such as the
 * CSS classes for the node are passed in at construction. If the
 * attributes of the node or any parent node change, the node should
 * be discarded and a new node created.  #EekThemeNode has generic
 * accessors to look up properties by name and specific accessors for
 * standard CSS properties that add caching and handling of various
 * details of the CSS specification. #EekThemeNode also has
 * convenience functions to help in implementing a #EekElement with
 * borders and padding.
 */

typedef enum {
    EEK_SIDE_TOP,
    EEK_SIDE_RIGHT,
    EEK_SIDE_BOTTOM,
    EEK_SIDE_LEFT
} EekSide;

typedef enum {
    EEK_CORNER_TOPLEFT,
    EEK_CORNER_TOPRIGHT,
    EEK_CORNER_BOTTOMRIGHT,
    EEK_CORNER_BOTTOMLEFT
} EekCorner;

#define EEK_TYPE_THEME_NODE (eek_theme_node_get_type())
#define EEK_THEME_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_THEME_NODE, EekThemeNode))
#define EEK_THEME_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_THEME_NODE, EekThemeNodeClass))
#define EEK_IS_THEME_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_THEME_NODE))
#define EEK_IS_THEME_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_THEME_NODE))
#define EEK_THEME_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_THEME_NODE, EekThemeNodeClass))

typedef struct _EekThemeNodeClass EekThemeNodeClass;
typedef struct _EekThemeNodePrivate EekThemeNodePrivate;

struct _EekThemeNode {
    GObject parent;

    EekThemeNodePrivate *priv;
};

struct _EekThemeNodeClass {
  GObjectClass parent_class;
};

GType         eek_theme_node_get_type
                                 (void) G_GNUC_CONST;

EekThemeNode *eek_theme_node_new (EekThemeNode               *parent_node,
                                  /* can be null */ EekTheme *theme,
                                  /* can be null */ GType     element_type,
                                  const char                 *element_id,
                                  const char                 *element_class,
                                  const char                 *pseudo_class,
                                  const char                 *inline_style);

EekThemeNode *eek_theme_node_get_parent
                                 (EekThemeNode               *node);

EekTheme     *eek_theme_node_get_theme
                                 (EekThemeNode               *node);

GType         eek_theme_node_get_element_type
                                 (EekThemeNode               *node);
const char   *eek_theme_node_get_element_id
                                 (EekThemeNode               *node);
const char   *eek_theme_node_get_element_class
                                 (EekThemeNode               *node);
const char   *eek_theme_node_get_pseudo_class
                                 (EekThemeNode               *node);

/* Generic getters ... these are not cached so are less efficient. The other
 * reason for adding the more specific version is that we can handle the
 * details of the actual CSS rules, which can be complicated, especially
 * for fonts
 */
void          eek_theme_node_get_color
                                 (EekThemeNode               *node,
                                  const char                 *property_name,
                                  EekColor                   *color);

/* Specific getters for particular properties: cached
 */
void          eek_theme_node_get_background_color
                                 (EekThemeNode               *node,
                                  EekColor                   *color);
void          eek_theme_node_get_foreground_color
                                 (EekThemeNode               *node,
                                  EekColor                   *color);
void          eek_theme_node_get_background_gradient
                                 (EekThemeNode               *node,
                                  EekGradientType            *type,
                                  EekColor                   *start,
                                  EekColor                   *end);
int           eek_theme_node_get_border_width
                                 (EekThemeNode               *node,
                                  EekSide                     side);
int           eek_theme_node_get_border_radius
                                 (EekThemeNode               *node,
                                  EekCorner                   corner);
void          eek_theme_node_get_border_color
                                 (EekThemeNode               *node,
                                  EekSide                     side,
                                  EekColor                   *color);

G_END_DECLS

#endif /* __EEK_THEME_NODE_H__ */
