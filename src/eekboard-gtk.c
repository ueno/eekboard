/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#include "eek/eek-gtk.h"
#include "eek/eek-xkl.h"

#define CSW 640
#define CSH 480

#define LICENSE \
    "This program is free software: you can redistribute it and/or modify " \
    "it under the terms of the GNU General Public License as published by " \
    "the Free Software Foundation, either version 3 of the License, or " \
    "(at your option) any later version." \
    "\n\n" \
    "This program is distributed in the hope that it will be useful, " \
    "but WITHOUT ANY WARRANTY; without even the implied warranty of " \
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the " \
    "GNU General Public License for more details." \
    "\n\n" \
    "You should have received a copy of the GNU General Public License " \
    "along with this program.  If not, see <http://www.gnu.org/licenses/>. " \

struct _EekBoard {
    EekKeyboard *keyboard;
    EekLayout *layout;          /* FIXME: eek_keyboard_get_layout() */
    Display *display;
    FakeKey *fakekey;
    guint modifiers;
    gfloat width, height;
};
typedef struct _EekBoard EekBoard;

static void on_about (GtkAction * action, GtkWidget *window);
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

struct _ConfigCallbackData {
    EekBoard *eekboard;
    XklConfigRec *config;
};
typedef struct _ConfigCallbackData ConfigCallbackData;

struct _LayoutCallbackData {
    EekBoard *eekboard;
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
    {"About", GTK_STOCK_ABOUT, NULL, NULL, NULL, G_CALLBACK (on_about)}
};

static const GtkToggleActionEntry toggle_action_entry[] = {
    {"MonitorKeyEvent", NULL, N_("Monitor Key Typing"), NULL, NULL,
     G_CALLBACK(on_monitor_key_event_toggled), FALSE}
};

static void
on_about (GtkAction * action, GtkWidget *window)
{
  const gchar *authors[] = { "Daiki Ueno", NULL };

  gtk_show_about_dialog (GTK_WINDOW (window),
                         "version", VERSION,
                         "copyright",
                         "Copyright \xc2\xa9 2010 Daiki Ueno\n"
                         "Copyright \xc2\xa9 2010 Red Hat, Inc.",
                         "license", LICENSE,
                         "comments",
                         _("A virtual keyboard for GNOME"),
                         "authors", authors,
                         "website",
                         "http://github.com/ueno/eek/",
                         "website-label", _("EekBoard web site"),
                         "wrap-license", TRUE, NULL);
}

static void
on_monitor_key_event_toggled (GtkToggleAction *action,
                              GtkWidget *window)
{
    g_warning ("not implemented");
}

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer user_data)
{
    EekBoard *eekboard = user_data;
    guint keysym;

    keysym = eek_key_get_keysym (key);
#if DEBUG
    g_debug ("%s %X", eek_keysym_to_string (keysym), eekboard->modifiers);
#endif
    fakekey_press_keysym (eekboard->fakekey, keysym, eekboard->modifiers);
    if (keysym == XK_Shift_L || keysym == XK_Shift_R) {
        gint group, level;

        eekboard->modifiers ^= ShiftMask;
        eek_keyboard_get_keysym_index (keyboard, &group, &level);
        eek_keyboard_set_keysym_index (keyboard, group,
                                       eekboard->modifiers & ShiftMask ? 1 : 0);
    } else if (keysym == XK_Control_L || keysym == XK_Control_R) {
        eekboard->modifiers ^= ControlMask;
    } else if (keysym == XK_Alt_L || keysym == XK_Alt_R) {
        eekboard->modifiers ^= Mod1Mask;
    }
}

static void
on_key_released (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer user_data)
{
    EekBoard *eekboard = user_data;
    fakekey_release (eekboard->fakekey);
}

static void
on_activate (GtkAction *action, gpointer user_data)
{
    ConfigCallbackData *data = user_data;

    eek_xkl_layout_set_config (EEK_XKL_LAYOUT(data->eekboard->layout),
                               data->config);
    g_object_unref (data->config);
}

#if 0
static void
create_keyboard (EekBoard  *eekboard,
                 ClutterActor *stage,
                 EekLayout *layout,
                 gfloat     initial_width,
                 gfloat     initial_height)
{
    ClutterActor *actor;

    eekboard->keyboard = eek_clutter_keyboard_new (initial_width,
                                                   initial_height);
    g_signal_connect (eekboard->keyboard, "key-pressed",
                      G_CALLBACK(on_key_pressed), eekboard);
    g_signal_connect (eekboard->keyboard, "key-released",
                      G_CALLBACK(on_key_released), eekboard);
    eek_keyboard_set_layout (eekboard->keyboard, layout);
    actor = eek_clutter_keyboard_get_actor
        (EEK_CLUTTER_KEYBOARD(eekboard->keyboard));
    clutter_actor_get_size (actor,
                            &eekboard->width,
                            &eekboard->height);
    clutter_container_add_actor (CLUTTER_CONTAINER(stage), actor);
    clutter_actor_set_size (stage,
                            eekboard->width,
                            eekboard->height);
}

/* FIXME: EekKeyboard should handle relayout by itself. */
static void
on_changed (EekLayout *layout, gpointer user_data)
{
    EekBoard *eekboard = user_data;
    ClutterActor *stage, *actor;
    gfloat width, height;

    actor = eek_clutter_keyboard_get_actor
        (EEK_CLUTTER_KEYBOARD(eekboard->keyboard));
    stage = clutter_actor_get_stage (actor);
    clutter_actor_get_size (stage, &width, &height);
    clutter_container_remove_actor (CLUTTER_CONTAINER(stage), actor);
    g_object_unref (eekboard->keyboard);
    create_keyboard (eekboard, stage, layout, width, height);
    clutter_actor_get_size (stage, &eekboard->width, &eekboard->height);
}
#endif

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
    ConfigCallbackData *config;

    g_snprintf (layout_action_name, sizeof (layout_action_name),
                "SetLayout%s", item->name);
    action = gtk_action_new (layout_action_name, item->description, NULL, NULL);
    gtk_action_group_add_action (data->action_group, action);

    xkl_config_registry_foreach_layout_variant (registry,
                                                item->name,
                                                variant_callback,
                                                &variants);

    if (!variants) {
        config = g_slice_new (ConfigCallbackData);
        config->eekboard = data->eekboard;
        config->config = xkl_config_rec_new ();
        config->config->layouts = g_new0 (char *, 2);
        config->config->layouts[0] = g_strdup (item->name);
        config->config->layouts[1] = NULL;
        config->config->variants = NULL;
        g_signal_connect (action, "activate", G_CALLBACK (on_activate),
                          config);
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

            config = g_slice_new (ConfigCallbackData);
            config->eekboard = data->eekboard;
            config->config = xkl_config_rec_new ();
            config->config->layouts = g_new0 (char *, 2);
            config->config->layouts[0] = g_strdup (item->name);
            config->config->layouts[1] = NULL;
            config->config->variants = g_new0 (char *, 2);
            config->config->variants[0] = g_strdup (_item->name);
            config->config->variants[1] = NULL;
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
create_layouts_menu (EekBoard *eekboard, GtkUIManager *ui_manager)
{
    XklEngine *engine;
    XklConfigRegistry *registry;
    LayoutCallbackData data;

    data.eekboard = eekboard;
    data.ui_manager = ui_manager;
    data.action_group = gtk_action_group_new ("Layouts");
    gtk_ui_manager_insert_action_group (data.ui_manager, data.action_group, -1);
    data.merge_id = gtk_ui_manager_new_merge_id (ui_manager);

    engine = xkl_engine_get_instance (eekboard->display);
    registry = xkl_config_registry_get_instance (engine);
    xkl_config_registry_load (registry, FALSE);

    xkl_config_registry_foreach_layout (registry, layout_callback, &data);
}

static void
create_menus (EekBoard      *eekboard,
              GtkWidget     *window,
              GtkUIManager * ui_manager)
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
    create_layouts_menu (eekboard, ui_manager);
}

#if 0
static void
on_resize (GObject *object,
	   GParamSpec *param_spec,
	   gpointer user_data)
{
    EekBoard *eekboard = user_data;
    GValue value = {0};
    gfloat width, height, scale;
    ClutterActor *stage, *actor;

    actor = eek_clutter_keyboard_get_actor
        (EEK_CLUTTER_KEYBOARD(eekboard->keyboard));
    stage = clutter_actor_get_stage (actor);

    g_object_get (G_OBJECT(stage), "width", &width, NULL);
    g_object_get (G_OBJECT(stage), "height", &height, NULL);

    g_value_init (&value, G_TYPE_DOUBLE);

    scale = width > height ? width / eekboard->width :
        width / eekboard->height;

    g_value_set_double (&value, scale);
    g_object_set_property (G_OBJECT (stage),
                           "scale-x",
                           &value);

    g_value_set_double (&value, scale);
    g_object_set_property (G_OBJECT (stage),
                           "scale-y",
                           &value);
}
#endif

int
main (int argc, char *argv[])
{
    EekBoard eekboard;
    GtkWidget *menubar, *embed, *vbox, *window;
    GtkUIManager *ui_manager;
    EekBounds bounds;

    if (!gtk_init_check (&argc, &argv)) {
        fprintf (stderr, "Can't init Clutter-Gtk\n");
        exit (1);
    }

    memset (&eekboard, 0, sizeof eekboard);
    eekboard.display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    if (!eekboard.display) {
        fprintf (stderr, "Can't open display\n");
        exit (1);
    }

    eekboard.fakekey = fakekey_init (eekboard.display);
    if (!eekboard.fakekey) {
        fprintf (stderr, "Can't init fakekey\n");
        exit (1);
    }

    eekboard.layout = eek_xkl_layout_new ();
    if (!eekboard.layout) {
        fprintf (stderr, "Failed to create layout\n");
        exit (1);
    }

#if 0
    g_signal_connect (eekboard.layout,
                      "changed",
                      G_CALLBACK(on_changed),
                      &eekboard);

    g_signal_connect (stage, 
                      "notify::width",
                      G_CALLBACK (on_resize),
                      &eekboard);

    g_signal_connect (stage,
                      "notify::height",
                      G_CALLBACK (on_resize),
                      &eekboard);
#endif

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_can_focus (window, FALSE);
    g_object_set (G_OBJECT(window), "accept_focus", FALSE, NULL);
    gtk_window_set_title (GTK_WINDOW(window), "Keyboard");
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);

    bounds.x = bounds.y = 0;
    bounds.width = CSW;
    bounds.height = CSH;
    eekboard.keyboard = eek_gtk_keyboard_new ();
    eek_element_set_bounds (EEK_ELEMENT(eekboard.keyboard), &bounds);
    eek_keyboard_set_layout (eekboard.keyboard, eekboard.layout);
    embed = eek_gtk_keyboard_get_widget (EEK_GTK_KEYBOARD (eekboard.keyboard));

    vbox = gtk_vbox_new (FALSE, 0);
    ui_manager = gtk_ui_manager_new ();
    create_menus (&eekboard, window, ui_manager);
    menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER(vbox), embed);
    gtk_container_add (GTK_CONTAINER(window), vbox);

    gtk_widget_set_size_request (embed, CSW, CSH);
    gtk_widget_show_all (window);
    //gtk_widget_set_size_request (embed, -1, -1);

    gtk_main ();

    return 0;
}
