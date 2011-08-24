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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "eek-gtk-renderer.h"
#include "eek-key.h"

G_DEFINE_TYPE (EekGtkRenderer, eek_gtk_renderer, EEK_TYPE_RENDERER);

static cairo_surface_t *
pixbuf_to_cairo_surface (GdkPixbuf *pixbuf)
{
  cairo_surface_t *dummy_surface;
  cairo_pattern_t *pattern;
  cairo_surface_t *surface;
  cairo_t *cr;

  dummy_surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 1, 1);

  cr = cairo_create (dummy_surface);
  gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
  pattern = cairo_get_source (cr);
  cairo_pattern_get_surface (pattern, &surface);
  cairo_surface_reference (surface);
  cairo_destroy (cr);
  cairo_surface_destroy (dummy_surface);

  return surface;
}

static cairo_surface_t *
eek_gtk_renderer_real_get_icon_surface (EekRenderer *self,
                                        const gchar *icon_name,
                                        gint size)
{
    GdkPixbuf *pixbuf;
    GError *error;
    cairo_surface_t *surface;

    error = NULL;
    pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                       icon_name,
                                       size,
                                       0,
                                       &error);
    if (pixbuf == NULL)
        return NULL;

    surface = pixbuf_to_cairo_surface (pixbuf);
    g_object_unref (pixbuf);
    return surface;
}

static void
eek_gtk_renderer_class_init (EekGtkRendererClass *klass)
{
    EekRendererClass *renderer_class = EEK_RENDERER_CLASS (klass);

    renderer_class->get_icon_surface = eek_gtk_renderer_real_get_icon_surface;
}

static void
eek_gtk_renderer_init (EekGtkRenderer *self)
{
}

EekRenderer *
eek_gtk_renderer_new (EekKeyboard  *keyboard,
                      PangoContext *pcontext,
                      GtkWidget    *widget)
{
    EekRenderer *renderer;

    renderer = g_object_new (EEK_TYPE_GTK_RENDERER,
                             "keyboard", keyboard,
                             "pango-context", pcontext,
                             NULL);

    return renderer;
}
