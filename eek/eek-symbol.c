#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-symbol.h"

enum {
    PROP_0,
    PROP_NAME,
    PROP_LABEL,
    PROP_CATEGORY,
    PROP_MODIFIER_MASK,
    PROP_LAST
};

struct _EekSymbolPrivate {
    gchar *name;
    gchar *label;
    EekSymbolCategory category;
    EekModifierType modifier_mask;
};

G_DEFINE_TYPE (EekSymbol, eek_symbol, G_TYPE_OBJECT);

#define EEK_SYMBOL_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_SYMBOL, EekSymbolPrivate))

static void
eek_symbol_real_set_name (EekSymbol   *self,
                          const gchar *name)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
    g_free (priv->name);
    priv->name = g_strdup (name);
}

G_CONST_RETURN gchar *
eek_symbol_real_get_name (EekSymbol *self)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
    return priv->name;
}

static void
eek_symbol_real_set_label (EekSymbol   *self,
                           const gchar *label)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
    g_free (priv->label);
    priv->label = g_strdup (label);
}

gchar *
eek_symbol_real_get_label (EekSymbol *self)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
    return priv->label;
}

static void
eek_symbol_real_set_category (EekSymbol        *self,
                              EekSymbolCategory category)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
    priv->category = category;
}

EekSymbolCategory
eek_symbol_real_get_category (EekSymbol *self)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
    return priv->category;
}

static void
eek_symbol_real_set_modifier_mask (EekSymbol      *self,
                                   EekModifierType mask)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
    priv->modifier_mask = mask;
}

EekModifierType
eek_symbol_real_get_modifier_mask (EekSymbol *self)
{
    EekSymbolPrivate *priv = EEK_SYMBOL_GET_PRIVATE(self);
    return priv->modifier_mask;
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
        eek_symbol_set_category (EEK_SYMBOL(object), g_value_get_int (value));
        break;
    case PROP_MODIFIER_MASK:
        eek_symbol_set_modifier_mask (EEK_SYMBOL(object),
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
        g_value_set_int (value, eek_symbol_get_category (EEK_SYMBOL(object)));
        break;
    case PROP_MODIFIER_MASK:
        g_value_set_int (value,
                         eek_symbol_get_modifier_mask (EEK_SYMBOL(object)));
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
    if (priv->name)
        g_free (priv->name);
}

static void
eek_symbol_class_init (EekSymbolClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_type_class_add_private (gobject_class, sizeof (EekSymbolPrivate));

    klass->set_name = eek_symbol_real_set_name;
    klass->get_name = eek_symbol_real_get_name;
    klass->set_label = eek_symbol_real_set_label;
    klass->get_label = eek_symbol_real_get_label;
    klass->set_category = eek_symbol_real_set_category;
    klass->get_category = eek_symbol_real_get_category;
    klass->set_modifier_mask = eek_symbol_real_set_modifier_mask;
    klass->get_modifier_mask = eek_symbol_real_get_modifier_mask;

    gobject_class->set_property = eek_symbol_set_property;
    gobject_class->get_property = eek_symbol_get_property;
    gobject_class->finalize = eek_symbol_finalize;

    pspec = g_param_spec_string ("name",
                                 "Name",
                                 "Canonical name of the keysym",
                                 NULL,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_NAME, pspec);

    pspec = g_param_spec_string ("label",
                                 "Label",
                                 "Text used to display the keysym",
                                 NULL,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_LABEL, pspec);
}

static void
eek_symbol_init (EekSymbol *self)
{
    EekSymbolPrivate *priv;

    priv = self->priv = EEK_SYMBOL_GET_PRIVATE(self);
    priv->name = NULL;
    priv->label = NULL;
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
    g_return_if_fail (EEK_IS_SYMBOL(symbol));
    EEK_SYMBOL_GET_CLASS(symbol)->set_name (symbol, name);
}

G_CONST_RETURN gchar *
eek_symbol_get_name (EekSymbol *symbol)
{
    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), NULL);
    return EEK_SYMBOL_GET_CLASS(symbol)->get_name (symbol);
}

void
eek_symbol_set_label (EekSymbol   *symbol,
                      const gchar *label)
{
    g_return_if_fail (EEK_IS_SYMBOL(symbol));
    return EEK_SYMBOL_GET_CLASS(symbol)->set_label (symbol, label);
}

gchar *
eek_symbol_get_label (EekSymbol *symbol)
{
    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), NULL);
    return EEK_SYMBOL_GET_CLASS(symbol)->get_label (symbol);
}

void
eek_symbol_set_category (EekSymbol        *symbol,
                         EekSymbolCategory category)
{
    g_return_if_fail (EEK_IS_SYMBOL(symbol));
    return EEK_SYMBOL_GET_CLASS(symbol)->set_category (symbol, category);
}

EekSymbolCategory
eek_symbol_get_category (EekSymbol *symbol)
{
    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), EEK_SYMBOL_CATEGORY_UNKNOWN);
    return EEK_SYMBOL_GET_CLASS(symbol)->get_category (symbol);
}

void
eek_symbol_set_modifier_mask (EekSymbol      *symbol,
                              EekModifierType mask)
{
    g_return_if_fail (EEK_IS_SYMBOL(symbol));
    return EEK_SYMBOL_GET_CLASS(symbol)->set_modifier_mask (symbol, mask);
}

EekModifierType
eek_symbol_get_modifier_mask (EekSymbol *symbol)
{
    g_return_val_if_fail (EEK_IS_SYMBOL(symbol), 0);
    return EEK_SYMBOL_GET_CLASS(symbol)->get_modifier_mask (symbol);
}
