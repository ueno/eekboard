#ifndef __EEK_THEME_NODE_PRIVATE_H__
#define __EEK_THEME_NODE_PRIVATE_H__

#include <libcroco/libcroco.h>
#include "eek-theme-node.h"

struct _EekThemeNode {
  GObject parent;

  EekThemeNode *parent_node;
  EekTheme *theme;

  EekColor background_color;
  /* If gradient is set, then background_color is the gradient start */
  EekGradientType background_gradient_type;
  EekColor background_gradient_end;

  EekColor foreground_color;

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

struct _EekThemeNodeClass {
  GObjectClass parent_class;
};

#endif /* __EEK_THEME_NODE_H__ */
