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
 * SECTION:eek-clutter-section
 * @short_description: #EekSection implemented as a #ClutterActor
 *
 * The #EekClutterSection class implements the #EekSectionIface
 * interface as a #ClutterActor.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-clutter-section.h"
#include "eek-clutter-private.h"
#include "eek-simple-section.h"
#include <stdio.h>

enum {
    PROP_0,
    PROP_COLUMNS,
    PROP_ROWS,
    PROP_ANGLE,
    PROP_BOUNDS,
    PROP_LAST
};

static void eek_section_iface_init (EekSectionIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekClutterSection, eek_clutter_section,
                         CLUTTER_TYPE_GROUP,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_SECTION,
                                                eek_section_iface_init));

#define EEK_CLUTTER_SECTION_GET_PRIVATE(obj)                           \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_SECTION, EekClutterSectionPrivate))

struct _EekClutterSectionPrivate
{
    EekSimpleSection *simple;
    GHashTable *key_outline_texture_hash; /* outline pointer -> texture */
};

static void
eek_clutter_section_real_set_rows (EekSection *self,
                                   gint        rows)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_rows (EEK_SECTION(priv->simple), rows);
}

static gint
eek_clutter_section_real_get_rows (EekSection *self)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, -1);
    return eek_section_get_rows (EEK_SECTION(priv->simple));
}

static void
eek_clutter_section_real_set_columns (EekSection *self,
                                      gint        row,
                                      gint        columns)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_columns (EEK_SECTION(priv->simple), row, columns);
}

static gint
eek_clutter_section_real_get_columns (EekSection *self,
                                      gint        row)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, -1);
    return eek_section_get_columns (EEK_SECTION(priv->simple), row);
}

static void
eek_clutter_section_real_set_orientation (EekSection    *self,
                                          gint           row,
                                          EekOrientation orientation)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_orientation (EEK_SECTION(priv->simple), row, orientation);
}

static EekOrientation
eek_clutter_section_real_get_orientation (EekSection *self,
                                          gint        row)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, EEK_ORIENTATION_INVALID);
    return eek_section_get_orientation (EEK_SECTION(priv->simple), row);
}

static void
eek_clutter_section_real_set_angle (EekSection *self,
                                    gint        angle)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_angle (EEK_SECTION(priv->simple), angle);
    clutter_actor_set_rotation (CLUTTER_ACTOR(self),
                                CLUTTER_Z_AXIS,
                                angle,
                                0,
                                0,
                                0);
}

static gint
eek_clutter_section_real_get_angle (EekSection *self)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, 0);
    eek_section_get_angle (EEK_SECTION(priv->simple));
}

static void
eek_clutter_section_real_set_bounds (EekSection *self,
                                     EekBounds  *bounds)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_bounds (EEK_SECTION(priv->simple), bounds);
    clutter_actor_set_position (CLUTTER_ACTOR(self), bounds->x, bounds->y);
    clutter_actor_set_size (CLUTTER_ACTOR(self), bounds->width, bounds->height);
}

static void
eek_clutter_section_real_get_bounds (EekSection *self,
                                     EekBounds  *bounds)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    return eek_section_get_bounds (EEK_SECTION(priv->simple), bounds);
}

static EekKey *
eek_clutter_section_real_create_key (EekSection  *self,
                                     const gchar *name,
                                     guint        keycode,
                                     guint       *keysyms,
                                     gint         num_groups,
                                     gint         num_levels,
                                     gint         column,
                                     gint         row,
                                     EekOutline  *outline,
                                     EekBounds   *bounds)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);
    EekKey *key;
    EekKeysymMatrix matrix;
    gint columns, rows;
    ClutterActor *texture;

    g_return_val_if_fail (priv, NULL);

    rows = eek_section_get_rows (self);
    g_return_val_if_fail (0 <= row && row < rows, NULL);
    columns = eek_section_get_columns (self, row);
    g_return_val_if_fail (column < columns, NULL);

    matrix.data = keysyms;
    matrix.num_groups = num_groups;
    matrix.num_levels = num_levels;
    key = g_object_new (EEK_TYPE_CLUTTER_KEY,
                        "name", name,
                        "keycode", keycode,
                        "keysyms", &matrix,
                        "column", column,
                        "row", row,
                        "outline", outline,
                        "bounds", bounds,
                        NULL);
    g_return_val_if_fail (key, NULL);

    texture = g_hash_table_lookup (priv->key_outline_texture_hash, outline);
    if (texture == NULL) {
        texture = eek_clutter_key_create_texture (EEK_CLUTTER_KEY(key));
        g_hash_table_insert (priv->key_outline_texture_hash, outline, texture);
    } else
        texture = clutter_clone_new (texture);

    eek_clutter_key_set_texture (EEK_CLUTTER_KEY(key), texture);
    clutter_container_add_actor (CLUTTER_CONTAINER(self),
                                 CLUTTER_ACTOR(key));
    return key;
}

static void
eek_clutter_section_real_foreach_key (EekSection *self,
                                      GFunc       func,
                                      gpointer    user_data)
{
    EekClutterCallbackData data;

    g_return_if_fail (EEK_IS_CLUTTER_SECTION(self));

    data.func = func;
    data.user_data = user_data;

    clutter_container_foreach (CLUTTER_CONTAINER(self),
                               eek_clutter_callback,
                               &data);
}

static void
eek_section_iface_init (EekSectionIface *iface)
{
    iface->set_rows = eek_clutter_section_real_set_rows;
    iface->get_rows = eek_clutter_section_real_get_rows;
    iface->set_columns = eek_clutter_section_real_set_columns;
    iface->get_columns = eek_clutter_section_real_get_columns;
    iface->set_orientation = eek_clutter_section_real_set_orientation;
    iface->get_orientation = eek_clutter_section_real_get_orientation;
    iface->set_angle = eek_clutter_section_real_set_angle;
    iface->get_angle = eek_clutter_section_real_get_angle;
    iface->set_bounds = eek_clutter_section_real_set_bounds;
    iface->get_bounds = eek_clutter_section_real_get_bounds;
    iface->create_key = eek_clutter_section_real_create_key;
    iface->foreach_key = eek_clutter_section_real_foreach_key;
}

static void
eek_clutter_section_dispose (GObject *object)
{
    clutter_group_remove_all (CLUTTER_GROUP(object));
    G_OBJECT_CLASS (eek_clutter_section_parent_class)->dispose (object);
}

static void
eek_clutter_section_finalize (GObject *object)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(object);

    g_object_unref (priv->simple);
    G_OBJECT_CLASS (eek_clutter_section_parent_class)->finalize (object);
}

static void
eek_clutter_section_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_ANGLE:
        eek_section_set_angle (EEK_SECTION(object),
                               g_value_get_int (value));
        break;
    case PROP_BOUNDS:
        eek_section_set_bounds (EEK_SECTION(object),
                                g_value_get_boxed (value));
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_clutter_section_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(object);
    EekBounds bounds;

    switch (prop_id) {
    case PROP_ANGLE:
        g_value_set_int (value, eek_section_get_angle (EEK_SECTION(object)));
        break;
    case PROP_BOUNDS:
        eek_section_get_bounds (EEK_SECTION(object), &bounds);
        g_value_set_boxed (value, &bounds);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_clutter_section_paint (ClutterActor *self)
{
    ClutterGeometry geom;

    CLUTTER_ACTOR_CLASS (eek_clutter_section_parent_class)->paint (self);

    //clutter_actor_get_allocation_geometry (self, &geom);
    //cogl_set_source_color4ub (0x80, 0x00, 0x00, 0xff);
    //cogl_rectangle (0, 0, geom.width, geom.height);
}

static void
eek_clutter_section_class_init (EekClutterSectionClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class, sizeof (EekClutterSectionPrivate));

    gobject_class->set_property = eek_clutter_section_set_property;
    gobject_class->get_property = eek_clutter_section_get_property;
    gobject_class->finalize     = eek_clutter_section_finalize;
    gobject_class->dispose      = eek_clutter_section_dispose;

    actor_class->paint = eek_clutter_section_paint;

    g_object_class_override_property (gobject_class,
                                      PROP_ANGLE,
                                      "angle");
    g_object_class_override_property (gobject_class,
                                      PROP_BOUNDS,
                                      "bounds");
}

static void
eek_clutter_section_init (EekClutterSection *self)
{
    EekClutterSectionPrivate *priv;

    priv = self->priv = EEK_CLUTTER_SECTION_GET_PRIVATE (self);
    priv->simple = g_object_new (EEK_TYPE_SIMPLE_SECTION, NULL);
    priv->key_outline_texture_hash =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               g_free);
}
