/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
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

#ifndef EEK_DRAWING_CONTEXT_H
#define EEK_DRAWING_CONTEXT_H 1

#include <pango/pango.h>
#include "eek-keysym.h"
#include "eek-theme.h"
#include "eek-types.h"

G_BEGIN_DECLS

#define EEK_TYPE_DRAWING_CONTEXT (eek_drawing_context_get_type())
#define EEK_DRAWING_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_DRAWING_CONTEXT, EekDrawingContext))
#define EEK_DRAWING_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_DRAWING_CONTEXT, EekDrawingContextClass))
#define EEK_IS_DRAWING_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_DRAWING_CONTEXT))
#define EEK_IS_DRAWING_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_DRAWING_CONTEXT))
#define EEK_DRAWING_CONTEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_DRAWING_CONTEXT, EekDrawingContextClass))

typedef struct _EekDrawingContext        EekDrawingContext;
typedef struct _EekDrawingContextClass   EekDrawingContextClass;
typedef struct _EekDrawingContextPrivate EekDrawingContextPrivate;

struct _EekDrawingContext
{
    /*< private >*/
    GInitiallyUnowned parent;

    /*< private >*/
    EekDrawingContextPrivate *priv;
};

struct _EekDrawingContextClass
{
    /*< private >*/
    GInitiallyUnownedClass parent_class;

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType                 eek_drawing_context_get_type
                                              (void) G_GNUC_CONST;
EekDrawingContext    *eek_drawing_context_new (void);

void                  eek_drawing_context_set_category_font
                                              (EekDrawingContext    *context,
                                               EekKeysymCategory     category,
                                               PangoFontDescription *fonts);
PangoFontDescription *eek_drawing_context_get_category_font
                                              (EekDrawingContext    *context,
                                               EekKeysymCategory     category);

void                  eek_drawing_context_set_theme
                                              (EekDrawingContext    *context,
                                               EekTheme             *theme);

void                  eek_drawing_context_set_scale
                                              (EekDrawingContext    *context,
                                               gdouble               scale);
EekKey               *eek_drawing_context_find_key_by_position
                                              (EekDrawingContext    *context,
                                               EekKeyboard          *keyboard,
                                               gdouble               x,
                                               gdouble               y);

G_END_DECLS
#endif  /* EEK_DRAWING_CONTEXT_H */

