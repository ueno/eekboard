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
#include "eek-simple-key.h"
#include "eek-keysym.h"
#include <string.h>

enum {
    PROP_0,
    PROP_NAME,
    PROP_KEYSYMS,
    PROP_COLUMN,
    PROP_ROW,
    PROP_OUTLINE,
    PROP_BOUNDS,
    PROP_GROUP,
    PROP_LEVEL,
    PROP_LAST
};

static void eek_key_iface_init (EekKeyIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekSimpleKey, eek_simple_key,
                         G_TYPE_INITIALLY_UNOWNED,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_KEY,
                                                eek_key_iface_init));

#define EEK_SIMPLE_KEY_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_SIMPLE_KEY, EekSimpleKeyPrivate))

struct _EekSimpleKeyPrivate
{
    gchar *name;
    guint *keysyms;
    gint num_levels;
    gint num_groups;
    gint column;
    gint row;
    EekOutline *outline;
    EekBounds bounds;
    gint group;
    gint level;
};

static void
eek_simple_key_real_set_keysyms (EekKey *self,
                                 guint  *keysyms,
                                 gint    groups,
                                 gint    levels)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    if (priv->keysyms)
        g_slice_free (guint, priv->keysyms);
    priv->keysyms = g_slice_alloc (groups * levels * sizeof(guint));
    memcpy (priv->keysyms, keysyms, groups * levels * sizeof(guint));
    priv->num_groups = groups;
    priv->num_levels = levels;
}

static gint
eek_simple_key_real_get_groups (EekKey *self)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, -1);
    return priv->num_groups;
}

static guint
eek_simple_key_real_get_keysym (EekKey *self)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, EEK_INVALID_KEYSYM);
    if (priv->num_groups * priv->num_levels == 0)
        return EEK_INVALID_KEYSYM;
    return priv->keysyms[priv->group * priv->num_levels + priv->level];
}

static void
eek_simple_key_real_set_index (EekKey *self,
                               gint    column,
                               gint    row)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_return_if_fail (column < 0);
    g_return_if_fail (row < 0);
    priv->column = column;
    priv->row = row;
}

static void
eek_simple_key_real_get_index (EekKey *self,
                               gint   *column,
                               gint   *row)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_return_if_fail (column);
    g_return_if_fail (row);
    *column = priv->column;
    *row = priv->row;
}

static void
eek_simple_key_real_set_outline (EekKey *self, EekOutline *outline)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    priv->outline = outline;
}

static EekOutline *
eek_simple_key_real_get_outline (EekKey *self)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, NULL);
    return priv->outline;
}

static void
eek_simple_key_real_set_bounds (EekKey    *self,
                                EekBounds *bounds)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    priv->bounds = *bounds;
}

static void
eek_simple_key_real_get_bounds (EekKey    *self,
                                EekBounds *bounds)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_return_if_fail (bounds);
    *bounds = priv->bounds;
}

static void
eek_simple_key_real_set_keysym_index (EekKey *self,
                                      gint    group,
                                      gint    level)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_return_if_fail (group < priv->num_groups);
    g_return_if_fail (level < priv->num_levels);
    priv->group = group;
    priv->level = level;
}

static void
eek_simple_key_real_get_keysym_index (EekKey *self,
                                      gint   *group,
                                      gint   *level)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    g_return_if_fail (group);
    g_return_if_fail (level);
    *group = priv->group;
    *level = priv->level;
}

static void
eek_key_iface_init (EekKeyIface *iface)
{
    iface->set_keysyms = eek_simple_key_real_set_keysyms;
    iface->get_groups = eek_simple_key_real_get_groups;
    iface->get_keysym = eek_simple_key_real_get_keysym;
    iface->set_index = eek_simple_key_real_set_index;
    iface->get_index = eek_simple_key_real_get_index;
    iface->set_outline = eek_simple_key_real_set_outline;
    iface->get_outline = eek_simple_key_real_get_outline;
    iface->set_bounds = eek_simple_key_real_set_bounds;
    iface->get_bounds = eek_simple_key_real_get_bounds;
    iface->set_keysym_index = eek_simple_key_real_set_keysym_index;
    iface->get_keysym_index = eek_simple_key_real_get_keysym_index;
}

static void
eek_simple_key_dispose (GObject *object)
{
    G_OBJECT_CLASS (eek_simple_key_parent_class)->dispose (object);
}

static void
eek_simple_key_finalize (GObject *object)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(object);

    g_free (priv->name);
    g_slice_free (guint, priv->keysyms);
    G_OBJECT_CLASS (eek_simple_key_parent_class)->finalize (object);
}

static void
eek_simple_key_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(object);
    EekKeysymMatrix *matrix;

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_NAME:
        g_free (priv->name);
        priv->name = g_strdup (g_value_get_string (value));
        break;
    case PROP_KEYSYMS:
        matrix = g_value_get_boxed (value);
        eek_key_set_keysyms (EEK_KEY(object),
                             matrix->data,
                             matrix->num_groups,
                             matrix->num_levels);
        break;
    case PROP_COLUMN:
        priv->column = g_value_get_int (value);
        break;
    case PROP_ROW:
        priv->row = g_value_get_int (value);
        break;
    case PROP_OUTLINE:
        priv->outline = g_value_get_pointer (value);
        break;
    case PROP_BOUNDS:
        priv->bounds = *(EekBounds *)g_value_get_boxed (value);
        break;
    case PROP_GROUP:
        priv->group = g_value_get_int (value);
        break;
    case PROP_LEVEL:
        priv->level = g_value_get_int (value);
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
eek_simple_key_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    EekSimpleKeyPrivate *priv = EEK_SIMPLE_KEY_GET_PRIVATE(object);
    EekKeysymMatrix matrix;

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;
    case PROP_KEYSYMS:
        matrix.data = priv->keysyms;
        matrix.num_groups = priv->num_groups;
        matrix.num_levels = priv->num_levels;
        g_value_set_boxed (value, &matrix);
        break;
    case PROP_COLUMN:
        g_value_set_int (value, priv->column);
        break;
    case PROP_ROW:
        g_value_set_int (value, priv->row);
        break;
    case PROP_OUTLINE:
        g_value_set_pointer (value, priv->outline);
        break;
    case PROP_BOUNDS:
        g_value_set_boxed (value, &priv->bounds);
        break;
    case PROP_GROUP:
        g_value_set_int (value, priv->group);
        break;
    case PROP_LEVEL:
        g_value_set_int (value, priv->level);
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
eek_simple_key_class_init (EekSimpleKeyClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekSimpleKeyPrivate));

    gobject_class->set_property = eek_simple_key_set_property;
    gobject_class->get_property = eek_simple_key_get_property;
    gobject_class->finalize     = eek_simple_key_finalize;
    gobject_class->dispose      = eek_simple_key_dispose;

    g_object_class_override_property (gobject_class,
                                      PROP_NAME,
                                      "name");
    g_object_class_override_property (gobject_class,
                                      PROP_KEYSYMS,
                                      "keysyms");
    g_object_class_override_property (gobject_class,
                                      PROP_COLUMN,
                                      "column");
    g_object_class_override_property (gobject_class,
                                      PROP_ROW,
                                      "row");
    g_object_class_override_property (gobject_class,
                                      PROP_OUTLINE,
                                      "outline");
    g_object_class_override_property (gobject_class,
                                      PROP_BOUNDS,
                                      "bounds");
    g_object_class_override_property (gobject_class,
                                      PROP_GROUP,
                                      "group");
    g_object_class_override_property (gobject_class,
                                      PROP_LEVEL,
                                      "level");
}

static void
eek_simple_key_init (EekSimpleKey *self)
{
    EekSimpleKeyPrivate *priv;

    priv = self->priv = EEK_SIMPLE_KEY_GET_PRIVATE(self);
    priv->keysyms = NULL;
    priv->num_groups = 0;
    priv->num_levels = 0;
    priv->column = 0;
    priv->row = 0;
    priv->outline = NULL;
    memset (&priv->bounds, 0, sizeof priv->bounds);
}
