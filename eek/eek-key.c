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

#include <string.h>
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */
#include "eek-key.h"
#include "eek-section.h"
#include "eek-keyboard.h"
#include "eek-symbol.h"
#include "eek-serializable.h"

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
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

static void eek_serializable_iface_init (EekSerializableIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekKey, eek_key, EEK_TYPE_ELEMENT,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_SERIALIZABLE,
                                                eek_serializable_iface_init));

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
};

static EekSerializableIface *eek_key_parent_serializable_iface;

static GVariant *
_g_variant_new_symbol_matrix (EekSymbolMatrix *symbol_matrix)
{
    GVariantBuilder builder, array;
    gint i, num_symbols = symbol_matrix->num_groups * symbol_matrix->num_levels;

    g_variant_builder_init (&builder, G_VARIANT_TYPE ("(iiv)"));
    g_variant_builder_add (&builder, "i", symbol_matrix->num_groups);
    g_variant_builder_add (&builder, "i", symbol_matrix->num_levels);
    g_variant_builder_init (&array, G_VARIANT_TYPE ("av"));
    for (i = 0; i < num_symbols; i++) {
        GVariant *symbol = eek_serializable_serialize
            (EEK_SERIALIZABLE(symbol_matrix->data[i]));
        g_variant_builder_add (&array, "v", symbol);
    }
    g_variant_builder_add (&builder, "v", g_variant_builder_end (&array));
    return g_variant_builder_end (&builder);
}

static EekSymbolMatrix *
_g_variant_get_symbol_matrix (GVariant *variant)
{
    gint num_groups, num_levels, i;
    EekSymbolMatrix *symbol_matrix;
    GVariant *array, *child;
    GVariantIter iter;

    g_variant_get_child (variant, 0, "i", &num_groups);
    g_variant_get_child (variant, 1, "i", &num_levels);
    symbol_matrix = eek_symbol_matrix_new (num_groups, num_levels);

    g_variant_get_child (variant, 2, "v", &array);
    g_variant_iter_init (&iter, array);
    for (i = 0; i < num_groups * num_levels; i++) {
        EekSerializable *serializable;

        if (!g_variant_iter_next (&iter, "v", &child)) {
            eek_symbol_matrix_free (symbol_matrix);
            g_return_val_if_reached (NULL);
        }

        serializable = eek_serializable_deserialize (child);
        symbol_matrix->data[i] = EEK_SYMBOL(serializable);
    }
    return symbol_matrix;
}

static void
eek_key_real_serialize (EekSerializable *self,
                        GVariantBuilder *builder)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);

    eek_key_parent_serializable_iface->serialize (self, builder);

    g_variant_builder_add (builder, "u", priv->keycode);
    g_variant_builder_add (builder, "v",
                           _g_variant_new_symbol_matrix (priv->symbol_matrix));
    g_variant_builder_add (builder, "i", priv->column);
    g_variant_builder_add (builder, "i", priv->row);
    g_variant_builder_add (builder, "u", priv->oref);
}

static gsize
eek_key_real_deserialize (EekSerializable *self,
                          GVariant        *variant,
                          gsize            index)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    GVariant *symbol_matrix;

    index = eek_key_parent_serializable_iface->deserialize (self,
                                                            variant,
                                                            index);

    g_variant_get_child (variant, index++, "u", &priv->keycode);
    g_variant_get_child (variant, index++, "v", &symbol_matrix);
    priv->symbol_matrix = _g_variant_get_symbol_matrix (symbol_matrix);
    g_variant_get_child (variant, index++, "i", &priv->column);
    g_variant_get_child (variant, index++, "i", &priv->row);
    g_variant_get_child (variant, index++, "u", &priv->oref);

    return index;
}

static void
eek_serializable_iface_init (EekSerializableIface *iface)
{
    eek_key_parent_serializable_iface =
        g_type_interface_peek_parent (iface);

    iface->serialize = eek_key_real_serialize;
    iface->deserialize = eek_key_real_deserialize;
}

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
eek_key_real_set_symbol_matrix (EekKey          *self,
                                EekSymbolMatrix *matrix)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    eek_symbol_matrix_free (priv->symbol_matrix);
    priv->symbol_matrix = eek_symbol_matrix_copy (matrix);
}

static EekSymbolMatrix *
eek_key_real_get_symbol_matrix (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    return priv->symbol_matrix;
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
eek_key_real_set_oref (EekKey *self, gulong oref)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    priv->oref = oref;
}

static gulong
eek_key_real_get_oref (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    return priv->oref;
}

static gboolean
eek_key_real_is_pressed (EekKey *self)
{
    EekKeyPrivate *priv = EEK_KEY_GET_PRIVATE(self);
    return priv->is_pressed;
}

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
    gint column, row;

    g_return_if_fail (EEK_IS_KEY(object));
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
    klass->set_symbol_matrix = eek_key_real_set_symbol_matrix;
    klass->get_symbol_matrix = eek_key_real_get_symbol_matrix;
    klass->set_index = eek_key_real_set_index;
    klass->get_index = eek_key_real_get_index;
    klass->set_oref = eek_key_real_set_oref;
    klass->get_oref = eek_key_real_get_oref;
    klass->is_pressed = eek_key_real_is_pressed;

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
    priv->symbol_matrix = eek_symbol_matrix_new (0, 0);
    priv->column = priv->row = 0;
    priv->oref = 0;
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
    EEK_KEY_GET_CLASS(key)->set_symbol_matrix (key, matrix);
}

/**
 * eek_key_get_symbol_matrix:
 * @key: an #EekKey
 *
 * Get the symbol matrix of @key.
 * Returns: #EekSymbolMatrix or %NULL
 */
EekSymbolMatrix *
eek_key_get_symbol_matrix (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY(key), NULL);
    return EEK_KEY_GET_CLASS(key)->get_symbol_matrix (key);
}

static EekKeyboard *
get_keyboard (EekKey *key)
{
    EekElement *parent;

    parent = eek_element_get_parent (EEK_ELEMENT(key));
    g_return_val_if_fail (EEK_IS_SECTION(parent), NULL);

    parent = eek_element_get_parent (parent);
    g_return_val_if_fail (EEK_IS_KEYBOARD(parent), NULL);

    return EEK_KEYBOARD(parent);
}

/**
 * eek_key_get_symbol:
 * @key: an #EekKey
 *
 * Get the current symbol of @key.
 * Returns: an #EekSymbol or %NULL on failure
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
 * Returns: an #EekSymbol or %NULL on failure
 */
EekSymbol *
eek_key_get_symbol_with_fallback (EekKey *key,
                                  gint    fallback_group,
                                  gint    fallback_level)
{
    gint group, level;
    EekKeyboard *keyboard;

    g_return_val_if_fail (EEK_IS_KEY (key), NULL);

    keyboard = get_keyboard (key);
    g_return_val_if_fail (keyboard, NULL);

    eek_keyboard_get_symbol_index (keyboard, &group, &level);
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
 * Returns: an #EekSymbol or %NULL on failure
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

    g_return_val_if_fail (group >= 0, NULL);
    g_return_val_if_fail (level >= 0, NULL);

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
    EEK_KEY_GET_CLASS(key)->set_index (key, column, row);
}

/**
 * eek_key_get_index:
 * @key: an #EekKey
 * @column: pointer where the column index of @key in #EekSection will be stored
 * @row: pointer where the row index of @key in #EekSection will be stored
 *
 * Get the location of @key in #EekSection.
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
 * eek_key_set_oref:
 * @key: an #EekKey
 * @oref: outline id of @key
 *
 * Set the outline id of @key to @oref.
 */
void
eek_key_set_oref (EekKey *key,
                  gulong  oref)
{
    g_return_if_fail (EEK_IS_KEY(key));
    EEK_KEY_GET_CLASS(key)->set_oref (key, oref);
}

/**
 * eek_key_get_oref:
 * @key: an #EekKey
 *
 * Get the outline id of @key.
 * Returns: a non-zero unsigned integer on success, 0 if the id is not set
 */
gulong
eek_key_get_oref (EekKey *key)
{
    g_return_val_if_fail (EEK_IS_KEY (key), 0);
    return EEK_KEY_GET_CLASS(key)->get_oref (key);
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
    g_assert (EEK_IS_KEY(key));
    return EEK_KEY_GET_CLASS(key)->is_pressed (key);
}
