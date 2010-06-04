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
#ifndef EEK_PRIVATE_H
#define EEK_PRIVATE_H 1

#include <glib/gtypes.h>
#include <cairo/cairo.h>
#include "eek-types.h"

G_BEGIN_DECLS

void eek_cairo_draw_rounded_polygon (cairo_t * cr,
                                      gboolean filled,
                                      gdouble radius,
                                      EekPoint * points,
                                      gint num_points);

G_END_DECLS
#endif  /* EEK_PRIVATE_H */
