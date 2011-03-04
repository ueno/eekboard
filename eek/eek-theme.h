/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
#ifndef __EEK_THEME_H__
#define __EEK_THEME_H__

#include <glib-object.h>

#include "eek-theme-node.h"

G_BEGIN_DECLS

/**
 * SECTION:EekTheme
 * @short_description: a set of stylesheets
 *
 * #EekTheme holds a set of stylesheets. (The "cascade" of the name
 * Cascading Stylesheets.) An #EekTheme can be set to apply to all the
 * keyboard elements.
 */

typedef struct _EekThemeClass EekThemeClass;

#define EEK_TYPE_THEME (eek_theme_get_type())
#define EEK_THEME(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_THEME, EekTheme))
#define EEK_THEME_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_THEME, EekThemeClass))
#define EEK_IS_THEME(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_THEME))
#define EEK_IS_THEME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_THEME))
#define EEK_THEME_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_THEME, EekThemeClass))

GType     eek_theme_get_type          (void) G_GNUC_CONST;

EekTheme *eek_theme_new               (const char *application_stylesheet,
                                       const char *theme_stylesheet,
                                       const char *default_stylesheet);

gboolean  eek_theme_load_stylesheet   (EekTheme   *theme,
                                       const char *path,
                                       GError    **error);

void      eek_theme_unload_stylesheet (EekTheme   *theme,
                                       const char *path);

G_END_DECLS

#endif /* __EEK_THEME_H__ */
