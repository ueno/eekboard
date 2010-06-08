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
 * SECTION:eek-gtk-key
 * @short_description: #EekKey implemented as a #GtkWidget
 *
 * The #EekClutterKey class implements the #EekKeyIface interface as a
 * #GtkWidget.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-gtk-key.h"
#include "eek-simple-key.h"
#include "eek-keysym.h"

enum {
    PROP_0,
    PROP_KEYCODE,
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

G_DEFINE_TYPE_WITH_CODE (EekGtkKey, eek_gtk_key,
                         GTK_TYPE_BUTTON,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_KEY,
                                                eek_key_iface_init));

#define EEK_GTK_KEY_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_GTK_KEY, EekGtkKeyPrivate))

struct _EekGtkKeyPrivate
{
    EekSimpleKey *simple;
};

static guint
eek_gtk_key_real_get_keycode (EekKey *self)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, EEK_INVALID_KEYCODE);
    return eek_key_get_keycode (EEK_KEY(priv->simple));
}

static void
eek_gtk_key_real_set_keysyms (EekKey *self,
                              guint  *keysyms,
                              gint    groups,
                              gint    levels)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_keysyms (EEK_KEY(priv->simple), keysyms, groups, levels);
    if (groups > 0 && levels > 0)
        eek_key_set_keysym_index (EEK_KEY(self), 0, 0);
}

static void
eek_gtk_key_real_get_keysyms (EekKey *self,
                              guint **keysyms,
                              gint   *groups,
                              gint   *levels)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_get_keysyms (EEK_KEY(priv->simple), keysyms, groups, levels);
}

static gint
eek_gtk_key_real_get_groups (EekKey *self)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, -1);
    return eek_key_get_groups (EEK_KEY(priv->simple));
}

static guint
eek_gtk_key_real_get_keysym (EekKey *self)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, EEK_INVALID_KEYSYM);
    return eek_key_get_keysym (EEK_KEY(priv->simple));
}

static void
eek_gtk_key_real_set_index (EekKey *self,
                            gint    column,
                            gint    row)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_index (EEK_KEY(priv->simple), column, row);
}

static void
eek_gtk_key_real_get_index (EekKey *self,
                                gint   *column,
                                gint   *row)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_get_index (EEK_KEY(priv->simple), column, row);
}

static void
eek_gtk_key_real_set_outline (EekKey *self, EekOutline *outline)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_outline (EEK_KEY(priv->simple), outline);
}

static EekOutline *
eek_gtk_key_real_get_outline (EekKey *self)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_val_if_fail (priv, NULL);
    return eek_key_get_outline (EEK_KEY(priv->simple));
}

static void
eek_gtk_key_real_set_bounds (EekKey *self, EekBounds *bounds)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    eek_key_set_bounds (EEK_KEY(priv->simple), bounds);
}

static void
eek_gtk_key_real_get_bounds (EekKey    *self,
                                 EekBounds *bounds)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    return eek_key_get_bounds (EEK_KEY(priv->simple), bounds);
}

static void
eek_gtk_key_real_set_keysym_index (EekKey *self,
                                      gint    group,
                                      gint    level)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);
    guint keysym;

    g_return_if_fail (priv);
    eek_key_set_keysym_index (EEK_KEY(priv->simple), group, level);
    keysym = eek_key_get_keysym (self);
    if (keysym != EEK_INVALID_KEYSYM) {
        const gchar *label;

        label = eek_keysym_to_string (keysym);
        gtk_button_set_label (GTK_BUTTON(self), label ? label : "");
    }
}

static void
eek_gtk_key_real_get_keysym_index (EekKey *self, gint *group, gint *level)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(self);

    g_return_if_fail (priv);
    return eek_key_get_keysym_index (EEK_KEY(priv->simple), group, level);
}

static void
eek_key_iface_init (EekKeyIface *iface)
{
    iface->get_keycode = eek_gtk_key_real_get_keycode;
    iface->set_keysyms = eek_gtk_key_real_set_keysyms;
    iface->get_keysyms = eek_gtk_key_real_get_keysyms;
    iface->get_groups = eek_gtk_key_real_get_groups;
    iface->get_keysym = eek_gtk_key_real_get_keysym;
    iface->set_index = eek_gtk_key_real_set_index;
    iface->get_index = eek_gtk_key_real_get_index;
    iface->set_outline = eek_gtk_key_real_set_outline;
    iface->get_outline = eek_gtk_key_real_get_outline;
    iface->set_bounds = eek_gtk_key_real_set_bounds;
    iface->get_bounds = eek_gtk_key_real_get_bounds;
    iface->set_keysym_index = eek_gtk_key_real_set_keysym_index;
    iface->get_keysym_index = eek_gtk_key_real_get_keysym_index;
}

static void
eek_gtk_key_dispose (GObject *object)
{
    G_OBJECT_CLASS (eek_gtk_key_parent_class)->dispose (object);
}

static void
eek_gtk_key_finalize (GObject *object)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(object);

    g_object_unref (priv->simple);
    G_OBJECT_CLASS (eek_gtk_key_parent_class)->finalize (object);
}

static void
eek_gtk_key_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(object);
    EekKeysymMatrix *matrix;

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_KEYSYMS:
        matrix = g_value_get_boxed (value);
        eek_key_set_keysyms (EEK_KEY(object),
                             matrix->data,
                             matrix->num_groups,
                             matrix->num_levels);
        break;
    case PROP_KEYCODE:
    case PROP_BOUNDS:
    case PROP_OUTLINE:
    case PROP_COLUMN:
    case PROP_GROUP:
    case PROP_ROW:
    case PROP_LEVEL:
        g_object_set_property (G_OBJECT(priv->simple),
                               g_param_spec_get_name (pspec),
                               value);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_gtk_key_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    EekGtkKeyPrivate *priv = EEK_GTK_KEY_GET_PRIVATE(object);

    g_return_if_fail (priv);
    switch (prop_id) {
    case PROP_KEYCODE:
    case PROP_BOUNDS:
    case PROP_KEYSYMS:
    case PROP_COLUMN:
    case PROP_ROW:
    case PROP_OUTLINE:
    case PROP_GROUP:
    case PROP_LEVEL:
        g_object_get_property (G_OBJECT(priv->simple),
                               g_param_spec_get_name (pspec),
                               value);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_gtk_key_class_init (EekGtkKeyClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekGtkKeyPrivate));

    gobject_class->set_property = eek_gtk_key_set_property;
    gobject_class->get_property = eek_gtk_key_get_property;
    gobject_class->finalize     = eek_gtk_key_finalize;
    gobject_class->dispose      = eek_gtk_key_dispose;

    g_object_class_override_property (gobject_class,
                                      PROP_KEYCODE,
                                      "keycode");
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
eek_gtk_key_init (EekGtkKey *self)
{
    EekGtkKeyPrivate *priv;

    priv = self->priv = EEK_GTK_KEY_GET_PRIVATE(self);
    priv->simple = g_object_new (EEK_TYPE_SIMPLE_KEY, NULL);
}
