/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
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

#include <clutter-gtk/clutter-gtk.h>
#include <fakekey/fakekey.h>
#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <libxklavier/xklavier.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek/eek-clutter.h"
#include "eek/eek-xkl.h"

#define CSW 640
#define CSH 480

static gchar *window_id = NULL;
gfloat stage_width, stage_height;
Display *display;
FakeKey *fakekey;
Window target;

EekLayout *layout;
EekKeyboard *eek_keyboard;

static const GOptionEntry options[] = {
    {"window-id", '\0', 0, G_OPTION_ARG_STRING, &window_id,
     "the target window ID; use xwininfo to obtain the value", NULL},
    {NULL},
};

static void on_monitor_key_event_toggled (GtkToggleAction *action,
                                          GtkWidget *window);

static const char ui_description[] =
    "<ui>"
    "  <menubar name='MainMenu'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
    "    <menu action='KeyboardMenu'>"
    "      <menuitem action='MonitorKeyEvent'/>"
    "      <menu action='SetLayout'>"
    "        <placeholder name='LayoutsPH'/>"
    "      </menu>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='About'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";

#define SET_LAYOUT_UI_PATH "/MainMenu/KeyboardMenu/SetLayout/LayoutsPH"

struct _LayoutVariant {
    gchar *layout;
    gchar *variant;
};
typedef struct _LayoutVariant LayoutVariant;

struct _LayoutCallbackData {
    GtkUIManager *ui_manager;
    GtkActionGroup *action_group;
    guint merge_id;
};
typedef struct _LayoutCallbackData LayoutCallbackData;

static const GtkActionEntry action_entry[] = {
    {"FileMenu", NULL, N_("_File")},
    {"KeyboardMenu", NULL, N_("_Keyboard")},
    {"HelpMenu", NULL, N_("_Help")},
    {"Quit", GTK_STOCK_QUIT, NULL, NULL, NULL, G_CALLBACK (gtk_main_quit)},
    {"SetLayout", NULL, N_("Set Layout"), NULL, NULL, NULL},
    {"About", GTK_STOCK_ABOUT, NULL, NULL, NULL, NULL}
};

static const GtkToggleActionEntry toggle_action_entry[] = {
    {"MonitorKeyEvent", NULL, N_("Monitor Key Typing"), NULL, NULL,
     G_CALLBACK(on_monitor_key_event_toggled), FALSE}
};

static void
on_monitor_key_event_toggled (GtkToggleAction *action,
                              GtkWidget *window)
{
    gboolean active;

    active = gtk_toggle_action_get_active (action);
    g_object_set (G_OBJECT(window), "accept_focus", active, NULL);
    g_object_set (G_OBJECT(window), "can_focus", active, NULL);
}

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key)
{
    fakekey_press_keysym (fakekey, eek_key_get_keysym (key), 0);
}

static void
on_key_released (EekKeyboard *keyboard,
                 EekKey      *key)
{
    fakekey_release (fakekey);
}

static void
on_activate (GtkAction *action, gpointer user_data)
{
    LayoutVariant *config = user_data;
    gchar *layouts[2], *variants[2], **vp = NULL;

    layouts[0] = config->layout;
    layouts[1] = NULL;
    if (config->variant) {
        variants[0] = config->variant;
        variants[1] = NULL;
        vp = variants;
    }
    eek_xkl_layout_set_config (EEK_XKL_LAYOUT(layout), layouts, vp, NULL);
}

static EekKeyboard *
create_keyboard (ClutterActor *stage,
                 EekLayout    *layout,
                 gfloat        width,
                 gfloat        height)
{
    EekKeyboard *keyboard;
    ClutterActor *actor;
    GValue value = {0};

    keyboard = eek_clutter_keyboard_new (width, height);
    g_signal_connect (keyboard, "key-pressed",
                      G_CALLBACK(on_key_pressed), NULL);
    g_signal_connect (keyboard, "key-released",
                      G_CALLBACK(on_key_released), NULL);
    eek_keyboard_set_layout (keyboard, layout);
    actor = eek_clutter_keyboard_get_actor (EEK_CLUTTER_KEYBOARD(keyboard));
    clutter_actor_set_name (actor, "keyboard");
    clutter_actor_get_size (actor, &stage_width, &stage_height);
    clutter_container_add_actor (CLUTTER_CONTAINER(stage), actor);
    clutter_actor_set_size (stage, stage_width, stage_height);
    return keyboard;
}

/* FIXME: EekKeyboard should handle relayout by itself. */
static void
on_changed (EekLayout *layout, gpointer user_data)
{
    ClutterActor *stage = user_data, *actor;
    gfloat width, height;

    clutter_actor_get_size (stage, &width, &height);
    actor = clutter_container_find_child_by_name (CLUTTER_CONTAINER(stage),
                                                  "keyboard");

    if (actor)
        clutter_container_remove_actor (CLUTTER_CONTAINER(stage), actor);
    g_object_unref (eek_keyboard);
    eek_keyboard = create_keyboard (stage, layout, width, height);
}

static void
variant_callback (XklConfigRegistry *registry,
                  const XklConfigItem *item,
                  gpointer user_data)
{
    GSList **r_variants = user_data;
    XklConfigItem *_item;

    _item = g_slice_dup (XklConfigItem, item);
    *r_variants = g_slist_prepend (*r_variants, _item);
}

static void
layout_callback (XklConfigRegistry *registry,
                 const XklConfigItem *item,
                 gpointer user_data)
{
    LayoutCallbackData *data = user_data;
    GtkAction *action;
    GSList *variants = NULL;
    char layout_action_name[128], variant_action_name[128];
    LayoutVariant *config;

    g_snprintf (layout_action_name, sizeof (layout_action_name),
                "SetLayout%s", item->name);
    action = gtk_action_new (layout_action_name, item->description, NULL, NULL);
    gtk_action_group_add_action (data->action_group, action);

    xkl_config_registry_foreach_layout_variant (registry,
                                                item->name,
                                                variant_callback,
                                                &variants);

    if (!variants) {
        config = g_slice_new (LayoutVariant);
        config->layout = g_strdup (item->name);
        config->variant = NULL;
        g_signal_connect (action, "activate", G_CALLBACK (on_activate), config);

        g_object_unref (action);
        gtk_ui_manager_add_ui (data->ui_manager, data->merge_id,
                               SET_LAYOUT_UI_PATH,
                               layout_action_name, layout_action_name,
                               GTK_UI_MANAGER_MENUITEM, FALSE);
    } else {
        char layout_path[128];
        GSList *head;

        g_object_unref (action);
        gtk_ui_manager_add_ui (data->ui_manager, data->merge_id,
                               SET_LAYOUT_UI_PATH,
                               layout_action_name, layout_action_name,
                               GTK_UI_MANAGER_MENU, FALSE);
        g_snprintf (layout_path, sizeof (layout_path),
                    SET_LAYOUT_UI_PATH "/%s", layout_action_name);

        for (head = variants; head; head = head->next) {
            XklConfigItem *_item = head->data;

            g_snprintf (variant_action_name, sizeof (variant_action_name),
                        "SetVariant%s%s", item->name, _item->name);
            action = gtk_action_new (variant_action_name,
                                     _item->description,
                                     NULL,
                                     NULL);

            config = g_slice_new (LayoutVariant);
            config->layout = g_strdup (item->name);
            config->variant = g_strdup (_item->name);
            g_signal_connect (action, "activate", G_CALLBACK (on_activate),
                              config);

            gtk_action_group_add_action (data->action_group, action);
            g_object_unref (action);

            gtk_ui_manager_add_ui (data->ui_manager, data->merge_id,
                                   layout_path,
                                   variant_action_name, variant_action_name,
                                   GTK_UI_MANAGER_MENUITEM, FALSE);
            g_slice_free (XklConfigItem, _item);
        }
        g_slist_free (variants);
    }
}

static void
create_layouts_menu (GtkUIManager *ui_manager)
{
    XklEngine *engine;
    XklConfigRegistry *registry;
    Display *display;
    LayoutCallbackData data;

    data.ui_manager = ui_manager;
    data.action_group = gtk_action_group_new ("Layouts");
    gtk_ui_manager_insert_action_group (data.ui_manager, data.action_group, -1);
    data.merge_id = gtk_ui_manager_new_merge_id (ui_manager);

    display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    g_return_if_fail (display);

    engine = xkl_engine_get_instance (display);
    registry = xkl_config_registry_get_instance (engine);
    xkl_config_registry_load (registry, FALSE);

    xkl_config_registry_foreach_layout (registry, layout_callback, &data);
}

static void
create_menus (GtkWidget *window, GtkUIManager * ui_manager)
{
    GtkActionGroup *action_group;

    action_group = gtk_action_group_new ("MenuActions");

    gtk_action_group_add_actions (action_group, action_entry,
                                  G_N_ELEMENTS (action_entry), window);
    gtk_action_group_add_toggle_actions (action_group, toggle_action_entry,
                                         G_N_ELEMENTS (toggle_action_entry),
                                         window);

    gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
    gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, NULL);
    create_layouts_menu (ui_manager);
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
    ClutterActor *stage;
    ClutterColor stage_color = { 0xff, 0xff, 0xff, 0xff };
    GOptionContext *context;
    GtkWidget *menubar, *embed, *vbox, *window;
    GtkUIManager *ui_manager;

    context = g_option_context_new ("eekboard");
    g_option_context_add_main_entries (context, options, NULL);
    g_option_context_parse (context, &argc, &argv, NULL);
    g_option_context_free (context);

    clutter_init (&argc, &argv);
    gtk_init (&argc, &argv);
    display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    if (!display) {
        fprintf (stderr, "Can't open display\n");
        exit (1);
    }
    if (window_id) {
        if (strncmp (window_id, "0x", 2) == 0)
            target = strtol (&window_id[2], NULL, 16);
        else
            target = strtol (window_id, NULL, 10);
    } else {
        int revert_to;
        XGetInputFocus (display, &target, &revert_to);
    }
    fakekey = fakekey_init (display);
    if (!fakekey) {
        fprintf (stderr, "Can't init fakekey\n");
        exit (1);
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_can_focus (window, FALSE);
    g_object_set (G_OBJECT(window), "accept_focus", FALSE, NULL);
    gtk_window_set_title (GTK_WINDOW(window), "Keyboard");
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);

    vbox = gtk_vbox_new (FALSE, 0);
    ui_manager = gtk_ui_manager_new ();
    create_menus (window, ui_manager);
    menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

    embed = gtk_clutter_embed_new ();
    gtk_widget_set_can_focus (embed, TRUE);
    gtk_container_add (GTK_CONTAINER(vbox), embed);
    gtk_container_add (GTK_CONTAINER(window), vbox);

    stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED(embed));
    clutter_stage_set_color (CLUTTER_STAGE(stage), &stage_color);
    clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);

    layout = eek_xkl_layout_new ();
    if (!layout) {
        fprintf (stderr, "Failed to create layout\n");
        exit (1);
    }

    clutter_actor_show (stage); /* workaround for clutter-gtk (<= 0.10.2) */
    eek_keyboard = create_keyboard (stage, layout, CSW, CSH);
    if (!eek_keyboard) {
        g_object_unref (layout);
        fprintf (stderr, "Failed to create keyboard\n");
        exit (1);
    }
    clutter_actor_get_size (stage, &stage_width, &stage_height);
    clutter_actor_show_all (stage);

    g_signal_connect (layout, "changed",
                      G_CALLBACK(on_changed), stage);

    g_signal_connect (stage, 
                      "notify::width",
                      G_CALLBACK (on_resize),
                      NULL);

    g_signal_connect (stage,
                      "notify::height",
                      G_CALLBACK (on_resize),
                      NULL);

    gtk_widget_set_size_request (embed, stage_width, stage_height);
    gtk_widget_show_all (window);
    gtk_widget_set_size_request (embed, -1, -1);

    gtk_main ();

    return 0;
}
