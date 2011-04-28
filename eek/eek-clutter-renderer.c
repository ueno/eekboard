/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
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
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cogl/cogl.h>
#include <cogl/cogl-pango.h>
#include <clutter/clutter.h>

#include "eek-clutter-renderer.h"
#include "eek-key.h"

G_DEFINE_TYPE (EekClutterRenderer, eek_clutter_renderer, EEK_TYPE_RENDERER);

#define EEK_CLUTTER_RENDERER_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_RENDERER, EekClutterRendererPrivate))

struct _EekClutterRendererPrivate
{
    GHashTable *outline_texture_cache;
};

/* This routine is copied from librsvg:
   Copyright © 2005 Dom Lachowicz <cinamod@hotmail.com>
   Copyright © 2005 Caleb Moore <c.moore@student.unsw.edu.au>
   Copyright © 2005 Red Hat, Inc.
 */
static void
cairo_pixels_to_pixbuf (guint8 *pixels,
                        int     rowstride,
                        int     height)
{
    int row;

    /* un-premultiply data */
    for (row = 0; row < height; row++) {
        guint8 *row_data = (pixels + (row * rowstride));
        int i;

        for (i = 0; i < rowstride; i += 4) {
            guint8 *b = &row_data[i];
            guint32 pixel;
            guint8 alpha;

            memcpy (&pixel, b, sizeof (guint32));
            alpha = (pixel & 0xff000000) >> 24;
            if (alpha == 0) {
                b[0] = b[1] = b[2] = b[3] = 0;
            } else {
                b[0] = (((pixel & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
                b[1] = (((pixel & 0x00ff00) >> 8) * 255 + alpha / 2) / alpha;
                b[2] = (((pixel & 0x0000ff) >> 0) * 255 + alpha / 2) / alpha;
                b[3] = alpha;
            }
        }
    }
}

static void
eek_clutter_renderer_real_invalidate (EekRenderer *self)
{
    EekClutterRendererPrivate *priv = EEK_CLUTTER_RENDERER_GET_PRIVATE(self);

    if (priv->outline_texture_cache)
        g_hash_table_remove_all (priv->outline_texture_cache);

    EEK_RENDERER_CLASS (eek_clutter_renderer_parent_class)->invalidate (self);
}

static void
eek_clutter_renderer_finalize (GObject *object)
{
    EekClutterRendererPrivate *priv = EEK_CLUTTER_RENDERER_GET_PRIVATE(object);
    g_hash_table_destroy (priv->outline_texture_cache);
    G_OBJECT_CLASS (eek_clutter_renderer_parent_class)->finalize (object);
}

static void
eek_clutter_renderer_class_init (EekClutterRendererClass *klass)
{
    EekRendererClass  *renderer_class = EEK_RENDERER_CLASS (klass);
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterRendererPrivate));

    renderer_class->invalidate = eek_clutter_renderer_real_invalidate;

    gobject_class->finalize = eek_clutter_renderer_finalize;
}

static void
eek_clutter_renderer_init (EekClutterRenderer *self)
{
    EekClutterRendererPrivate *priv;

    priv = self->priv = EEK_CLUTTER_RENDERER_GET_PRIVATE(self);
    priv->outline_texture_cache =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               cogl_handle_unref);
}

void
eek_clutter_renderer_render_key (EekClutterRenderer *renderer,
                                 ClutterActor       *actor,
                                 EekKey             *key)
{
    EekClutterRendererPrivate *priv;
    EekOutline *outline;
    CoglHandle *outline_texture;
    PangoLayout *layout;
    PangoRectangle extents = { 0, };
    EekColor foreground;
    CoglColor color;
    ClutterGeometry geom;
    gulong oref;
    EekKeyboard *keyboard;

    g_assert (EEK_IS_CLUTTER_RENDERER(renderer));
    g_assert (CLUTTER_IS_ACTOR(actor));
    g_assert (EEK_IS_KEY(key));

    oref = eek_key_get_oref (key);
    g_object_get (renderer, "keyboard", &keyboard, NULL);
    outline = eek_keyboard_get_outline (keyboard, oref);
    g_object_unref (keyboard);

    priv = EEK_CLUTTER_RENDERER_GET_PRIVATE(renderer);
    outline_texture = g_hash_table_lookup (priv->outline_texture_cache,
                                           outline);
    if (!outline_texture) {
        gint rowstride;
        guint8 *data;
        cairo_surface_t *surface;
        cairo_t *cr;
        EekBounds bounds;
        gdouble scale;
        GdkPixbuf *pixbuf;

        eek_element_get_bounds (EEK_ELEMENT(key), &bounds);
        scale = eek_renderer_get_scale (EEK_RENDERER(renderer));
        rowstride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32,
                                                   bounds.width * scale);

        data = g_malloc0 (rowstride * bounds.height);
        surface = cairo_image_surface_create_for_data (data,
                                                       CAIRO_FORMAT_ARGB32,
                                                       bounds.width * scale,
                                                       bounds.height * scale,
                                                       rowstride);
        cr = cairo_create (surface);
        eek_renderer_render_key_outline (EEK_RENDERER(renderer),
                                         cr,
                                         key,
                                         1.0,
                                         FALSE);
        cairo_destroy (cr);
        cairo_surface_destroy (surface);
        cairo_pixels_to_pixbuf (data, rowstride, bounds.height * scale);

        pixbuf = gdk_pixbuf_new_from_data (data,
                                           GDK_COLORSPACE_RGB,
                                           TRUE,
                                           8,
                                           bounds.width * scale,
                                           bounds.height * scale,
                                           rowstride,
                                           (GdkPixbufDestroyNotify) g_free,
                                           data);

        outline_texture =
            cogl_texture_new_from_data (gdk_pixbuf_get_width (pixbuf),
                                        gdk_pixbuf_get_height (pixbuf),
                                        COGL_TEXTURE_NONE,
                                        gdk_pixbuf_get_has_alpha (pixbuf)
                                        ? COGL_PIXEL_FORMAT_RGBA_8888
                                        : COGL_PIXEL_FORMAT_RGB_888,
                                        COGL_PIXEL_FORMAT_ANY,
                                        gdk_pixbuf_get_rowstride (pixbuf),
                                        gdk_pixbuf_get_pixels (pixbuf));
        g_object_unref (pixbuf);

        g_hash_table_insert (priv->outline_texture_cache,
                             outline,
                             outline_texture);
    }

    clutter_actor_get_allocation_geometry (actor, &geom);
    cogl_set_source_texture (outline_texture);
    cogl_rectangle (0.0f, 0.0f, geom.width, geom.height);

    layout = eek_renderer_create_pango_layout (EEK_RENDERER(renderer));
    eek_renderer_render_key_label (EEK_RENDERER(renderer), layout, key);
    pango_layout_get_extents (layout, NULL, &extents);

    eek_renderer_get_foreground_color (EEK_RENDERER(renderer),
                                       EEK_ELEMENT(key),
                                       &foreground);

    cogl_color_set_from_4f (&color,
                            foreground.red,
                            foreground.green,
                            foreground.blue,
                            foreground.alpha);

    cogl_pango_render_layout (layout,
                              (geom.width - extents.width / PANGO_SCALE) / 2,
                              (geom.height - extents.height / PANGO_SCALE) / 2,
                              &color,
                              0);
    g_object_unref (layout);
}

EekClutterRenderer *
eek_clutter_renderer_new (EekKeyboard        *keyboard,
                          PangoContext       *pcontext)
{
    EekClutterRenderer *renderer;

    renderer = g_object_new (EEK_TYPE_CLUTTER_RENDERER,
                             "keyboard", keyboard,
                             "pango-context", pcontext,
                             NULL);

    return renderer;
}
