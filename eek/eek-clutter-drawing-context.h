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
#ifndef EEK_CLUTTER_DRAWING_CONTEXT_H
#define EEK_CLUTTER_DRAWING_CONTEXT_H 1

#include <clutter/clutter.h>
#include <pango/pango.h>

#include "eek-keysym.h"
#include "eek-types.h"

G_BEGIN_DECLS
#define EEK_TYPE_CLUTTER_DRAWING_CONTEXT (eek_clutter_drawing_context_get_type())
#define EEK_CLUTTER_DRAWING_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_CLUTTER_DRAWING_CONTEXT, EekClutterDrawingContext))
#define EEK_CLUTTER_DRAWING_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_CLUTTER_DRAWING_CONTEXT, EekClutterDrawingContextClass))
#define EEK_IS_CLUTTER_DRAWING_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_CLUTTER_DRAWING_CONTEXT))
#define EEK_IS_CLUTTER_DRAWING_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_CLUTTER_DRAWING_CONTEXT))
#define EEK_CLUTTER_DRAWING_CONTEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_CLUTTER_DRAWING_CONTEXT, EekClutterDrawingContextClass))

typedef struct _EekClutterDrawingContext        EekClutterDrawingContext;
typedef struct _EekClutterDrawingContextClass   EekClutterDrawingContextClass;
typedef struct _EekClutterDrawingContextPrivate EekClutterDrawingContextPrivate;

struct _EekClutterDrawingContext
{
    /*< private >*/
    GInitiallyUnowned parent;

    /*< private >*/
    EekClutterDrawingContextPrivate *priv;
};

struct _EekClutterDrawingContextClass
{
    /*< private >*/
    GInitiallyUnownedClass parent_class;

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType                     eek_clutter_drawing_context_get_type
                          (void) G_GNUC_CONST;
EekClutterDrawingContext *eek_clutter_drawing_context_new
                          (void);

void                      eek_clutter_drawing_context_set_outline_texture
                          (EekClutterDrawingContext *context,
                           EekOutline               *outline,
                           ClutterActor             *texture);
ClutterActor             *eek_clutter_drawing_context_get_outline_texture
                          (EekClutterDrawingContext *context,
                           EekOutline               *outline);

void                      eek_clutter_drawing_context_set_category_font
                          (EekClutterDrawingContext *context,
                           EekKeysymCategory         category,
                           PangoFontDescription     *fonts);
PangoFontDescription     *eek_clutter_drawing_context_get_category_font
                          (EekClutterDrawingContext *context,
                           EekKeysymCategory         category);

G_END_DECLS
#endif  /* EEK_CLUTTER_DRAWING_CONTEXT_H */
