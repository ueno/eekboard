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

/**
 * SECTION:eek-symbol
 * @short_description: Base class of a symbol
 *
 * The #EekSymbolClass class represents a symbol assigned to a key.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-symbol.h"
#include "eek-serializable.h"

enum {
    PROP_0,
    PROP_NAME,
    PROP_LABEL,
    PROP_CATEGORY,
    PROP_MODIFIER_MASK,
    PROP_ICON_NAME,
    PROP_LAST
};

struct _EekSymbolPrivate {
    gchar *name;
    gchar *label;
    EekSymbolCategory category;
    EekModifierType modifier_mask;
    gchar *icon_name;
};

static void eek_serializable_iface_init (EekSerializableIface *iface);

G_DEFINE_TYPE_WITH_CODE (EekSymbol, eek_symbol, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (EEK_TYPE_SERIALIZABLE,
                                                eek_serializable_iface_init));

#define EEK_SYMBOL_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_SYMBOL, EekSymbolPrivate))

static void
eek_symbol_real_serialize (EekSerializable *self,
                           GVariantBuilder *builder)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
#define NOTNULL(s) ((s) != NULL ? (s) : "")
    g_variant_builder_add (builder, "s", NOTNULL(priv->name));
    g_variant_builder_add (builder, "s", NOTNULL(priv->label));
    g_variant_builder_add (builder, "u", priv->category);
    g_variant_builder_add (builder, "u", priv->modifier_mask);
    g_variant_builder_add (builder, "s", NOTNULL(priv->icon_name));
#undef NOTNULL
}

static gsize
eek_symbol_real_deserialize (EekSerializable *self,
                             GVariant        *variant,
                             gsize            index)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);

    g_variant_get_child (variant, index++, "s", &priv->name);
    g_variant_get_child (variant, index++, "s", &priv->label);
    g_variant_get_child (variant, index++, "u", &priv->category);
    g_variant_get_child (variant, index++, "u", &priv->modifier_mask);
    g_variant_get_child (variant, index++, "s", &priv->icon_name);

    return index;
}

static void
eek_serializable_iface_init (EekSerializableIface *iface)
{
    iface->serialize = eek_symbol_real_serialize;
    iface->deserialize = eek_symbol_real_deserialize;
}

static void
eek_symbol_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    switch (prop_id) {
    case PROP_NAME:
        eek_symbol_set_name (EEK_SYMBOL(object), g_value_get_string (value));
        break;
    case PROP_LABEL:
        eek_symbol_set_label (EEK_SYMBOL(object), g_value_get_string (value));
        break;
    case PROP_CATEGORY:
        eek_symbol_set_category (EEK_SYMBOL(object), g_value_get_uint (value));
        break;
    case PROP_MODIFIER_MASK:
        eek_symbol_set_modifier_mask (EEK_SYMBOL(object),
                                      g_value_get_uint (value));
        break;
    case PROP_ICON_NAME:
        eek_symbol_set_icon_name (EEK_SYMBOL(object),
                                  g_value_get_string (value));
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_symbol_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, eek_symbol_get_name (EEK_SYMBOL(object)));
        break;
    case PROP_LABEL:
        g_value_set_string (value, eek_symbol_get_label (EEK_SYMBOL(object)));
        break;
    case PROP_CATEGORY:
        g_value_set_uint (value, eek_symbol_get_category (EEK_SYMBOL(object)));
        break;
    case PROP_MODIFIER_MASK:
        g_value_set_uint (value,
                          eek_symbol_get_modifier_mask (EEK_SYMBOL(object)));
        break;
    case PROP_ICON_NAME:
        g_value_set_string (value,
                            eek_symbol_get_icon_name (EEK_SYMBOL(object)));
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_symbol_finalize (GObject *object)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(object);

    g_free (priv->name);
    g_free (priv->label);
    g_free (priv->icon_name);
    G_OBJECT_CLASS (eek_symbol_parent_class)->finalize (object);
}

static void
eek_symbol_class_init (EekSymbolClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_type_class_add_private (gobject_class, sizeof (EekSymbolPrivate));

    gobject_class->set_property = eek_symbol_set_property;
    gobject_class->get_property = eek_symbol_get_property;
    gobject_class->finalize = eek_symbol_finalize;

    pspec = g_param_spec_string ("name",
                                 "Name",
                                 "Canonical name of the symbol",
                                 NULL,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_NAME, pspec);

    pspec = g_param_spec_string ("label",
                                 "Label",
                                 "Text used to display the symbol",
                                 NULL,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_LABEL, pspec);

    pspec = g_param_spec_uint ("category",
                               "Category",
                               "Category of the symbol",
                               0, G_MAXUINT, 0,
                               G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_CATEGORY, pspec);

    pspec = g_param_spec_uint ("modifier-mask",
                               "Modifier mask",
                               "Modifier mask of the symbol",
                               0, G_MAXUINT, 0,
                               G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_MODIFIER_MASK, pspec);

    pspec = g_param_spec_string ("icon-name",
                                 "Icon name",
                                 "Icon name used to render the symbol",
                                 NULL,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_ICON_NAME, pspec);
}

static void
eek_symbol_init (EekSymbol *self)
{
    EekSymbolPrivate *priv;

    priv = self->priv = EEK_SYMBOL_GET_PRIVATE(self);
    priv->name = NULL;
    priv->label = NULL;
    priv->icon_name = NULL;
    priv->category = EEK_SYMBOL_CATEGORY_UNKNOWN;
    priv->modifier_mask = 0;
}

EekSymbol *
eek_symbol_new (const gchar *name)
{
    return g_object_new (EEK_TYPE_SYMBOL, "name", name);
}

void
eek_symbol_set_name (EekSymbol   *symbol,
                     const gchar *name)
{
    EekSymbolPrivate *priv;

    g_return_if_fail (EEK_IS_SYMBOL(symbol));

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    g_free (priv->name);
    priv->name = g_strdup (name);
}

G_CONST_RETURN gchar *
eek_symbol_get_name (EekSymbol *symbol)
{
    EekSymbolPrivate *priv;

    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), NULL);

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    if (priv->name == NULL || *priv->name == '\0')
        return NULL;
    return priv->name;
}

void
eek_symbol_set_label (EekSymbol   *symbol,
                      const gchar *label)
{
    EekSymbolPrivate *priv;

    g_return_if_fail (EEK_IS_SYMBOL(symbol));

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    g_free (priv->label);
    priv->label = g_strdup (label);
}

gchar *
eek_symbol_get_label (EekSymbol *symbol)
{
    EekSymbolPrivate *priv;

    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), NULL);

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    if (priv->label == NULL || *priv->label == '\0')
        return NULL;
    return g_strdup (priv->label);
}

/**
 * eek_symbol_set_category:
 * @symbol: an #EekSymbol
 * @category: an #EekSymbolCategory
 *
 * Set symbol category of @symbol to @category.
 */
void
eek_symbol_set_category (EekSymbol        *symbol,
                         EekSymbolCategory category)
{
    EekSymbolPrivate *priv;

    g_return_if_fail (EEK_IS_SYMBOL(symbol));

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    priv->category = category;
}

/**
 * eek_symbol_get_category:
 * @symbol: an #EekSymbol
 *
 * Returns symbol category of @symbol.
 */
EekSymbolCategory
eek_symbol_get_category (EekSymbol *symbol)
{
    EekSymbolPrivate *priv;

    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), EEK_SYMBOL_CATEGORY_UNKNOWN);

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    return priv->category;
}

/**
 * eek_symbol_set_modifier_mask:
 * @symbol: an #EekSymbol
 *
 * Set modifier mask @symbol can trigger.
 */
void
eek_symbol_set_modifier_mask (EekSymbol      *symbol,
                              EekModifierType mask)
{
    EekSymbolPrivate *priv;

    g_return_if_fail (EEK_IS_SYMBOL(symbol));

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    priv->modifier_mask = mask;
}

/**
 * eek_symbol_get_modifier_mask:
 * @symbol: an #EekSymbol
 *
 * Returns modifier mask @symbol can trigger.
 */
EekModifierType
eek_symbol_get_modifier_mask (EekSymbol *symbol)
{
    EekSymbolPrivate *priv;

    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), 0);

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    return priv->modifier_mask;
}

/**
 * eek_symbol_is_modifier:
 * @symbol: an #EekSymbol
 *
 * Check if @symbol is a modifier.
 * Returns: %TRUE if @symbol is a modifier.
 */
gboolean
eek_symbol_is_modifier (EekSymbol *symbol)
{
    return eek_symbol_get_modifier_mask (symbol) != 0;
}

void
eek_symbol_set_icon_name (EekSymbol   *symbol,
                     const gchar *icon_name)
{
    EekSymbolPrivate *priv;

    g_return_if_fail (EEK_IS_SYMBOL(symbol));

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    g_free (priv->icon_name);
    priv->icon_name = g_strdup (icon_name);
}

G_CONST_RETURN gchar *
eek_symbol_get_icon_name (EekSymbol *symbol)
{
    EekSymbolPrivate *priv;

    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), NULL);

    priv = EEK_SYMBOL_GET_PRIVATE(symbol);
    if (priv->icon_name == NULL || *priv->icon_name == '\0')
        return NULL;
    return priv->icon_name;
}
