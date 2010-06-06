#include "eek/eek-gtk.h"
#include "eek/eek-xkb.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static gchar *symbols = NULL;
static gchar *keycodes = NULL;
static gchar *geometry = NULL;

static const GOptionEntry options[] = {
    {"symbols", '\0', 0, G_OPTION_ARG_STRING, &symbols,
     "Symbols component of the keyboard. If you omit this option, it is "
     "obtained from the X server; that is, the keyboard that is currently "
     "configured is drawn. Examples: --symbols=us or "
     "--symbols=us(pc104)+iso9995-3+group(switch)+ctrl(nocaps)", NULL},
    {"keycodes", '\0', 0, G_OPTION_ARG_STRING, &keycodes,
     "Keycodes component of the keyboard. If you omit this option, it is "
     "obtained from the X server; that is, the keyboard that is currently"
     " configured is drawn. Examples: --keycodes=xfree86+aliases(qwerty)",
     NULL},
    {"geometry", '\0', 0, G_OPTION_ARG_STRING, &geometry,
     "Geometry xkb component. If you omit this option, it is obtained from the"
     " X server; that is, the keyboard that is currently configured is drawn. "
     "Example: --geometry=kinesis", NULL},
    {NULL},
};

gfloat window_width, window_height;

int
main (int argc, char *argv[])
{
    EekKeyboard *keyboard;
    EekLayout *layout;
    GtkWidget *window;
    GOptionContext *context;

    context = g_option_context_new ("test-xkb-gtk");
    g_option_context_add_main_entries (context, options, NULL);
    g_option_context_parse (context, &argc, &argv, NULL);
    g_option_context_free (context);

    gtk_init (&argc, &argv);

    layout = eek_xkb_layout_new (keycodes, geometry, symbols);
    if (layout == NULL) {
        fprintf (stderr, "Failed to create layout\n");
        exit(1);
    }
    g_object_ref_sink (layout);

    keyboard = eek_gtk_keyboard_new ();
    if (keyboard == NULL) {
        g_object_unref (layout);
        fprintf (stderr, "Failed to create keyboard\n");
        exit(1);
    }
    g_object_ref_sink (keyboard);

    eek_keyboard_set_layout (keyboard, layout);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_add (GTK_CONTAINER(window), GTK_WIDGET(keyboard));

    gtk_widget_show_all (window);
    gtk_main ();

    return 0;
}
