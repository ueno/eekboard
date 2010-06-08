#include "eek/eek-clutter.h"
#include "eek/eek-xkb.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CSW 1280
#define CSH 1024

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

gfloat stage_width, stage_height;

static gboolean
on_event (ClutterStage *stage,
	  ClutterEvent *event,
	  gpointer      user_data)
{
    if (event->type == CLUTTER_BUTTON_PRESS) {
        ClutterActor *actor = clutter_event_get_source (event);

        if (EEK_IS_KEY(actor)) {
            guint keysym;
            const gchar *label = NULL;

            keysym = eek_key_get_keysym (EEK_KEY(actor));
            if (keysym != EEK_INVALID_KEYSYM)
                label = eek_keysym_to_string (keysym);
            if (label) {
                printf ("%s", label);
                fflush (stdout);
            }
        }
        return TRUE;
    }
    return FALSE;
}

static void
on_resize (GObject *object,
	   GParamSpec *param_spec,
	   gpointer user_data)
{
  GValue value = {0};
  gfloat width, height, scale;
  ClutterActor *stage = CLUTTER_ACTOR(object);

  g_object_get (G_OBJECT(stage), "width", &width, NULL);
  g_object_get (G_OBJECT(stage), "height", &height, NULL);

  g_value_init (&value, G_TYPE_DOUBLE);

  scale = width > height ? width / stage_width : width / stage_height;

  g_value_set_double (&value, scale);
  g_object_set_property (G_OBJECT (stage),
			 "scale-x",
			 &value);

  g_value_set_double (&value, scale);
  g_object_set_property (G_OBJECT (stage),
			 "scale-y",
			 &value);
}

int
main (int argc, char *argv[])
{
    EekKeyboard *keyboard;
    EekLayout *layout;
    ClutterActor *stage;
    ClutterColor stage_color = { 0xff, 0xff, 0xff, 0xff };
    GOptionContext *context;

    context = g_option_context_new ("test-xkb-clutter");
    g_option_context_add_main_entries (context, options, NULL);
    g_option_context_parse (context, &argc, &argv, NULL);
    g_option_context_free (context);

    clutter_init (&argc, &argv);

    gtk_init (&argc, &argv);

    layout = eek_xkb_layout_new (keycodes, geometry, symbols);
    if (layout == NULL) {
        fprintf (stderr, "Failed to create layout\n");
        exit(1);
    }
    g_object_ref_sink (layout);

    keyboard = eek_clutter_keyboard_new (CSW, CSH);
    if (keyboard == NULL) {
        g_object_unref (layout);
        fprintf (stderr, "Failed to create keyboard\n");
        exit(1);
    }
    g_object_ref_sink (keyboard);

    eek_keyboard_set_layout (keyboard, layout);

    stage = clutter_stage_get_default ();

    clutter_stage_set_color (CLUTTER_STAGE(stage), &stage_color);
    clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);
    clutter_actor_get_size (CLUTTER_ACTOR(keyboard), &stage_width, &stage_height);
    clutter_actor_set_size (stage, stage_width, stage_height);

    clutter_group_add (CLUTTER_GROUP(stage), CLUTTER_ACTOR(keyboard));

    clutter_actor_show_all (stage);

    g_signal_connect (stage, 
                      "notify::width",
                      G_CALLBACK (on_resize),
                      NULL);

    g_signal_connect (stage,
                      "notify::height",
                      G_CALLBACK (on_resize),
                      NULL);

    g_signal_connect (stage, 
                      "event",
                      G_CALLBACK (on_event),
                      NULL);

    clutter_main ();

    return 0;
}
