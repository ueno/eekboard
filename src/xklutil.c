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
#include <string.h>
#include "xklutil.h"

XklConfigRec *
eekboard_xkl_config_rec_new_from_string (const gchar *layouts)
{
    XklConfigRec *rec;
    gchar **l, **v;
    gint i;

    l = g_strsplit (layouts, ",", -1);
    v = g_strdupv (l);
    for (i = 0; l[i]; i++) {
        gchar *layout = l[i], *variant = v[i],
            *variant_start, *variant_end;

        variant_start = strchr (layout, '(');
        variant_end = strrchr (layout, ')');
        if (variant_start && variant_end) {
            *variant_start++ = '\0';
            g_strlcpy (variant, variant_start,
                       variant_end - variant_start + 1);
        } else
            *variant = '\0';
    }

    rec = xkl_config_rec_new ();
    rec->layouts = l;
    rec->variants = v;

    return rec;
}

static XklConfigItem *
xkl_config_item_copy (const XklConfigItem *item)
{
    XklConfigItem *_item = xkl_config_item_new ();
    memcpy (_item->name,
            item->name,
            sizeof (item->name));
    memcpy (_item->short_description,
            item->short_description,
            sizeof (item->short_description));
    memcpy (_item->description,
            item->description,
            sizeof (item->description));
    return _item;
}

static void
prepend_item (XklConfigRegistry   *registry,
              const XklConfigItem *item,
              gpointer             data)
{
    GSList **list = data;
    XklConfigItem *_item = xkl_config_item_copy (item);
    *list = g_slist_prepend (*list, _item);
}

static gint
compare_item_by_name (gconstpointer a, gconstpointer b)
{
    const XklConfigItem *ia = a, *ib = b;
    return g_strcmp0 (ia->name, ib->name);
}

GSList *
eekboard_xkl_list_models (XklConfigRegistry *registry)
{
    GSList *list = NULL;
    xkl_config_registry_foreach_model (registry, prepend_item, &list);
    return g_slist_sort (list, compare_item_by_name);
}

GSList *
eekboard_xkl_list_layouts (XklConfigRegistry *registry)
{
    GSList *list = NULL;
    xkl_config_registry_foreach_layout (registry, prepend_item, &list);
    return g_slist_sort (list, compare_item_by_name);
}

GSList *
eekboard_xkl_list_option_groups (XklConfigRegistry *registry)
{
    GSList *list = NULL;
    xkl_config_registry_foreach_option_group (registry, prepend_item, &list);
    return g_slist_sort (list, compare_item_by_name);
}

GSList *
eekboard_xkl_list_layout_variants (XklConfigRegistry *registry,
                                   const gchar       *layout)
{
    GSList *list = NULL;
    xkl_config_registry_foreach_layout_variant (registry,
                                                layout,
                                                prepend_item,
                                                &list);
    return g_slist_sort (list, compare_item_by_name);
}

GSList *
eekboard_xkl_list_options (XklConfigRegistry *registry,
                           const gchar       *group)
{
    GSList *list = NULL;
    xkl_config_registry_foreach_option (registry, group, prepend_item, &list);
    return g_slist_sort (list, compare_item_by_name);
}
