/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
#ifndef __EEK_THEME_PRIVATE_H__
#define __EEK_THEME_PRIVATE_H__

#include <libcroco/libcroco.h>
#include "eek-theme.h"

G_BEGIN_DECLS

GPtrArray *_eek_theme_get_matched_properties (EekTheme       *theme,
                                             EekThemeNode   *node);

/* Resolve an URL from the stylesheet to a filename */
char *_eek_theme_resolve_url (EekTheme      *theme,
                             CRStyleSheet *base_stylesheet,
                             const char   *url);

CRDeclaration *_eek_theme_parse_declaration_list (const char *str);

G_END_DECLS

#endif /* __EEK_THEME_PRIVATE_H__ */
