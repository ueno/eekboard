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

#include "eek-clutter-drawing-context.h"
#include "eek-element.h"
#include "eek-drawing.h"

G_DEFINE_TYPE (EekClutterDrawingContext, eek_clutter_drawing_context,
               EEK_TYPE_DRAWING_CONTEXT);

#define EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(obj)                    \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_DRAWING_CONTEXT, EekClutterDrawingContextPrivate))

struct _EekClutterDrawingContextPrivate
{
    GHashTable *texture_cache;
};

static void
eek_clutter_drawing_context_dispose (GObject *object)
{
    EekClutterDrawingContextPrivate *priv =
        EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(object);
    if (priv->texture_cache) {
        g_hash_table_unref (priv->texture_cache);
        priv->texture_cache = NULL;
    }
    G_OBJECT_CLASS (eek_clutter_drawing_context_parent_class)->dispose (object);
}

static void
eek_clutter_drawing_context_finalize (GObject *object)
{
    G_OBJECT_CLASS (eek_clutter_drawing_context_parent_class)->finalize (object);
}

static void
eek_clutter_drawing_context_class_init (EekClutterDrawingContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterDrawingContextPrivate));

    gobject_class->finalize = eek_clutter_drawing_context_finalize;
    gobject_class->dispose = eek_clutter_drawing_context_dispose;
}

static void
eek_clutter_drawing_context_init (EekClutterDrawingContext *self)
{
    EekClutterDrawingContextPrivate *priv;

    priv = self->priv = EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(self);
    priv->texture_cache = g_hash_table_new_full (eek_texture_source_hash,
                                                 eek_texture_source_equal,
                                                 eek_texture_source_free,
                                                 g_object_unref);
}

ClutterActor *
eek_clutter_drawing_context_get_texture (EekClutterDrawingContext *context,
                                         EekOutline               *outline,
                                         EekBounds                *bounds,
                                         EekThemeNode             *tnode)
{
    EekClutterDrawingContextPrivate *priv =
        EEK_CLUTTER_DRAWING_CONTEXT_GET_PRIVATE(context);
    EekGradientType gradient_type = EEK_GRADIENT_VERTICAL;
    EekColor gradient_start = {0xFF, 0xFF, 0xFF, 0xFF},
        gradient_end = {0x80, 0x80, 0x80, 0xFF};
    EekTextureSource *source;
    ClutterActor *texture;
    cairo_t *cr;

    g_return_val_if_fail (priv, NULL);
    source = g_slice_new0 (EekTextureSource);
    source->outline = outline;
    if (tnode)
        eek_theme_node_get_background_gradient (tnode,
                                                &gradient_type,
                                                &gradient_start,
                                                &gradient_end);
    source->gradient_type = gradient_type;
    memcpy (&source->gradient_start, &gradient_start, sizeof(EekColor));
    memcpy (&source->gradient_end, &gradient_end, sizeof(EekColor));
    texture = g_hash_table_lookup (priv->texture_cache, source);
    if (texture) {
        eek_texture_source_free (source);
        return clutter_clone_new (texture);
    }

    texture = clutter_cairo_texture_new (bounds->width, bounds->height);
    cr = clutter_cairo_texture_create (CLUTTER_CAIRO_TEXTURE(texture));

    if (tnode) {
        eek_theme_node_get_background_gradient (tnode,
                                                &gradient_type,
                                                &gradient_start,
                                                &gradient_end);
    }
    eek_draw_outline (cr,
                      bounds,
                      outline,
                      gradient_type,
                      &gradient_start,
                      &gradient_end);
    cairo_destroy (cr);
    g_hash_table_insert (priv->texture_cache, source, texture);
    return texture;
}

EekClutterDrawingContext *
eek_clutter_drawing_context_new (void)
{
    return g_object_new (EEK_TYPE_CLUTTER_DRAWING_CONTEXT, NULL);
}
