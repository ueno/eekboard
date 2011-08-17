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
#ifndef EEKBOARD_XKLUTIL_H
#define EEKBOARD_XKLUTIL_H 1

#include <libxklavier/xklavier.h>

G_BEGIN_DECLS

XklConfigRec *eekboard_xkl_config_rec_from_string (const gchar       *layouts);
gchar        *eekboard_xkl_config_rec_to_string   (XklConfigRec      *rec);

GSList       *eekboard_xkl_list_models            (XklConfigRegistry *registry);
GSList       *eekboard_xkl_list_layouts           (XklConfigRegistry *registry);
GSList       *eekboard_xkl_list_option_groups     (XklConfigRegistry *registry);
GSList       *eekboard_xkl_list_layout_variants   (XklConfigRegistry *registry,
                                                   const gchar       *layout);
GSList       *eekboard_xkl_list_options           (XklConfigRegistry *registry,
                                                   const gchar       *group);

G_END_DECLS
#endif  /* EEKBOARD_XKLUTIL_H */
