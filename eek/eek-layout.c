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

/**
 * SECTION:eek-layout
 * @short_description: Base class of a layout engine
 *
 * The #EekLayout class is a base abstract class of layout engine
 * which arranges keyboard elements.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-layout.h"
#include "eek-keyboard.h"

G_DEFINE_ABSTRACT_TYPE (EekLayout, eek_layout, G_TYPE_INITIALLY_UNOWNED);

static void
eek_layout_finalize (GObject *object)
{
    G_OBJECT_CLASS (eek_layout_parent_class)->finalize (object);
}

static void
eek_layout_dispose (GObject *object)
{
    G_OBJECT_CLASS (eek_layout_parent_class)->dispose (object);
}

static void
eek_layout_class_init (EekLayoutClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    gobject_class->finalize     = eek_layout_finalize;
    gobject_class->dispose      = eek_layout_dispose;
}

static void
eek_layout_init (EekLayout *self)
{
}
