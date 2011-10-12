/* 
 * Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2011 Red Hat, Inc.
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

#if !defined(__EEK_H_INSIDE__) && !defined(EEK_COMPILATION)
#error "Only <eek/eek.h> can be included directly."
#endif

#ifndef EEK_TEXT_H
#define EEK_TEXT_H 1

#include "eek-symbol.h"

G_BEGIN_DECLS

#define EEK_TYPE_TEXT (eek_text_get_type())
#define EEK_TEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_TEXT, EekText))
#define EEK_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_TEXT, EekTextClass))
#define EEK_IS_TEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_TEXT))
#define EEK_IS_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_TEXT))
#define EEK_TEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_TEXT, EekTextClass))

typedef struct _EekTextClass EekTextClass;
typedef struct _EekTextPrivate EekTextPrivate;

/**
 * EekText:
 *
 * The #EekText structure contains only private data and should only
 * be accessed using the provided API.
 */
struct _EekText {
    /*< private >*/
    EekSymbol parent;

    EekTextPrivate *priv;
};

struct _EekTextClass {
    /*< private >*/
    EekSymbolClass parent_class;
};

GType        eek_text_get_type (void) G_GNUC_CONST;
EekText     *eek_text_new      (const gchar *text);
const gchar *eek_text_get_text (EekText     *text);

G_END_DECLS

#endif  /* EEK_TEXT_H */
