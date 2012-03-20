/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <string.h>
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#endif

#include "eek-key.h"
#include "eek-section.h"
#include "eek-keyboard.h"
#include "eek-symbol.h"

enum {
    PROP_0,
    PROP_KEYCODE,
    PROP_SYMBOL_MATRIX,
    PROP_COLUMN,
    PROP_ROW,
    PROP_OREF,
    PROP_LAST
};

enum {
    PRESSED,
    RELEASED,
    LOCKED,
    UNLOCKED,
    CANCELLED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (EekKey, eek_key, EEK_TYPE_ELEMENT);

#define EEK_KEY_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_KEY, EekKeyPrivate))


struct _EekKeyPrivate
{
    guint keycode;
    EekSymbolMatrix *symbol_matrix;
    gint column;
    gint row;
    gulong oref;
    gboolean is_pressed;
    gboolean is_locked;
};

static void
eek_key_real_pressed (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    priv->is_pressed = TRUE;
#if DEBUG
    g_debug ("pressed %X", eek_key_get_keycode (self));
#endif
}

static void
eek_key_real_released (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    priv->is_pressed = FALSE;
#if DEBUG
    g_debug ("released %X", eek_key_get_keycode (self));
#endif
}

static void
eek_key_real_locked (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    priv->is_locked = TRUE;
#if DEBUG
    g_debug ("locked %X", eek_key_get_keycode (self));
#endif
}

static void
eek_key_real_unlocked (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    priv->is_locked = FALSE;
#if DEBUG
    g_debug ("unlocked %X", eek_key_get_keycode (self));
#endif
}

static void
eek_key_real_cancelled (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    priv->is_pressed = FALSE;
#if DEBUG
    g_debug ("cancelled %X", eek_key_get_keycode (self));
#endif
}

static void
eek_key_finalize (GObject *object)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(object);
    eek_symbol_matrix_free (priv->symbol_matrix);
    G_OBJECT_CLASS (eek_key_parent_class)->finalize (object);
}

static void
eek_key_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
    EekSymbolMatrix *matrix;
    gint column, row;

    switch (prop_id) {
    case PROP_KEYCODE:
        eek_key_set_keycode (EEK_KEY(object), g_value_get_uint (value));
        break;
    case PROP_SYMBOL_MATRIX:
        matrix = g_value_get_boxed (value);
        eek_key_set_symbol_matrix (EEK_KEY(object), matrix);
        break;
    case PROP_COLUMN:
        eek_key_get_index (EEK_KEY(object), &column, &row);
        eek_key_set_index (EEK_KEY(object), g_value_get_int (value), row);
        break;
    case PROP_ROW:
        eek_key_get_index (EEK_KEY(object), &column, &row);
        eek_key_set_index (EEK_KEY(object), column, g_value_get_int (value));
        break;
    case PROP_OREF:
        eek_key_set_oref (EEK_KEY(object), g_value_get_uint (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
eek_key_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
    gint column, row;

    switch (prop_id) {
    case PROP_KEYCODE:
        g_value_set_uint (value, eek_key_get_keycode (EEK_KEY(object)));
        break;
    case PROP_SYMBOL_MATRIX:
        g_value_set_boxed (value,
                           eek_key_get_symbol_matrix (EEK_KEY(object)));
        break;
    case PROP_COLUMN:
        eek_key_get_index (EEK_KEY(object), &column, &row);
        g_value_set_int (value, column);
        break;
    case PROP_ROW:
        eek_key_get_index (EEK_KEY(object), &column, &row);
        g_value_set_int (value, row);
        break;
    case PROP_OREF:
        g_value_set_uint (value, eek_key_get_oref (EEK_KEY(object)));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
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

    gobject_class->set_property = eek_key_set_property;
    gobject_class->get_property = eek_key_get_property;
    gobject_class->finalize     = eek_key_finalize;

    /* signals */
    klass->pressed = eek_key_real_pressed;
    klass->released = eek_key_real_released;
    klass->locked = eek_key_real_locked;
    klass->unlocked = eek_key_real_unlocked;
    klass->cancelled = eek_key_real_cancelled;

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
     * EekKey:symbol-matrix:
     *
     * The symbol matrix of #EekKey.
     */
    pspec = g_param_spec_boxed ("symbol-matrix",
                                "Symbol matrix",
                                "Symbol matrix of the key",
                                EEK_TYPE_SYMBOL_MATRIX,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_SYMBOL_MATRIX, pspec);

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
     * EekKey:oref:
     *
     * The outline id of #EekKey.
     */
    pspec = g_param_spec_ulong ("oref",
                                "Oref",
                                "Outline id of the key",
                                0, G_MAXULONG, 0,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_OREF, pspec);

    /**
     * EekKey::pressed:
     * @key: an #EekKey
     *
     * The ::pressed signal is emitted each time @key is shifted to
     * the pressed state.  The class handler runs before signal
     * handlers to allow signal handlers to read the status of @key
     * with eek_key_is_pressed().
     */
    signals[PRESSED] =
        g_signal_new (I_("pressed"),
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
        g_signal_new (I_("released"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekKeyClass, released),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    /**
     * EekKey::locked:
     * @key: an #EekKey
     *
     * The ::locked signal is emitted each time @key is shifted to
     * the locked state.  The class handler runs before signal
     * handlers to allow signal handlers to read the status of @key
     * with eek_key_is_locked().
     */
    signals[LOCKED] =
        g_signal_new (I_("locked"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekKeyClass, locked),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    /**
     * EekKey::unlocked:
     * @key: an #EekKey
     *
     * The ::unlocked signal is emitted each time @key is shifted to
     * the unlocked state.
     */
   signals[UNLOCKED] =
        g_signal_new (I_("unlocked"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekKeyClass, unlocked),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    /**
     * EekKey::cancelled:
     * @key: an #EekKey
     *
     * The ::cancelled signal is emitted each time @key is shifted to
     * the cancelled state.
     */
   signals[CANCELLED] =
        g_signal_new (I_("cancelled"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekKeyClass, cancelled),
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
    priv->symbol_matrix = eek_symbol_matrix_new (0, 0);
}

/**
 * eek_key_set_keycode:
 * @key: an #EekKey
 * @keycode: keycode
 *
 * Set the keycode of @key to @keycode.  Since typically the keycode
 * value is used to find a key in a keyboard by calling
 * eek_keyboard_find_key_by_keycode, it is not necessarily the same as
 * the X keycode but it should be unique in the keyboard @key belongs
 * to.
 */
void
eek_key_set_keycode (EekKey *key,
                     guint   keycode)
{
    g_return_if_fail (EEK_IS_KEY (key));
    key->priv->keycode = keycode;
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
    return key->priv->keycode;
}

/**
 * eek_key_set_symbol_matrix:
 * @key: an #EekKey
 * @matrix: an #EekSymbolMatrix
 *
 * Set the symbol matrix of @key to @matrix.
 */
void
eek_key_set_symbol_matrix (EekKey          *key,
                           EekSymbolMatrix *matrix)
{
    g_return_if_fail (EEK_IS_KEY(key));

    eek_symbol_matrix_free (key->priv->symbol_matrix);
    key->priv->symbol_matrix = eek_symbol_matrix_copy (matrix);
}

/**
 * eek_key_get_symbol_matrix:
 * @key: an #EekKey
 *
 * Get the symbol matrix of @key.
 * Returns: (transfer none): #EekSymbolMatrix or %NULL
 */
EekSymbolMatrix *
eek_key_get_symbol_matrix (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY(key), NULL);
    return key->priv->symbol_matrix;
}

/**
 * eek_key_get_symbol:
 * @key: an #EekKey
 *
 * Get the current symbol of @key.
 * Return value: (transfer none): the current #EekSymbol or %NULL on failure
 */
EekSymbol *
eek_key_get_symbol (EekKey *key)
{
    return eek_key_get_symbol_with_fallback (key, 0, 0);
}

/**
 * eek_key_get_symbol_with_fallback:
 * @key: an #EekKey
 * @fallback_group: fallback group index
 * @fallback_level: fallback level index
 *
 * Get the current symbol of @key.
 * Return value: (transfer none): the current #EekSymbol or %NULL on failure
 */
EekSymbol *
eek_key_get_symbol_with_fallback (EekKey *key,
                                  gint    fallback_group,
                                  gint    fallback_level)
{
    gint group, level;

    g_return_val_if_fail (EEK_IS_KEY (key), NULL);
    g_return_val_if_fail (fallback_group >= 0, NULL);
    g_return_val_if_fail (fallback_level >= 0, NULL);

    eek_element_get_symbol_index (EEK_ELEMENT(key), &group, &level);

    if (group < 0 || level < 0) {
        EekElement *section;

        section = eek_element_get_parent (EEK_ELEMENT(key));
        g_return_val_if_fail (EEK_IS_SECTION (section), NULL);

        if (group < 0)
            group = eek_element_get_group (section);

        if (level < 0)
            level = eek_element_get_level (section);

        if (group < 0 || level < 0) {
            EekElement *keyboard;

            keyboard = eek_element_get_parent (section);
            g_return_val_if_fail (EEK_IS_KEYBOARD (keyboard), NULL);

            if (group < 0)
                group = eek_element_get_group (keyboard);
            if (level < 0)
                level = eek_element_get_level (keyboard);
        }
    }

    return eek_key_get_symbol_at_index (key,
                                        group,
                                        level,
                                        fallback_group,
                                        fallback_level);
}

/**
 * eek_key_get_symbol_at_index:
 * @key: an #EekKey
 * @group: group index of the symbol matrix
 * @level: level index of the symbol matrix
 * @fallback_group: fallback group index
 * @fallback_level: fallback level index
 *
 * Get the symbol at (@group, @level) in the symbol matrix of @key.
 * Return value: (transfer none): an #EekSymbol at (@group, @level), or %NULL
 */
EekSymbol *
eek_key_get_symbol_at_index (EekKey *key,
                             gint    group,
                             gint    level,
                             gint    fallback_group,
                             gint    fallback_level)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(key);
    gint num_symbols;

    g_return_val_if_fail (fallback_group >= 0, NULL);
    g_return_val_if_fail (fallback_level >= 0, NULL);

    if (group < 0)
        group = fallback_group;
    if (level < 0)
        level = fallback_level;

    if (!priv->symbol_matrix)
        return NULL;

    num_symbols = priv->symbol_matrix->num_groups *
        priv->symbol_matrix->num_levels;
    if (num_symbols == 0)
        return NULL;

    if (group >= priv->symbol_matrix->num_groups) {
        if (fallback_group < 0)
            return NULL;
        group = fallback_group;
    }

    if (level >= priv->symbol_matrix->num_levels) {
        if (fallback_level < 0)
            return NULL;
        level = fallback_level;
    }

    return priv->symbol_matrix->data[group * priv->symbol_matrix->num_levels +
                                     level];
}

/**
 * eek_key_set_index:
 * @key: an #EekKey
 * @column: column index of @key in #EekSection
 * @row: row index of @key in #EekSection
 *
 * Set the location of @key in #EekSection with @column and @row.
 */
void
eek_key_set_index (EekKey *key,
                   gint    column,
                   gint    row)
{
    g_return_if_fail (EEK_IS_KEY(key));
    g_return_if_fail (0 <= column);
    g_return_if_fail (0 <= row);

    if (key->priv->column != column) {
        key->priv->column = column;
        g_object_notify (G_OBJECT(key), "column");
    }
    if (key->priv->row != row) {
        key->priv->row = row;
        g_object_notify (G_OBJECT(key), "row");
    }
}

/**
 * eek_key_get_index:
 * @key: an #EekKey
 * @column: (allow-none): pointer where the column index of @key in #EekSection will be stored
 * @row: (allow-none): pointer where the row index of @key in #EekSection will be stored
 *
 * Get the location of @key in #EekSection.
 */
void
eek_key_get_index (EekKey *key,
                   gint   *column,
                   gint   *row)
{
    g_return_if_fail (EEK_IS_KEY(key));
    g_return_if_fail (column != NULL || row != NULL);

    if (column != NULL)
        *column = key->priv->column;
    if (row != NULL)
        *row = key->priv->row;
}

/**
 * eek_key_set_oref:
 * @key: an #EekKey
 * @oref: outline id of @key
 *
 * Set the outline id of @key to @oref.
 */
void
eek_key_set_oref (EekKey *key,
                  guint   oref)
{
    g_return_if_fail (EEK_IS_KEY(key));
    if (key->priv->oref != oref) {
        key->priv->oref = oref;
        g_object_notify (G_OBJECT(key), "oref");
    }
}

/**
 * eek_key_get_oref:
 * @key: an #EekKey
 *
 * Get the outline id of @key.
 * Returns: unsigned integer
 */
guint
eek_key_get_oref (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY (key), 0);
    return key->priv->oref;
}

/**
 * eek_key_is_pressed:
 * @key: an #EekKey
 *
 * Return %TRUE if key is marked as pressed.
 */
gboolean
eek_key_is_pressed (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY(key), FALSE);
    return key->priv->is_pressed;
}

/**
 * eek_key_is_locked:
 * @key: an #EekKey
 *
 * Return %TRUE if key is marked as locked.
 */
gboolean
eek_key_is_locked (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY(key), FALSE);
    return key->priv->is_locked;
}
