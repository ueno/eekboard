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
 * SECTION:eek-key
 * @short_description: Base class of a key
 *
 * The #EekKeyClass class represents a key.
 */

#include <string.h>
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-key.h"
#include "eek-keysym.h"

enum {
    PROP_0,
    PROP_KEYCODE,
    PROP_KEYSYMS,
    PROP_COLUMN,
    PROP_ROW,
    PROP_OUTLINE,
    PROP_GROUP,
    PROP_LEVEL,
    PROP_LAST
};

enum {
    PRESSED,
    RELEASED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (EekKey, eek_key, EEK_TYPE_ELEMENT);

#define EEK_KEY_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_KEY, EekKeyPrivate))


struct _EekKeyPrivate
{
    guint keycode;
    EekKeysymMatrix keysyms;
    gint column;
    gint row;
    EekOutline *outline;
    gint group;
    gint level;
};

static void
eek_key_real_set_keycode (EekKey *self, guint keycode)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    priv->keycode = keycode;
}

static guint
eek_key_real_get_keycode (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    return priv->keycode;
}

static void
eek_key_real_set_keysyms (EekKey *self,
                          guint  *keysyms,
                          gint    num_groups,
                          gint    num_levels)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    gint num_keysyms = num_groups * num_levels;
    
    if (num_keysyms > 0) {
        priv->keysyms.data =
            g_slice_alloc (num_keysyms * sizeof(guint));
        memcpy (priv->keysyms.data, keysyms,
                num_keysyms * sizeof(guint));
    }
    priv->keysyms.num_groups = num_groups;
    priv->keysyms.num_levels = num_levels;

#if DEBUG
    {
        const gchar *name;
        gint i;

        name = eek_element_get_name (EEK_ELEMENT(self));
        fprintf (stderr, "%s: ", name);
        for (i = 0; i < priv->keysyms.num_groups * priv->keysyms.num_levels; i++)
            fprintf (stderr, "\"%s\" ", eek_keysym_to_string (priv->keysyms.data[i]));
        fprintf (stderr, "\n");
    }
#endif
}

static void
eek_key_real_get_keysyms (EekKey *self,
                          guint **keysyms,
                          gint   *num_groups,
                          gint   *num_levels)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    gint num_keysyms = priv->keysyms.num_groups * priv->keysyms.num_levels;

    if (num_groups)
        *num_groups = priv->keysyms.num_groups;
    if (num_levels)
        *num_levels = priv->keysyms.num_levels;
    if (keysyms && num_keysyms > 0) {
        *keysyms = g_slice_alloc (num_keysyms * sizeof(guint));
        memcpy (*keysyms, priv->keysyms.data, num_keysyms * sizeof(guint));
    }
}

static guint
eek_key_real_get_keysym (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    gint num_keysyms = priv->keysyms.num_groups * priv->keysyms.num_levels;

    if (num_keysyms == 0)
        return EEK_INVALID_KEYSYM;
    return priv->keysyms.data[priv->group * priv->keysyms.num_levels +
                              priv->level];
}

static void
eek_key_real_set_index (EekKey *self,
                        gint    column,
                        gint    row)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    g_return_if_fail (0 <= column);
    g_return_if_fail (0 <= row);
    priv->column = column;
    priv->row = row;
}

static void
eek_key_real_get_index (EekKey *self,
                        gint   *column,
                        gint   *row)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    if (column)
        *column = priv->column;
    if (row)
        *row = priv->row;
}

static void
eek_key_real_set_outline (EekKey *self, EekOutline *outline)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    priv->outline = outline;
}

static EekOutline *
eek_key_real_get_outline (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    return priv->outline;
}

static void
eek_key_real_set_keysym_index (EekKey *self,
                               gint    group,
                               gint    level)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    g_return_if_fail (0 <= group);
    if (group >= priv->keysyms.num_groups)
        group = 0;
    g_return_if_fail (0 <= level);
    if (level >= priv->keysyms.num_levels)
        level = 0;
    priv->group = group;
    priv->level = level;
}

static void
eek_key_real_get_keysym_index (EekKey *self,
                                      gint   *group,
                                      gint   *level)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    g_return_if_fail (group);
    g_return_if_fail (level);
    if (group)
        *group = priv->group;
    if (level)
        *level = priv->level;
}

static void
eek_key_real_pressed (EekKey *key)
{
#if DEBUG
    g_debug ("pressed %X", eek_key_get_keycode (key));
#endif
}

static void
eek_key_real_released (EekKey *key)
{
#if DEBUG
    g_debug ("released %X", eek_key_get_keycode (key));
#endif
}

static void
eek_key_finalize (GObject *object)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(object);
    gint num_keysyms = priv->keysyms.num_groups * priv->keysyms.num_levels;

    g_slice_free1 (num_keysyms * sizeof (guint), priv->keysyms.data);
    G_OBJECT_CLASS (eek_key_parent_class)->finalize (object);
}

static void
eek_key_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
    EekKeysymMatrix *matrix;
    gint column, row;
    gint group, level;

    g_return_if_fail (EEK_IS_KEY(object));
    switch (prop_id) {
    case PROP_KEYCODE:
        eek_key_set_keycode (EEK_KEY(object), g_value_get_uint (value));
        break;
    case PROP_KEYSYMS:
        matrix = g_value_get_boxed (value);
        eek_key_set_keysyms (EEK_KEY(object),
                             matrix->data,
                             matrix->num_groups,
                             matrix->num_levels);
        break;
    case PROP_COLUMN:
        eek_key_get_index (EEK_KEY(object), &column, &row);
        eek_key_set_index (EEK_KEY(object), g_value_get_int (value), row);
        break;
    case PROP_ROW:
        eek_key_get_index (EEK_KEY(object), &column, &row);
        eek_key_set_index (EEK_KEY(object), column, g_value_get_int (value));
        break;
    case PROP_OUTLINE:
        eek_key_set_outline (EEK_KEY(object), g_value_get_pointer (value));
        break;
    case PROP_GROUP:
        eek_key_get_keysym_index (EEK_KEY(object), &group, &level);
        eek_key_set_keysym_index (EEK_KEY(object), g_value_get_int (value),
                                  level);
        break;
    case PROP_LEVEL:
        eek_key_get_keysym_index (EEK_KEY(object), &group, &level);
        eek_key_set_keysym_index (EEK_KEY(object), group,
                                  g_value_get_int (value));
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_key_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
    EekKeysymMatrix matrix;
    gint column, row;
    gint group, level;

    g_return_if_fail (EEK_IS_KEY(object));
    switch (prop_id) {
    case PROP_KEYCODE:
        g_value_set_uint (value, eek_key_get_keycode (EEK_KEY(object)));
        break;
    case PROP_KEYSYMS:
        eek_key_get_keysyms (EEK_KEY(object), &matrix.data, &matrix.num_groups,
                             &matrix.num_levels);
        g_value_set_boxed (value, &matrix);
        break;
    case PROP_COLUMN:
        eek_key_get_index (EEK_KEY(object), &column, &row);
        g_value_set_int (value, column);
        break;
    case PROP_ROW:
        eek_key_get_index (EEK_KEY(object), &column, &row);
        g_value_set_int (value, row);
        break;
    case PROP_OUTLINE:
        g_value_set_pointer (value, eek_key_get_outline (EEK_KEY(object)));
        break;
    case PROP_GROUP:
        eek_key_get_keysym_index (EEK_KEY(object), &group, &level);
        g_value_set_int (value, group);
        break;
    case PROP_LEVEL:
        eek_key_get_keysym_index (EEK_KEY(object), &group, &level);
        g_value_set_int (value, level);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_key_class_init (EekKeyClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekKeyPrivate));

    klass->get_keycode = eek_key_real_get_keycode;
    klass->set_keycode = eek_key_real_set_keycode;
    klass->set_keysyms = eek_key_real_set_keysyms;
    klass->get_keysyms = eek_key_real_get_keysyms;
    klass->get_keysym = eek_key_real_get_keysym;
    klass->set_index = eek_key_real_set_index;
    klass->get_index = eek_key_real_get_index;
    klass->set_outline = eek_key_real_set_outline;
    klass->get_outline = eek_key_real_get_outline;
    klass->set_keysym_index = eek_key_real_set_keysym_index;
    klass->get_keysym_index = eek_key_real_get_keysym_index;

    gobject_class->set_property = eek_key_set_property;
    gobject_class->get_property = eek_key_get_property;
    gobject_class->finalize     = eek_key_finalize;

    /* signals */
    klass->pressed = eek_key_real_pressed;
    klass->released = eek_key_real_released;

    /**
     * EekKey:keycode:
     *
     * The keycode of #EekKey.
     */
    pspec = g_param_spec_uint ("keycode",
                               "Keycode",
                               "Keycode of the key",
                               0, G_MAXUINT, 0,
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_KEYCODE, pspec);

    /**
     * EekKey:keysyms:
     *
     * The symbol matrix of #EekKey.
     */
    pspec = g_param_spec_boxed ("keysyms",
                                "Keysyms",
                                "Symbol matrix of the key",
                                EEK_TYPE_KEYSYM_MATRIX,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_KEYSYMS, pspec);

    /**
     * EekKey:column:
     *
     * The column index of #EekKey in the parent #EekSection.
     */
    pspec = g_param_spec_int ("column",
                              "Column",
                              "Column index of the key in section",
                              -1, G_MAXINT, -1,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_COLUMN, pspec);

    /**
     * EekKey:row:
     *
     * The row index of #EekKey in the parent #EekSection.
     */
    pspec = g_param_spec_int ("row",
                              "Row",
                              "Row index of the key in section",
                              -1, G_MAXINT, -1,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_ROW, pspec);

    /**
     * EekKey:outline:
     *
     * The pointer to the outline shape of #EekKey.
     */
    /* Use pointer instead of boxed to avoid copy, since we can
       assume that only a few outline shapes are used in a whole
       keyboard (unlike keysyms and bounds). */
    pspec = g_param_spec_pointer ("outline",
                                  "Outline",
                                  "Pointer to outline shape of the key",
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_OUTLINE, pspec);

    /**
     * EekKey:group:
     *
     * The column index of #EekKey in the symbol matrix #EekKey:keysyms.
     */
    pspec = g_param_spec_int ("group",
                              "Group",
                              "Current group of the key",
                              0, 64, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GROUP, pspec);

    /**
     * EekKey:level:
     *
     * The row index of #EekKey in the symbol matrix #EekKey:keysyms.
     */
    pspec = g_param_spec_int ("level",
                              "Level",
                              "Current level of the key",
                              0, 3, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_LEVEL, pspec);

    /**
     * EekKey::pressed:
     * @key: an #EekKey
     *
     * The ::pressed signal is emitted each time @key is shifted to
     * the pressed state.
     */
    signals[PRESSED] =
        g_signal_new ("pressed",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekKeyClass, pressed),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    /**
     * EekKey::released:
     * @key: an #EekKey
     *
     * The ::released signal is emitted each time @key is shifted to
     * the released state.
     */
   signals[RELEASED] =
        g_signal_new ("released",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekKeyClass, released),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}

static void
eek_key_init (EekKey *self)
{
    EekKeyPrivate *priv;

    priv = self->priv = EEK_KEY_GET_PRIVATE(self);
    priv->keycode = 0;
    memset (&priv->keysyms, 0, sizeof priv->keysyms);
    priv->column = priv->row = 0;
    priv->outline = NULL;
    priv->group = priv->level = 0;
}

/**
 * eek_key_set_keycode:
 * @key: an #EekKey
 * @keycode: keycode
 *
 * Set keycode of @key to @keycode.
 */
void
eek_key_set_keycode (EekKey *key,
                     guint   keycode)
{
    g_return_if_fail (EEK_IS_KEY (key));
    EEK_KEY_GET_CLASS(key)->set_keycode (key, keycode);
}

/**
 * eek_key_get_keycode:
 * @key: an #EekKey
 *
 * Get keycode of @key.
 * Returns: keycode or %EEK_INVALID_KEYCODE on failure
 */
guint
eek_key_get_keycode (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY (key), EEK_INVALID_KEYCODE);
    return EEK_KEY_GET_CLASS(key)->get_keycode (key);
}

/**
 * eek_key_set_keysyms:
 * @key: an #EekKey
 * @keysyms: symbol matrix of @key
 * @num_groups: number of groups (rows) of @keysyms
 * @num_levels: number of levels (columns) of @keysyms
 *
 * Set the symbol matrix of @key to @keysyms.  The length of @keysyms
 * is @num_groups * @num_levels.
 */
void
eek_key_set_keysyms (EekKey *key,
                     guint  *keysyms,
                     gint    num_groups,
                     gint    num_levels)
{
    g_return_if_fail (EEK_IS_KEY(key));
    EEK_KEY_GET_CLASS(key)->set_keysyms (key, keysyms, num_groups, num_levels);
}

/**
 * eek_key_get_keysyms:
 * @key: an #EekKey
 * @keysyms: pointer where symbol matrix of @key will be stored
 * @num_groups: pointer where the number of groups (rows) of @keysyms
 * will be stored
 * @num_levels: pointer where the number of levels (columns) of
 * @keysyms will be stored
 *
 * Get the symbol matrix of @key.  If either @keysyms, @num_groups, or
 * @num_levels are NULL, this function does not try to get the value.
 */
void
eek_key_get_keysyms (EekKey *key,
                     guint **keysyms,
                     gint   *num_groups,
                     gint   *num_levels)
{
    g_return_if_fail (EEK_IS_KEY(key));
    EEK_KEY_GET_CLASS(key)->get_keysyms (key, keysyms, num_groups, num_levels);
}

/**
 * eek_key_get_keysym:
 * @key: an #EekKey
 *
 * Get the current symbol of @key.
 * Returns: a symbol or %EEK_INVALID_KEYSYM on failure
 */
guint
eek_key_get_keysym (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY(key), EEK_INVALID_KEYSYM);
    return EEK_KEY_GET_CLASS(key)->get_keysym (key);
}

/**
 * eek_key_set_index:
 * @key: an #EekKey
 * @column: column index of @key in #EekSection
 * @row: row index of @key in #EekSection
 *
 * Set the index of @key (i.e. logical location of @key in
 * #EekSection) to @column and @row.
 */
void
eek_key_set_index (EekKey *key,
                   gint    column,
                   gint    row)
{
    g_return_if_fail (EEK_IS_KEY(key));
    EEK_KEY_GET_CLASS(key)->set_index (key, column, row);
}

/**
 * eek_key_get_index:
 * @key: an #EekKey
 * @column: pointer where the column index of @key in #EekSection will be stored
 * @row: pointer where the row index of @key in #EekSection will be stored
 *
 * Get the index of @key (i.e. logical location of @key in
 * #EekSection).
 */
void
eek_key_get_index (EekKey *key,
                   gint   *column,
                   gint   *row)
{
    g_return_if_fail (EEK_IS_KEY(key));
    EEK_KEY_GET_CLASS(key)->get_index (key, column, row);
}

/**
 * eek_key_set_outline:
 * @key: an #EekKey
 * @outline: outline of @key
 *
 * Set the outline shape of @key to @outline.
 */
void
eek_key_set_outline (EekKey     *key,
                     EekOutline *outline)
{
    g_return_if_fail (EEK_IS_KEY(key));
    EEK_KEY_GET_CLASS(key)->set_outline (key, outline);
}

/**
 * eek_key_get_outline:
 * @key: an #EekKey
 *
 * Get the outline shape of @key.
 * Returns: an #EekOutline pointer or NULL on failure
 */
EekOutline *
eek_key_get_outline (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY (key), NULL);
    return EEK_KEY_GET_CLASS(key)->get_outline (key);
}

/**
 * eek_key_set_keysym_index:
 * @key: an #EekKey
 * @group: group (row) index of @key
 * @level: level (column) index of @key
 *
 * Set the current group and/or level index of @key in its symbol
 * matrix to @group and @level.
 */
void
eek_key_set_keysym_index (EekKey *key,
                          gint    group,
                          gint    level)
{
    g_return_if_fail (EEK_IS_KEY(key));
    EEK_KEY_GET_CLASS(key)->set_keysym_index (key, group, level);
}

/**
 * eek_key_get_keysym_index:
 * @key: an #EekKey
 * @group: pointer where group (row) index of @key will be stored
 * @level: pointer where level (column) index of @key will be stored
 *
 * Get the current group and/or level index of @key in its symbol
 * matrix.
 */
void
eek_key_get_keysym_index (EekKey *key,
                          gint   *group,
                          gint   *level)
{
    g_return_if_fail (EEK_IS_KEY(key));
    EEK_KEY_GET_CLASS(key)->get_keysym_index (key, group, level);
}
