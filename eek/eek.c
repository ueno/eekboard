#include "eek.h"

void
eek_init (void)
{
    g_type_init ();

    g_type_class_ref (EEK_TYPE_SYMBOL);
    g_type_class_ref (EEK_TYPE_KEYSYM);
}
