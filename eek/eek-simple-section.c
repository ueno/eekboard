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
#include "eek-simple-section.h"
#include <string.h>

enum {
    PROP_0,
    PROP_NAME,
    PROP_COLUMNS,
    PROP_ROWS,
    PROP_ANGLE,
    PROP_BOUNDS,
    PROP_LAST
};

static void eek_section_iface_init (EekSectionIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekSimpleSection, eek_simple_section,
                         G_TYPE_INITIALLY_UNOWNED,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_SECTION,
                                                eek_section_iface_init));

#define EEK_SIMPLE_SECTION_GET_PRIVATE(obj)                           \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_SIMPLE_SECTION, EekSimpleSectionPrivate))

struct _EekSimpleSectionPrivate
{
    gchar *name;
    gint num_columns;
    gint num_rows;
    gint angle;
    EekBounds bounds;
    GSList *keys;
};

static void
eek_simple_section_real_set_dimensions (EekSection *self,
                                        gint        columns,
                                        gint        rows)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    priv->num_columns = columns;
    priv->num_rows = rows;
}

static void
eek_simple_section_real_get_dimensions (EekSection *self,
                                        gint       *columns,
                                        gint       *rows)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    *columns = priv->num_columns;
    *rows = priv->num_rows;
}

static void
eek_simple_section_real_set_angle (EekSection *self,
                                   gint        angle)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    priv->angle = angle;
}

static gint
eek_simple_section_real_get_angle (EekSection *self)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, 0);
    return priv->angle;
}

static void
eek_simple_section_real_set_bounds (EekSection *self,
                                    EekBounds *bounds)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_return_if_fail (bounds);
    priv->bounds = *bounds;
}

static void
eek_simple_section_real_get_bounds (EekSection *self, EekBounds *bounds)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_return_if_fail (bounds);
    priv->bounds = *bounds;
}

static EekKey *
eek_simple_section_real_create_key (EekSection  *self,
                                    const gchar *name,
                                    guint       *keysyms,
                                    gint         num_groups,
                                    gint         num_levels,
                                    gint         column,
                                    gint         row,
                                    EekOutline  *outline,
                                    EekBounds   *bounds)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(self);
    EekKey *key;
    EekKeysymMatrix matrix;

    g_return_val_if_fail (priv, NULL);
    g_return_val_if_fail (column < priv->num_columns, NULL);
    g_return_val_if_fail (row < priv->num_rows, NULL);

    matrix.data = keysyms;
    matrix.num_groups = num_groups;
    matrix.num_levels = num_levels;
    key = g_object_new (EEK_TYPE_SIMPLE_KEY,
                        "name", name,
                        "keysyms", &matrix,
                        "column", column,
                        "row", row,
                        "outline", outline,
                        "bounds", bounds,
                        NULL);
    g_return_val_if_fail (key, NULL);
    priv->keys = g_slist_prepend (priv->keys, key);
    return key;
}

static void
eek_simple_section_real_foreach_key (EekSection *self,
                                     GFunc       func,
                                     gpointer    user_data)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_slist_foreach (priv->keys, func, user_data);
}

static void
eek_section_iface_init (EekSectionIface *iface)
{
    iface->set_dimensions = eek_simple_section_real_set_dimensions;
    iface->get_dimensions = eek_simple_section_real_get_dimensions;
    iface->set_angle = eek_simple_section_real_set_angle;
    iface->get_angle = eek_simple_section_real_get_angle;
    iface->set_bounds = eek_simple_section_real_set_bounds;
    iface->get_bounds = eek_simple_section_real_get_bounds;
    iface->create_key = eek_simple_section_real_create_key;
    iface->foreach_key = eek_simple_section_real_foreach_key;
}

static void
eek_simple_section_dispose (GObject *object)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(object);
    GSList *head;

    for (head = priv->keys; head; head = g_slist_next (head))
        g_object_unref (head->data);
    
    G_OBJECT_CLASS (eek_simple_section_parent_class)->dispose (object);
}

static void
eek_simple_section_finalize (GObject *object)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(object);

    g_free (priv->name);
    g_slist_free (priv->keys);
    G_OBJECT_CLASS (eek_simple_section_parent_class)->finalize (object);
}

static void
eek_simple_section_set_property (GObject    *object,
                                 guint       prop_id,
                                 const GValue     *value,
                                 GParamSpec *pspec)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_NAME:
        g_free (priv->name);
        priv->name = g_strdup (g_value_get_string (value));
        break;
    case PROP_ANGLE:
        eek_section_set_angle (EEK_SECTION(object),
                               g_value_get_int (value));
        break;
    case PROP_BOUNDS:
        eek_section_set_bounds (EEK_SECTION(object),
                                g_value_get_boxed (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_simple_section_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    EekSimpleSectionPrivate *priv = EEK_SIMPLE_SECTION_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;
    case PROP_ANGLE:
        g_value_set_int (value, eek_section_get_angle (EEK_SECTION(object)));
        break;
    case PROP_BOUNDS:
        g_value_set_boxed (value, &priv->bounds);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_simple_section_class_init (EekSimpleSectionClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class, sizeof (EekSimpleSectionPrivate));

    gobject_class->set_property = eek_simple_section_set_property;
    gobject_class->get_property = eek_simple_section_get_property;
    gobject_class->finalize     = eek_simple_section_finalize;
    gobject_class->dispose      = eek_simple_section_dispose;

    g_object_class_override_property (gobject_class,
                                      PROP_NAME,
                                      "name");
    g_object_class_override_property (gobject_class,
                                      PROP_ANGLE,
                                      "angle");
    g_object_class_override_property (gobject_class,
                                      PROP_BOUNDS,
                                      "bounds");
}

static void
eek_simple_section_init (EekSimpleSection *self)
{
    EekSimpleSectionPrivate *priv;

    priv = self->priv = EEK_SIMPLE_SECTION_GET_PRIVATE (self);
    priv->angle = 0;
    memset (&priv->bounds, 0, sizeof priv->bounds);
    priv->keys = NULL;
}
