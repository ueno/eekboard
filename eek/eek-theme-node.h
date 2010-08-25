/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
#ifndef __EEK_THEME_NODE_H__
#define __EEK_THEME_NODE_H__

#include <pango/pango.h>
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

typedef struct _EekTheme          EekTheme;
typedef struct _EekThemeContext   EekThemeContext;

typedef struct _EekThemeNode      EekThemeNode;
typedef struct _EekThemeNodeClass EekThemeNodeClass;

#define EEK_TYPE_THEME_NODE (eek_theme_node_get_type())
#define EEK_THEME_NODE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_THEME_NODE, EekThemeNode))
#define EEK_THEME_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_THEME_NODE, EekThemeNodeClass))
#define EEK_IS_THEME_NODE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_THEME_NODE))
#define EEK_IS_THEME_NODE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_THEME_NODE))
#define EEK_THEME_NODE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_THEME_NODE, EekThemeNodeClass))

GType eek_theme_node_get_type (void) G_GNUC_CONST;

EekThemeNode *eek_theme_node_new (EekThemeNode    *parent_node,   /* can be null */
                                  EekTheme        *theme,         /* can be null */
                                  GType           element_type,
                                  const char     *element_id,
                                  const char     *element_class,
                                  const char     *pseudo_class,
                                  const char     *inline_style);

EekThemeNode *eek_theme_node_get_parent (EekThemeNode *node);

EekTheme *eek_theme_node_get_theme (EekThemeNode *node);

gboolean    eek_theme_node_equal (EekThemeNode *node_a, EekThemeNode *node_b);

GType       eek_theme_node_get_element_type  (EekThemeNode *node);
const char *eek_theme_node_get_element_id    (EekThemeNode *node);
const char *eek_theme_node_get_element_class (EekThemeNode *node);
const char *eek_theme_node_get_pseudo_class  (EekThemeNode *node);

/* Generic getters ... these are not cached so are less efficient. The other
 * reason for adding the more specific version is that we can handle the
 * details of the actual CSS rules, which can be complicated, especially
 * for fonts
 */
gboolean eek_theme_node_get_color  (EekThemeNode  *node,
                                    const char   *property_name,
                                    gboolean      inherit,
                                    EekColor *color);

/* Specific getters for particular properties: cached
 */
void eek_theme_node_get_background_color (EekThemeNode  *node,
                                          EekColor *color);
void eek_theme_node_get_foreground_color (EekThemeNode  *node,
                                          EekColor *color);
void eek_theme_node_get_background_gradient (EekThemeNode   *node,
                                             EekGradientType *type,
                                             EekColor   *start,
                                             EekColor   *end);

G_END_DECLS

#endif /* __EEK_THEME_NODE_H__ */
