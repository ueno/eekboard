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

#if HAVE_CLUTTER_GTK
#include <clutter-gtk/clutter-gtk.h>
#endif

#include <fakekey/fakekey.h>
#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <libxklavier/xklavier.h>
#include <atk/atk.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#if HAVE_CLUTTER_GTK
#include "eek/eek-clutter.h"
#endif

#include "eek/eek-gtk.h"
#include "eek/eek-xkl.h"

#define CSW 640
#define CSH 480

#if HAVE_CLUTTER_GTK
#define USE_CLUTTER TRUE
#else
#define USE_CLUTTER FALSE
#endif

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

struct _Eekboard {
    gboolean use_clutter;
    Display *display;
    FakeKey *fakekey;
    GtkWidget *widget;
    gint width, height;
    guint key_event_listener;

    EekKeyboard *keyboard;
    EekLayout *layout;          /* FIXME: eek_keyboard_get_layout() */
    guint modifiers;
};
typedef struct _Eekboard Eekboard;

struct _ConfigCallbackData {
    Eekboard *eekboard;
    XklConfigRec *config;
};
typedef struct _ConfigCallbackData ConfigCallbackData;

struct _LayoutCallbackData {
    Eekboard *eekboard;
    GtkUIManager *ui_manager;
    GtkActionGroup *action_group;
    guint merge_id;
};
typedef struct _LayoutCallbackData LayoutCallbackData;

static void       on_about      (GtkAction       *action,
                                 GtkWidget       *window);
static void       on_monitor_key_event_toggled
                                (GtkToggleAction *action,
                                 GtkWidget       *window);
static GtkWidget *create_widget (Eekboard        *eekboard,
                                 gint           initial_width,
                                 gint           initial_height);
static GtkWidget *create_widget_gtk (Eekboard        *eekboard,
                                     gint           initial_width,
                                     gint           initial_height);

#if !HAVE_CLUTTER_GTK
#define create_widget create_widget_gtk
#endif

static const char ui_description[] =
    "<ui>"
    "  <menubar name='MainMenu'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='Quit'/>"
    "    </menu>"
    "    <menu action='KeyboardMenu'>"
#if 0
    "      <menuitem action='MonitorKeyEvent'/>"
#endif
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
                         "program-name", PACKAGE,
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
                         "website-label", _("Eekboard web site"),
                         "wrap-license", TRUE, NULL);
}

static gint
key_snoop (AtkKeyEventStruct *event, gpointer func_data)
{
    g_debug ("key_snoop");
    return FALSE;
}

static void
on_monitor_key_event_toggled (GtkToggleAction *action,
                              GtkWidget *window)
{

    Eekboard *eekboard = g_object_get_data (G_OBJECT(window), "eekboard");

    if (gtk_toggle_action_get_active (action)) {
        if (eekboard->key_event_listener == 0)
            eekboard->key_event_listener =
                atk_add_key_event_listener (key_snoop, eekboard);
        g_warning ("failed to enable ATK key event listener");
    } else
        if (eekboard->key_event_listener != 0) {
            atk_remove_key_event_listener (eekboard->key_event_listener);
            eekboard->key_event_listener = 0;
        }
}

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer user_data)
{
    Eekboard *eekboard = user_data;
    guint keysym;

    keysym = eek_key_get_keysym (key);
#if DEBUG
    g_debug ("%s %X", eek_keysym_to_string (keysym), eekboard->modifiers);
#endif
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
    } else
        fakekey_press_keysym (eekboard->fakekey, keysym, eekboard->modifiers);
}

static void
on_key_released (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer user_data)
{
    Eekboard *eekboard = user_data;
    fakekey_release (eekboard->fakekey);
}

static void
on_activate (GtkAction *action,
             gpointer   user_data)
{
    ConfigCallbackData *data = user_data;
    eek_xkl_layout_set_config (EEK_XKL_LAYOUT(data->eekboard->layout),
                               data->config);
}

static void
on_changed (EekLayout *layout, gpointer user_data)
{
    Eekboard *eekboard = user_data;
    GtkWidget *vbox, *widget;

    vbox = gtk_widget_get_parent (eekboard->widget);
    /* gtk_widget_destroy() seems not usable for GtkClutterEmbed */
    gtk_container_remove (GTK_CONTAINER(vbox), eekboard->widget);

    g_object_unref (eekboard->keyboard);
    widget = create_widget (eekboard, eekboard->width, eekboard->height);
    gtk_container_add (GTK_CONTAINER(vbox), widget);
    gtk_widget_show_all (vbox);
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
        /* reset the existing variant setting */
        config->config->variants = g_new0 (char *, 1);
        config->config->variants[0] = NULL;
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
create_layouts_menu (Eekboard *eekboard, GtkUIManager *ui_manager)
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
create_menus (Eekboard      *eekboard,
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

#if HAVE_CLUTTER_GTK
static GtkWidget *
create_widget_clutter (Eekboard *eekboard,
                       gint      initial_width,
                       gint      initial_height)
{
    ClutterActor *stage, *actor;
    ClutterColor stage_color = { 0xff, 0xff, 0xff, 0xff };
    EekBounds bounds;

    bounds.x = bounds.y = 0;
    bounds.width = initial_width;
    bounds.height = initial_height;

    eekboard->keyboard = eek_clutter_keyboard_new ();
    eek_keyboard_set_layout (eekboard->keyboard, eekboard->layout);
    eek_element_set_bounds (EEK_ELEMENT(eekboard->keyboard), &bounds);
    g_signal_connect (eekboard->keyboard, "key-pressed",
                      G_CALLBACK(on_key_pressed), eekboard);
    g_signal_connect (eekboard->keyboard, "key-released",
                      G_CALLBACK(on_key_released), eekboard);

    eekboard->widget = gtk_clutter_embed_new ();
    stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED(eekboard->widget));
    clutter_stage_set_color (CLUTTER_STAGE(stage), &stage_color);
    clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);

    actor = eek_clutter_keyboard_get_actor
        (EEK_CLUTTER_KEYBOARD(eekboard->keyboard));
    clutter_actor_get_size (actor, &eekboard->width, &eekboard->height);
    clutter_container_add_actor (CLUTTER_CONTAINER(stage), actor);
    clutter_actor_set_size (stage, eekboard->width, eekboard->height);
    return eekboard->widget;
}
#endif

static GtkWidget *
create_widget_gtk (Eekboard *eekboard,
                   gint      initial_width,
                   gint      initial_height)
{
    EekBounds bounds;

    bounds.x = bounds.y = 0;
    bounds.width = initial_width;
    bounds.height = initial_height;

    eekboard->keyboard = eek_gtk_keyboard_new ();
    eek_keyboard_set_layout (eekboard->keyboard, eekboard->layout);
    eek_element_set_bounds (EEK_ELEMENT(eekboard->keyboard), &bounds);
    g_signal_connect (eekboard->keyboard, "key-pressed",
                      G_CALLBACK(on_key_pressed), eekboard);
    g_signal_connect (eekboard->keyboard, "key-released",
                      G_CALLBACK(on_key_released), eekboard);

    eekboard->widget =
        eek_gtk_keyboard_get_widget (EEK_GTK_KEYBOARD (eekboard->keyboard));
    eek_element_get_bounds (EEK_ELEMENT(eekboard->keyboard), &bounds);
    eekboard->width = bounds.width;
    eekboard->height = bounds.height;
    return eekboard->widget;
}

#if HAVE_CLUTTER_GTK
static GtkWidget *
create_widget (Eekboard *eekboard,
               gint    initial_width,
               gint    initial_height)
{
    if (eekboard->use_clutter)
        return create_widget_clutter (eekboard, initial_width, initial_height);
    else
        return create_widget_gtk (eekboard, initial_width, initial_height);
}
#endif

Eekboard *
eekboard_new (gboolean use_clutter)
{
    Eekboard *eekboard;

    eekboard = g_slice_new0 (Eekboard);
    eekboard->use_clutter = use_clutter;
    eekboard->display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    if (!eekboard->display) {
        g_slice_free (Eekboard, eekboard);
        g_warning ("Can't open display");
        return NULL;
    }

    eekboard->fakekey = fakekey_init (eekboard->display);
    if (!eekboard->fakekey) {
        g_slice_free (Eekboard, eekboard);
        g_warning ("Can't init fakekey");
        return NULL;
    }

    eekboard->layout = eek_xkl_layout_new ();
    if (!eekboard->layout) {
        g_slice_free (Eekboard, eekboard);
        g_warning ("Can't create layout");
        return NULL;
    }
    g_signal_connect (eekboard->layout, "changed",
                      G_CALLBACK(on_changed), eekboard);

    create_widget (eekboard, CSW, CSH);

    return eekboard;
}

int
main (int argc, char *argv[])
{
    const gchar *env;
    gboolean use_clutter = USE_CLUTTER;
    Eekboard *eekboard;
    GtkWidget *widget, *vbox, *menubar, *window;
    GtkUIManager *ui_manager;

    env = g_getenv ("EEKBOARD_DISABLE_CLUTTER");
    if (env && g_strcmp0 (env, "1") == 0)
        use_clutter = FALSE;

#if HAVE_CLUTTER_GTK
    if (use_clutter &&
        gtk_clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS) {
        g_warning ("Can't init Clutter-Gtk...fallback to GTK");
        use_clutter = FALSE;
    }
#endif

    if (!use_clutter && !gtk_init_check (&argc, &argv)) {
        g_warning ("Can't init GTK");
        exit (1);
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_can_focus (window, FALSE);
    g_object_set (G_OBJECT(window), "accept_focus", FALSE, NULL);
    gtk_window_set_title (GTK_WINDOW(window), "Keyboard");
    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);

    vbox = gtk_vbox_new (FALSE, 0);

    eekboard = eekboard_new (use_clutter);
    g_object_set_data (G_OBJECT(window), "eekboard", eekboard);
    widget = create_widget (eekboard, CSW, CSH);
    
    ui_manager = gtk_ui_manager_new ();
    create_menus (eekboard, window, ui_manager);
    menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER(vbox), widget);
    gtk_container_add (GTK_CONTAINER(window), vbox);
  
    gtk_widget_set_size_request (widget, eekboard->width, eekboard->height);
    gtk_widget_show_all (window);
    gtk_widget_set_size_request (widget, -1, -1);

    gtk_main ();

    return 0;
}
