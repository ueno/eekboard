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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <string.h>
#include <stdlib.h>
#include <gdk/gdkx.h>
#include <glib/gi18n.h>

#include "eek/eek.h"
#include "eek/eek-xkl.h"

#if HAVE_CLUTTER_GTK
#include <clutter-gtk/clutter-gtk.h>
#include "eek/eek-clutter.h"
#else  /* HAVE_CLUTTER_GTK */
#include "eek/eek-gtk.h"
#endif  /* !HAVE_CLUTTER_GTK */

#include "eekboard/eekboard-xklutil.h"

#define BUFSIZE 8192

static gchar *opt_load = NULL;
static gboolean opt_dump = FALSE;
static gchar *opt_list = NULL;
static guint opt_group = 0;
static gchar *opt_theme = NULL;

static const GOptionEntry options[] = {
    {"load", 'l', 0, G_OPTION_ARG_STRING, &opt_load,
     N_("Show the keyboard loaded from an XML file")},
    {"dump", 'd', 0, G_OPTION_ARG_NONE, &opt_dump,
     N_("Output the current layout into an XML file")},
    {"list", 'L', 0, G_OPTION_ARG_STRING, &opt_list,
     N_("List configuration items for given spec")},
    {"group", 'g', 0, G_OPTION_ARG_INT, &opt_group,
     N_("Specify group")},
    {"theme", 't', 0, G_OPTION_ARG_STRING, &opt_theme,
     N_("Specify theme")},
    {NULL}
};

#if HAVE_CLUTTER_GTK
static void
on_allocation_changed (ClutterActor          *stage,
                       ClutterActorBox       *box,
                       ClutterAllocationFlags flags,
                       gpointer               user_data)
{
    ClutterActor *actor = user_data;
    clutter_actor_set_size (actor,
                            box->x2 - box->x1,
                            box->y2 - box->y1);
}
#endif

static void
on_destroy (gpointer user_data)
{
    gtk_main_quit ();
}

static void
print_item (gpointer data,
            gpointer user_data)
{
    XklConfigItem *item = data;
    g_assert (item);
    printf ("%s: %s\n", item->name, item->description);
}

int
main (int argc, char **argv)
{
    GOptionContext *context;
    EekKeyboard *keyboard = NULL;

#if HAVE_CLUTTER_GTK
    if (gtk_clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS) {
        g_printerr ("Can't init GTK with Clutter\n");
        exit (1);
    }
#else
    if (!gtk_init_check (&argc, &argv)) {
        g_printerr ("Can't init GTK\n");
        exit (1);
    }
#endif

    g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

    context = g_option_context_new ("eek-example-xml");
    g_option_context_add_main_entries (context, options, NULL);
    g_option_context_parse (context, &argc, &argv, NULL);
    g_option_context_free (context);

    if (!opt_list && !opt_load && !opt_dump) {
        g_printerr ("Specify -l, -d, or -L option\n");
        exit (1);
    }

    if (opt_list) {
        GdkDisplay *display;
        XklEngine *engine;
        XklConfigRegistry *registry;
        GSList *items = NULL, *head;

        display = gdk_display_get_default ();
        engine = xkl_engine_get_instance (GDK_DISPLAY_XDISPLAY(display));
        registry = xkl_config_registry_get_instance (engine);
        xkl_config_registry_load (registry, FALSE);

        if (g_strcmp0 (opt_list, "model") == 0) {
            items = eekboard_xkl_list_models (registry);
        } else if (g_strcmp0 (opt_list, "layout") == 0) {
            items = eekboard_xkl_list_layouts (registry);
        } else if (g_strcmp0 (opt_list, "option-group") == 0) {
            items = eekboard_xkl_list_option_groups (registry);
        } else if (g_str_has_prefix (opt_list, "layout-variant-")) {
            items =  eekboard_xkl_list_layout_variants
                (registry,
                 opt_list + strlen ("layout-variant-"));
        } else if (g_str_has_prefix (opt_list, "option-")) {
            items = eekboard_xkl_list_options
                (registry,
                 opt_list + strlen ("option-"));
        } else {
            g_printerr ("Unknown list spec \"%s\"\n", opt_list);
        }
        g_slist_foreach (items, print_item, NULL);
        for (head = items; head; head = g_slist_next (head))
            g_object_unref (head->data);
        g_slist_free (items);
        g_object_unref (engine);
        g_object_unref (registry);
        exit (0);
    }

    if (opt_load) {
        EekLayout *layout;

        if (g_str_has_prefix (opt_load, "xkb:")) {
            XklConfigRec *rec;

            rec = eekboard_xkl_config_rec_from_string (&opt_load[4]);
            layout = eek_xkl_layout_new ();
            eek_xkl_layout_set_config (EEK_XKL_LAYOUT(layout), rec);
            g_object_unref (rec);
        } else {
            GFile *file;
            GFileInputStream *input;
            GError *error;

            file = g_file_new_for_path (opt_load);

            error = NULL;
            input = g_file_read (file, NULL, &error);
            if (error) {
                g_printerr ("Can't read file %s: %s\n",
                            opt_load, error->message);
                exit (1);
            }

            layout = eek_xml_layout_new (G_INPUT_STREAM(input));
            g_object_unref (input);
        }

        keyboard = eek_keyboard_new (layout, 640, 480);
        g_object_unref (layout);
    }

    if (!keyboard) {
        g_printerr ("Can't create keyboard\n");
        exit (1);
    }

    if (opt_dump) {
        GString *output;

        output = g_string_sized_new (BUFSIZE);
        eek_keyboard_output (keyboard, output, 0);
        g_object_unref (keyboard);
        fwrite (output->str, sizeof(gchar), output->len, stdout);
        g_string_free (output, TRUE);
        exit (0);
    } else {
        EekBounds bounds;
        GtkWidget *widget, *window;
#if HAVE_CLUTTER_GTK
        ClutterActor *stage, *actor;
        ClutterColor stage_color = { 0xff, 0xff, 0xff, 0xff };
#endif

        eek_element_set_group (EEK_ELEMENT(keyboard), opt_group);
        eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);

#if HAVE_CLUTTER_GTK
        widget = gtk_clutter_embed_new ();
        stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED(widget));
        actor = eek_clutter_keyboard_new (keyboard);
        if (opt_theme) {
            EekTheme *theme = eek_theme_new (opt_theme, NULL, NULL);

            eek_clutter_keyboard_set_theme (EEK_CLUTTER_KEYBOARD(actor), theme);
            g_object_unref (theme);
        }
        clutter_container_add_actor (CLUTTER_CONTAINER(stage), actor);

        clutter_stage_set_color (CLUTTER_STAGE(stage), &stage_color);
        clutter_stage_set_user_resizable (CLUTTER_STAGE(stage), TRUE);
        clutter_stage_set_minimum_size (CLUTTER_STAGE(stage),
                                        bounds.width / 3,
                                        bounds.height / 3);
        g_signal_connect (stage,
                          "allocation-changed",
                          G_CALLBACK(on_allocation_changed),
                          actor);
#else
        widget = eek_gtk_keyboard_new (keyboard);
        if (opt_theme) {
            EekTheme *theme = eek_theme_new (opt_theme, NULL, NULL);

            eek_gtk_keyboard_set_theme (EEK_GTK_KEYBOARD(widget), theme);
            g_object_unref (theme);
        }
#endif
        g_object_unref (keyboard);

        gtk_widget_set_size_request (widget, bounds.width, bounds.height);

        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_container_add (GTK_CONTAINER(window), widget);
        gtk_widget_show_all (window);

        g_signal_connect (G_OBJECT (window), "destroy",
                          G_CALLBACK (on_destroy), NULL);

        gtk_main ();
        exit (0);
    }

    return 0;
}
