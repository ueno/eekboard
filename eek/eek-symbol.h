#ifndef EEK_SYMBOL_H
#define EEK_SYMBOL_H 1

#include <glib-object.h>
#include "eek-types.h"

G_BEGIN_DECLS

/**
 * EekSymbolCategory:
 * @EEK_SYMBOL_CATEGORY_LETTER: the symbol represents an alphabet letter
 * @EEK_SYMBOL_CATEGORY_FUNCTION: the symbol represents a function
 * @EEK_SYMBOL_CATEGORY_KEYNAME: the symbol does not have meaning but
 * have a name
 * @EEK_SYMBOL_CATEGORY_UNKNOWN: used for error reporting
 *
 * Category of the key symbols.
 */
typedef enum {
    EEK_SYMBOL_CATEGORY_LETTER,
    EEK_SYMBOL_CATEGORY_FUNCTION,
    EEK_SYMBOL_CATEGORY_KEYNAME,
    EEK_SYMBOL_CATEGORY_USER0,
    EEK_SYMBOL_CATEGORY_USER1,
    EEK_SYMBOL_CATEGORY_USER2,
    EEK_SYMBOL_CATEGORY_USER3,
    EEK_SYMBOL_CATEGORY_USER4,
    EEK_SYMBOL_CATEGORY_UNKNOWN,
    /*< private >*/
    EEK_SYMBOL_CATEGORY_LAST = EEK_SYMBOL_CATEGORY_UNKNOWN
} EekSymbolCategory;

#define EEK_TYPE_SYMBOL (eek_symbol_get_type())
#define EEK_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_SYMBOL, EekSymbol))
#define EEK_SYMBOL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_SYMBOL, EekSymbolClass))
#define EEK_IS_SYMBOL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_SYMBOL))
#define EEK_IS_SYMBOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_SYMBOL))
#define EEK_SYMBOL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_SYMBOL, EekSymbolClass))

typedef struct _EekSymbolClass EekSymbolClass;
typedef struct _EekSymbolPrivate EekSymbolPrivate;

struct _EekSymbol {
    /*< private >*/
    GObject parent;

    EekSymbolPrivate *priv;
};

struct _EekSymbolClass {
    /*< private >*/
    GObjectClass parent_class;

    /*< public >*/
    void                  (* set_name)          (EekSymbol        *self,
                                                 const gchar      *name);
    G_CONST_RETURN gchar *(* get_name)          (EekSymbol        *self);
    void                  (* set_label)         (EekSymbol        *self,
                                                 const gchar      *label);
    gchar                *(* get_label)         (EekSymbol        *self);
    void                  (* set_category)      (EekSymbol        *self,
                                                 EekSymbolCategory category);
    EekSymbolCategory     (* get_category)      (EekSymbol        *self);
    void                  (* set_modifier_mask) (EekSymbol        *self,
                                                 EekModifierType   mask);
    EekModifierType       (* get_modifier_mask) (EekSymbol        *self);

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType                 eek_symbol_get_type          (void) G_GNUC_CONST;

EekSymbol            *eek_symbol_new               (const gchar      *name);
void                  eek_symbol_set_name          (EekSymbol        *symbol,
                                                    const gchar      *name);
G_CONST_RETURN gchar *eek_symbol_get_name          (EekSymbol        *symbol);
void                  eek_symbol_set_label         (EekSymbol        *symbol,
                                                    const gchar      *label);
gchar                *eek_symbol_get_label         (EekSymbol        *symbol);
void                  eek_symbol_set_category      (EekSymbol        *symbol,
                                                    EekSymbolCategory category);
EekSymbolCategory     eek_symbol_get_category      (EekSymbol        *symbol);
EekModifierType       eek_symbol_get_modifier_mask (EekSymbol        *keysym);
void                  eek_symbol_set_modifier_mask (EekSymbol        *keysym,
                                                    EekModifierType   mask);

/**
 * eek_symbol_is_modifier:
 * @symbol: an #EekSymbol
 *
 * Check if @symbol is a modifier.
 * Returns: %TRUE if @symbol is a modifier.
 */
#define eek_symbol_is_modifier(symbol) \
    (eek_symbol_get_modifier_mask ((symbol)) != 0)

G_END_DECLS

#endif  /* EEK_SYMBOL_H */
