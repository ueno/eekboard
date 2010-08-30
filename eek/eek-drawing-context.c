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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <string.h>
#include "eek-drawing-context.h"

G_DEFINE_TYPE (EekDrawingContext, eek_drawing_context,
               G_TYPE_INITIALLY_UNOWNED);

#define EEK_DRAWING_CONTEXT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_DRAWING_CONTEXT, EekDrawingContextPrivate))

struct _EekDrawingContextPrivate
{
    /* keysym category -> PangoFontDescription * */
    PangoFontDescription *category_fonts[EEK_KEYSYM_CATEGORY_LAST];

    EekTheme *theme;

    gdouble scale;
};

static void
eek_drawing_context_dispose (GObject *object)
{
    G_OBJECT_CLASS(eek_drawing_context_parent_class)->dispose (object);
}

static void
eek_drawing_context_finalize (GObject *object)
{
    EekDrawingContextPrivate *priv =
        EEK_DRAWING_CONTEXT_GET_PRIVATE(object);
    gint i;

    for (i = 0; i < G_N_ELEMENTS(priv->category_fonts); i++)
        pango_font_description_free (priv->category_fonts[i]);

    G_OBJECT_CLASS(eek_drawing_context_parent_class)->finalize (object);
}

static void
eek_drawing_context_class_init (EekDrawingContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekDrawingContextPrivate));

    gobject_class->finalize = eek_drawing_context_finalize;
    gobject_class->dispose = eek_drawing_context_dispose;
}

static void
eek_drawing_context_init (EekDrawingContext *self)
{
    EekDrawingContextPrivate *priv;

    priv = self->priv = EEK_DRAWING_CONTEXT_GET_PRIVATE(self);
    memset (priv->category_fonts, 0, sizeof *priv->category_fonts);
}

void
eek_drawing_context_set_category_font (EekDrawingContext    *context,
                                       EekKeysymCategory     category,
                                       PangoFontDescription *font)
{
    EekDrawingContextPrivate *priv =
        EEK_DRAWING_CONTEXT_GET_PRIVATE(context);
    g_return_if_fail (priv);
    priv->category_fonts[category] = pango_font_description_copy (font);
}

PangoFontDescription *
eek_drawing_context_get_category_font (EekDrawingContext *context,
                                       EekKeysymCategory  category)
{
    EekDrawingContextPrivate *priv =
        EEK_DRAWING_CONTEXT_GET_PRIVATE(context);
    g_return_val_if_fail (priv, NULL);
    return priv->category_fonts[category];
}
