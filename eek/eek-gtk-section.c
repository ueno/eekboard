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
 * SECTION:eek-gtk-section
 * @short_description: #EekSection implemented as a #GtkWidget
 *
 * The #EekGtkSection class implements the #EekSectionIface
 * interface as a #GtkWidget.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-gtk-section.h"
#include "eek-gtk-private.h"
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

G_DEFINE_TYPE_WITH_CODE (EekGtkSection, eek_gtk_section,
                         GTK_TYPE_VBOX,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_SECTION,
                                                eek_section_iface_init));

#define EEK_GTK_SECTION_GET_PRIVATE(obj)                           \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_GTK_SECTION, EekGtkSectionPrivate))

struct _EekGtkSectionPrivate
{
    EekSimpleSection *simple;
    GtkWidget **rows;           /* GtkHBox * */
};

static void
eek_gtk_section_real_set_rows (EekSection *self,
                               gint        rows)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_rows (EEK_SECTION(priv->simple), rows);
    priv->rows = g_slice_alloc0 (sizeof(GtkHBox *) * rows);
}

static gint
eek_gtk_section_real_get_rows (EekSection *self)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, -1);
    return eek_section_get_rows (EEK_SECTION(priv->simple));
}

static void
eek_gtk_section_real_set_columns (EekSection *self,
                                  gint        row,
                                  gint        columns)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_columns (EEK_SECTION(priv->simple), row, columns);
}

static gint
eek_gtk_section_real_get_columns (EekSection *self,
                                  gint        row)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, -1);
    return eek_section_get_columns (EEK_SECTION(priv->simple), row);
}

static void
eek_gtk_section_real_set_orientation (EekSection    *self,
                                      gint           row,
                                      EekOrientation orientation)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_orientation (EEK_SECTION(priv->simple), row, orientation);
}

static EekOrientation
eek_gtk_section_real_get_orientation (EekSection *self,
                                      gint        row)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, EEK_ORIENTATION_INVALID);
    return eek_section_get_orientation (EEK_SECTION(priv->simple), row);
}

static void
eek_gtk_section_real_set_angle (EekSection *self,
                                gint        angle)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_angle (EEK_SECTION(priv->simple), angle);
}

static gint
eek_gtk_section_real_get_angle (EekSection *self)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_val_if_fail (priv, 0);
    eek_section_get_angle (EEK_SECTION(priv->simple));
}

static void
eek_gtk_section_real_set_bounds (EekSection *self,
                                 EekBounds  *bounds)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_section_set_bounds (EEK_SECTION(priv->simple), bounds);
}

static void
eek_gtk_section_real_get_bounds (EekSection *self,
                                 EekBounds  *bounds)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);

    g_return_if_fail (priv);
    return eek_section_get_bounds (EEK_SECTION(priv->simple), bounds);
}

static EekKey *
eek_gtk_section_real_create_key (EekSection  *self,
                                 const gchar *name,
                                 guint       *keysyms,
                                 gint         num_groups,
                                 gint         num_levels,
                                 gint         column,
                                 gint         row,
                                 EekOutline  *outline,
                                 EekBounds   *bounds)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);
    EekKey *key;
    EekKeysymMatrix matrix;
    gint columns, rows;

    g_return_val_if_fail (priv, NULL);

    rows = eek_section_get_rows (self);
    g_return_val_if_fail (0 <= row && row < rows, NULL);
    columns = eek_section_get_columns (self, row);
    g_return_val_if_fail (column < columns, NULL);

    matrix.data = keysyms;
    matrix.num_groups = num_groups;
    matrix.num_levels = num_levels;
    key = g_object_new (EEK_TYPE_GTK_KEY,
                        "name", name,
                        "keysyms", &matrix,
                        "column", column,
                        "row", row,
                        "outline", outline,
                        "bounds", bounds,
                        NULL);
    g_return_val_if_fail (key, NULL);

    if (priv->rows[row] == NULL) {
        priv->rows[row] = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX(self), priv->rows[row],
                            FALSE, FALSE, 0);
        gtk_box_reorder_child (GTK_BOX(self), priv->rows[row], row);
    }
    gtk_box_pack_start (GTK_BOX(priv->rows[row]), GTK_WIDGET(key),
                        FALSE, FALSE, 0);
    gtk_box_reorder_child (GTK_BOX(priv->rows[row]), GTK_WIDGET(key), column);
    return key;
}

static void
eek_gtk_section_real_foreach_key (EekSection *self,
                                  GFunc       func,
                                  gpointer    user_data)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(self);
    EekGtkCallbackData data;
    gint i, num_rows;

    g_return_if_fail (priv);

    data.func = func;
    data.user_data = user_data;

    num_rows = eek_section_get_rows (self);
    for (i = 0; i < num_rows; i++)
        gtk_container_foreach (GTK_CONTAINER(priv->rows[i]),
                               eek_gtk_callback,
                               &data);
}

static void
eek_section_iface_init (EekSectionIface *iface)
{
    iface->set_rows = eek_gtk_section_real_set_rows;
    iface->get_rows = eek_gtk_section_real_get_rows;
    iface->set_columns = eek_gtk_section_real_set_columns;
    iface->get_columns = eek_gtk_section_real_get_columns;
    iface->set_orientation = eek_gtk_section_real_set_orientation;
    iface->get_orientation = eek_gtk_section_real_get_orientation;
    iface->set_angle = eek_gtk_section_real_set_angle;
    iface->get_angle = eek_gtk_section_real_get_angle;
    iface->set_bounds = eek_gtk_section_real_set_bounds;
    iface->get_bounds = eek_gtk_section_real_get_bounds;
    iface->create_key = eek_gtk_section_real_create_key;
    iface->foreach_key = eek_gtk_section_real_foreach_key;
}

static void
eek_gtk_section_dispose (GObject *object)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(object);
    gint i, num_rows;

    num_rows = eek_section_get_rows (EEK_SECTION(object));
    for (i = 0; i < num_rows; i++)
        if (priv->rows[i]) {
            g_object_unref (priv->rows[i]);
            priv->rows[i] = NULL;
        }
    if (priv->simple) {
        g_object_unref (priv->simple);
        priv->simple = NULL;
    }
    G_OBJECT_CLASS (eek_gtk_section_parent_class)->dispose (object);
}

static void
eek_gtk_section_finalize (GObject *object)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(object);

    g_slice_free (GtkWidget *, priv->rows);
    G_OBJECT_CLASS (eek_gtk_section_parent_class)->finalize (object);
}

static void
eek_gtk_section_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(object);

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
eek_gtk_section_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    EekGtkSectionPrivate *priv = EEK_GTK_SECTION_GET_PRIVATE(object);
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
eek_gtk_section_class_init (EekGtkSectionClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class, sizeof (EekGtkSectionPrivate));

    gobject_class->set_property = eek_gtk_section_set_property;
    gobject_class->get_property = eek_gtk_section_get_property;
    gobject_class->finalize     = eek_gtk_section_finalize;
    gobject_class->dispose      = eek_gtk_section_dispose;

    g_object_class_override_property (gobject_class,
                                      PROP_ANGLE,
                                      "angle");
    g_object_class_override_property (gobject_class,
                                      PROP_BOUNDS,
                                      "bounds");
}

static void
eek_gtk_section_init (EekGtkSection *self)
{
    EekGtkSectionPrivate *priv;

    priv = self->priv = EEK_GTK_SECTION_GET_PRIVATE (self);
    priv->simple = g_object_new (EEK_TYPE_SIMPLE_SECTION, NULL);
    priv->rows = NULL;
}
