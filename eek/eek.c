#include "eek.h"

void
eek_init (void)
{
    g_type_init ();

    /* preload Eek* types for EekKeyboard deserialization */
    g_type_class_ref (EEK_TYPE_KEYBOARD);
    g_type_class_ref (EEK_TYPE_SECTION);
    g_type_class_ref (EEK_TYPE_KEY);
    g_type_class_ref (EEK_TYPE_SYMBOL);
    g_type_class_ref (EEK_TYPE_KEYSYM);
}
