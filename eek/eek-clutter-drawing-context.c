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

#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-clutter-drawing-context.h"

G_DEFINE_TYPE (EekClutterDrawingContext, eek_clutter_drawing_context,
               G_TYPE_INITIALLY_UNOWNED);

#define EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_DRAWING_CONTEXT, EekClutterDrawingContextPrivate))

struct _EekClutterDrawingContextPrivate
{
    /* outline pointer -> ClutterTexture */
    GHashTable *outline_textures;

    /* keysym category -> PangoFontDescription * */
    PangoFontDescription *category_fonts[EEK_KEYSYM_CATEGORY_LAST];
};

static void
eek_clutter_drawing_context_dispose (GObject *object)
{
    EekClutterDrawingContextPrivate *priv =
        EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(object);
    if (priv->outline_textures) {
        g_hash_table_unref (priv->outline_textures);
        priv->outline_textures = NULL;
    }
}

static void
eek_clutter_drawing_context_finalize (GObject *object)
{
    EekClutterDrawingContextPrivate *priv =
        EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(object);
    gint i;

    for (i = 0; i < EEK_KEYSYM_CATEGORY_LAST; i++)
        pango_font_description_free (priv->category_fonts[i]);
}

static void
eek_clutter_drawing_context_class_init (EekClutterDrawingContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterDrawingContextPrivate));

    gobject_class->finalize = eek_clutter_drawing_context_finalize;
}

static void
eek_clutter_drawing_context_init (EekClutterDrawingContext *self)
{
    EekClutterDrawingContextPrivate *priv;

    priv = self->priv = EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(self);
    priv->outline_textures = g_hash_table_new (g_direct_hash, g_direct_equal);
    memset (priv->category_fonts, 0, sizeof *priv->category_fonts);
}

void
eek_clutter_drawing_context_set_outline_texture
 (EekClutterDrawingContext *context,
  EekOutline               *outline,
  ClutterActor             *texture)
{
    EekClutterDrawingContextPrivate *priv =
        EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(context);
    g_return_if_fail (priv);
    g_hash_table_insert (context->priv->outline_textures, outline, texture);
}

ClutterActor *
eek_clutter_drawing_context_get_outline_texture
 (EekClutterDrawingContext *context,
  EekOutline               *outline)
{
    EekClutterDrawingContextPrivate *priv =
        EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(context);
    g_return_val_if_fail (priv, NULL);
    return g_hash_table_lookup (context->priv->outline_textures, outline);
}

void
eek_clutter_drawing_context_set_category_font
 (EekClutterDrawingContext *context,
  EekKeysymCategory         category,
  PangoFontDescription     *font)
{
    EekClutterDrawingContextPrivate *priv =
        EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(context);
    g_return_if_fail (priv);
    priv->category_fonts[category] = pango_font_description_copy (font);
}

PangoFontDescription *
eek_clutter_drawing_context_get_category_font
 (EekClutterDrawingContext *context,
  EekKeysymCategory         category)
{
    EekClutterDrawingContextPrivate *priv =
        EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(context);
    g_return_val_if_fail (priv, NULL);
    return priv->category_fonts[category];
}

EekClutterDrawingContext *
eek_clutter_drawing_context_new (void)
{
    return g_object_new (EEK_TYPE_CLUTTER_DRAWING_CONTEXT, NULL);
}
