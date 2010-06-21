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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#if HAVE_CLUTTER_GTK
#include <clutter-gtk/clutter-gtk.h>
#endif

#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <libxklavier/xklavier.h>
#include <fakekey/fakekey.h>
#if 0
#include <atk/atk.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

#ifdef EEKBOARD_ENABLE_DEBUG
#define EEKBOARD_NOTE(x,a...) g_message (G_STRLOC ": " x, ##a);
#else
#define EEKBOARD_NOTE(x,a...)
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
    XklEngine *engine;
    XklConfigRegistry *registry;
    GtkUIManager *ui_manager;

    guint countries_merge_id;
    GtkActionGroup *countries_action_group;

    guint languages_merge_id;
    GtkActionGroup *languages_action_group;

    guint models_merge_id;
    GtkActionGroup *models_action_group;

    guint layouts_merge_id;
    GtkActionGroup *layouts_action_group;

    guint options_merge_id;
    GtkActionGroup *options_action_group;

    EekKeyboard *keyboard;
    EekLayout *layout;          /* FIXME: eek_keyboard_get_layout() */
    guint modifiers;
};
typedef struct _Eekboard Eekboard;

struct _SetConfigCallbackData {
    Eekboard *eekboard;
    XklConfigRec *config;
};
typedef struct _SetConfigCallbackData SetConfigCallbackData;

struct _CreateMenuCallbackData {
    Eekboard *eekboard;
    GtkActionGroup *action_group;
    guint merge_id;
};
typedef struct _CreateMenuCallbackData CreateMenuCallbackData;

struct _LayoutVariant {
    XklConfigItem *layout;
    XklConfigItem *variant;
};
typedef struct _LayoutVariant LayoutVariant;

static void       on_countries_menu (GtkAction       *action,
                                     GtkWidget       *widget);
static void       on_languages_menu (GtkAction       *action,
                                     GtkWidget       *widget);
static void       on_models_menu    (GtkAction       *action,
                                     GtkWidget       *window);
static void       on_layouts_menu   (GtkAction       *action,
                                     GtkWidget       *window);
static void       on_options_menu   (GtkAction       *action,
                                     GtkWidget       *window);
static void       on_about          (GtkAction       *action,
                                     GtkWidget       *window);
#if 0
static void       on_monitor_key_event_toggled
                                    (GtkToggleAction *action,
                                     GtkWidget       *window);
#endif
static GtkWidget *create_widget     (Eekboard        *eekboard,
                                     gint             initial_width,
                                     gint             initial_height);
static GtkWidget *create_widget_gtk (Eekboard        *eekboard,
                                     gint             initial_width,
                                     gint             initial_height);

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
    "      <menu action='Country'>"
    "        <placeholder name='CountriesPH'/>"
    "      </menu>"
    "      <menu action='Language'>"
    "        <placeholder name='LanguagesPH'/>"
    "      </menu>"
    "      <separator/>"
    "      <menu action='Model'>"
    "        <placeholder name='ModelsPH'/>"
    "      </menu>"
    "      <menu action='Layout'>"
    "        <placeholder name='LayoutsPH'/>"
    "      </menu>"
    "      <menu action='Option'>"
    "        <placeholder name='OptionsPH'/>"
    "      </menu>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='About'/>"
    "    </menu>"
    "  </menubar>"
    "</ui>";

#define COUNTRIES_UI_PATH "/MainMenu/KeyboardMenu/Country/CountriesPH"
#define LANGUAGES_UI_PATH "/MainMenu/KeyboardMenu/Language/LanguagesPH"
#define MODELS_UI_PATH "/MainMenu/KeyboardMenu/Model/ModelsPH"
#define LAYOUTS_UI_PATH "/MainMenu/KeyboardMenu/Layout/LayoutsPH"
#define OPTIONS_UI_PATH "/MainMenu/KeyboardMenu/Option/OptionsPH"

static const GtkActionEntry action_entry[] = {
    {"FileMenu", NULL, N_("_File")},
    {"KeyboardMenu", NULL, N_("_Keyboard")},
    {"HelpMenu", NULL, N_("_Help")},
    {"Quit", GTK_STOCK_QUIT, NULL, NULL, NULL, G_CALLBACK (gtk_main_quit)},
    {"Country", NULL, N_("Country"), NULL, NULL,
     G_CALLBACK(on_countries_menu)},
    {"Language", NULL, N_("Language"), NULL, NULL,
     G_CALLBACK(on_languages_menu)},
    {"Model", NULL, N_("Model"), NULL, NULL,
     G_CALLBACK(on_models_menu)},
    {"Layout", NULL, N_("Layout"), NULL, NULL,
     G_CALLBACK(on_layouts_menu)},
    {"Option", NULL, N_("Option"), NULL, NULL,
     G_CALLBACK(on_options_menu)},
    {"About", GTK_STOCK_ABOUT, NULL, NULL, NULL, G_CALLBACK (on_about)}
};

#if 0
static const GtkToggleActionEntry toggle_action_entry[] = {
    {"MonitorKeyEvent", NULL, N_("Monitor Key Typing"), NULL, NULL,
     G_CALLBACK(on_monitor_key_event_toggled), FALSE}
};
#endif

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

#if 0
static gint
key_snoop (AtkKeyEventStruct *event, gpointer func_data)
{
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
#endif

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer user_data)
{
    Eekboard *eekboard = user_data;
    guint keysym;

    keysym = eek_key_get_keysym (key);
    EEKBOARD_NOTE("%s %X", eek_keysym_to_string (keysym), eekboard->modifiers);
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
on_config_activate (GtkAction *action,
             gpointer   user_data)
{
    SetConfigCallbackData *data = user_data;
    eek_xkl_layout_set_config (EEK_XKL_LAYOUT(data->eekboard->layout),
                               data->config);
}

static void
on_option_toggled (GtkToggleAction *action,
                   gpointer         user_data)
{
    SetConfigCallbackData *data = user_data;
    if (gtk_toggle_action_get_active (action))
        eek_xkl_layout_enable_option (EEK_XKL_LAYOUT(data->eekboard->layout),
                                      data->config->options[0]);
    else
        eek_xkl_layout_disable_option (EEK_XKL_LAYOUT(data->eekboard->layout),
                                       data->config->options[0]);
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
collect_layout_variant (XklConfigRegistry *registry,
                        const XklConfigItem *layout,
                        const XklConfigItem *variant,
                        gpointer user_data)
{
    GSList **r_lv_list = user_data;
    LayoutVariant *lv;

    lv = g_slice_new0 (LayoutVariant);
    lv->layout = g_slice_dup (XklConfigItem, layout);
    if (variant)
        lv->variant = g_slice_dup (XklConfigItem, variant);
    *r_lv_list = g_slist_prepend (*r_lv_list, lv);
}

static void
add_layout_variant_actions (CreateMenuCallbackData *data,
                            const gchar            *name,
                            const gchar            *path,
                            GSList                 *lv_list)
{
    gchar variant_action_name[128];
    SetConfigCallbackData *config;
    GSList *head;

    for (head = lv_list; head; head = head->next) {
        LayoutVariant *lv = head->data;
        GtkAction *action;
        gchar description[XKL_MAX_CI_DESC_LENGTH * 2 + 4];

        if (lv->variant) {
            g_snprintf (variant_action_name, sizeof (variant_action_name),
                        "SetLayoutVariant %s %s %s",
                        name,
                        lv->layout->name,
                        lv->variant->name);
            g_snprintf (description, sizeof (description),
                        "%s (%s)",
                        lv->layout->description,
                        lv->variant->description);
        } else {
            g_snprintf (variant_action_name, sizeof (variant_action_name),
                        "SetLayout %s %s",
                        name,
                        lv->layout->name);
            g_strlcpy (description,
                       lv->layout->description,
                       sizeof (description));
        }

        action = gtk_action_new (variant_action_name,
                                 description,
                                 NULL,
                                 NULL);

        config = g_slice_new (SetConfigCallbackData);
        config->eekboard = data->eekboard;
        config->config = xkl_config_rec_new ();
        config->config->layouts = g_new0 (char *, 2);
        config->config->layouts[0] = g_strdup (lv->layout->name);
        config->config->layouts[1] = NULL;
        if (lv->variant) {
            config->config->variants = g_new0 (char *, 2);
            config->config->variants[0] = g_strdup (lv->variant->name);
            config->config->variants[1] = NULL;
        }
        g_signal_connect (action, "activate", G_CALLBACK (on_config_activate), config);

        gtk_action_group_add_action (data->action_group, action);
        g_object_unref (action);

        gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                               path,
                               variant_action_name, variant_action_name,
                               GTK_UI_MANAGER_MENUITEM, FALSE);

        g_slice_free (XklConfigItem, lv->layout);
        if (lv->variant)
            g_slice_free (XklConfigItem, lv->variant);
        g_slice_free (LayoutVariant, lv);
    }
}

static void
country_callback (XklConfigRegistry   *registry,
                  const XklConfigItem *item,
                  gpointer             user_data)
{
    CreateMenuCallbackData *data = user_data;
    GtkAction *action;
    GSList *lv_list = NULL;
    char country_action_name[128];
    char country_action_path[128];

    g_snprintf (country_action_name, sizeof (country_action_name),
                "Country %s", item->name);
    action = gtk_action_new (country_action_name, item->description,
                             NULL, NULL);
    gtk_action_group_add_action (data->action_group, action);
    g_object_unref (action);

    gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                           COUNTRIES_UI_PATH,
                           country_action_name, country_action_name,
                           GTK_UI_MANAGER_MENU, FALSE);
    g_snprintf (country_action_path, sizeof (country_action_path),
                COUNTRIES_UI_PATH "/%s", country_action_name);

    xkl_config_registry_foreach_country_variant (data->eekboard->registry,
                                                 item->name,
                                                 collect_layout_variant,
                                                 &lv_list);
    add_layout_variant_actions (data, item->name, country_action_path,
                                lv_list);
    g_slist_free (lv_list);
}

static guint
create_countries_menu (Eekboard *eekboard)
{
    CreateMenuCallbackData data;

    data.eekboard = eekboard;
    data.merge_id = gtk_ui_manager_new_merge_id (eekboard->ui_manager);
    data.action_group = eekboard->countries_action_group;

    xkl_config_registry_foreach_country (eekboard->registry,
                                         country_callback,
                                         &data);
    return data.merge_id;
}

static void
on_countries_menu (GtkAction *action, GtkWidget *window)
{
    Eekboard *eekboard = g_object_get_data (G_OBJECT(window), "eekboard");
    if (eekboard->countries_merge_id == 0)
        eekboard->countries_merge_id = create_countries_menu (eekboard);
}

static void
language_callback (XklConfigRegistry *registry,
                   const XklConfigItem *item,
                   gpointer user_data)
{
    CreateMenuCallbackData *data = user_data;
    GtkAction *action;
    GSList *lv_list = NULL;
    char language_action_name[128];
    char language_action_path[128];

    g_snprintf (language_action_name, sizeof (language_action_name),
                "Language %s", item->name);
    action = gtk_action_new (language_action_name, item->description,
                             NULL, NULL);
    gtk_action_group_add_action (data->action_group, action);
    g_object_unref (action);

    gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                           LANGUAGES_UI_PATH,
                           language_action_name, language_action_name,
                           GTK_UI_MANAGER_MENU, FALSE);
    g_snprintf (language_action_path, sizeof (language_action_path),
                LANGUAGES_UI_PATH "/%s", language_action_name);

    xkl_config_registry_foreach_language_variant (data->eekboard->registry,
                                                  item->name,
                                                  collect_layout_variant,
                                                  &lv_list);
    add_layout_variant_actions (data, item->name, language_action_path,
                                lv_list);
    g_slist_free (lv_list);
}

static guint
create_languages_menu (Eekboard *eekboard)
{
    CreateMenuCallbackData data;

    data.eekboard = eekboard;
    data.merge_id = gtk_ui_manager_new_merge_id (eekboard->ui_manager);
    data.action_group = eekboard->languages_action_group;

    xkl_config_registry_foreach_language (eekboard->registry,
                                          language_callback,
                                          &data);
    return data.merge_id;
}

static void
on_languages_menu (GtkAction *action, GtkWidget *window)
{
    Eekboard *eekboard = g_object_get_data (G_OBJECT(window), "eekboard");
    if (eekboard->languages_merge_id == 0)
        eekboard->languages_merge_id = create_languages_menu (eekboard);
}

static void
model_callback (XklConfigRegistry *registry,
                const XklConfigItem *item,
                gpointer user_data)
{
    CreateMenuCallbackData *data = user_data;
    GtkAction *action;
    char model_action_name[128];
    SetConfigCallbackData *config;

    g_snprintf (model_action_name, sizeof (model_action_name),
                "Model %s", item->name);
    action = gtk_action_new (model_action_name, item->description, NULL, NULL);
    gtk_action_group_add_action (data->action_group, action);

    config = g_slice_new (SetConfigCallbackData);
    config->eekboard = data->eekboard;
    config->config = xkl_config_rec_new ();
    config->config->model = g_strdup (item->name);

    g_signal_connect (action, "activate", G_CALLBACK (on_config_activate),
                      config);
    g_object_unref (action);
    gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                           MODELS_UI_PATH,
                           model_action_name, model_action_name,
                           GTK_UI_MANAGER_MENUITEM, FALSE);
}

static guint
create_models_menu (Eekboard *eekboard)
{
    CreateMenuCallbackData data;

    data.eekboard = eekboard;
    data.merge_id = gtk_ui_manager_new_merge_id (eekboard->ui_manager);
    data.action_group = eekboard->models_action_group;

    xkl_config_registry_foreach_model (eekboard->registry,
                                       model_callback,
                                       &data);
    return data.merge_id;
}

static void
on_models_menu (GtkAction *action, GtkWidget *window)
{
    Eekboard *eekboard = g_object_get_data (G_OBJECT(window), "eekboard");
    if (eekboard->models_merge_id == 0)
        eekboard->models_merge_id = create_models_menu (eekboard);
}

static void
collect_variant (XklConfigRegistry *registry,
                 const XklConfigItem *variant,
                 gpointer user_data)
{
    GSList **r_variants = user_data;
    XklConfigItem *item;

    item = g_slice_dup (XklConfigItem, variant);
    *r_variants = g_slist_prepend (*r_variants, item);
}

static void
layout_callback (XklConfigRegistry *registry,
                 const XklConfigItem *item,
                 gpointer user_data)
{
    CreateMenuCallbackData *data = user_data;
    GtkAction *action;
    GSList *variants = NULL;
    char layout_action_name[128], variant_action_name[128];
    SetConfigCallbackData *config;

    g_snprintf (layout_action_name, sizeof (layout_action_name),
                "SetLayout %s", item->name);
    action = gtk_action_new (layout_action_name, item->description, NULL, NULL);
    gtk_action_group_add_action (data->action_group, action);

    xkl_config_registry_foreach_layout_variant (registry,
                                                item->name,
                                                collect_variant,
                                                &variants);

    if (!variants) {
        config = g_slice_new (SetConfigCallbackData);
        config->eekboard = data->eekboard;
        config->config = xkl_config_rec_new ();
        config->config->layouts = g_new0 (char *, 2);
        config->config->layouts[0] = g_strdup (item->name);
        config->config->layouts[1] = NULL;
        /* Reset the existing variant setting. */
        config->config->variants = g_new0 (char *, 1);
        config->config->variants[0] = NULL;
        g_signal_connect (action, "activate", G_CALLBACK (on_config_activate),
                          config);
        g_object_unref (action);
        gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                               LAYOUTS_UI_PATH,
                               layout_action_name, layout_action_name,
                               GTK_UI_MANAGER_MENUITEM, FALSE);
    } else {
        char layout_path[128];
        GSList *head;

        g_object_unref (action);
        gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                               LAYOUTS_UI_PATH,
                               layout_action_name, layout_action_name,
                               GTK_UI_MANAGER_MENU, FALSE);
        g_snprintf (layout_path, sizeof (layout_path),
                    LAYOUTS_UI_PATH "/%s", layout_action_name);

        for (head = variants; head; head = head->next) {
            XklConfigItem *_item = head->data;

            g_snprintf (variant_action_name, sizeof (variant_action_name),
                        "SetLayoutVariant %s %s", item->name, _item->name);
            action = gtk_action_new (variant_action_name,
                                     _item->description,
                                     NULL,
                                     NULL);

            config = g_slice_new (SetConfigCallbackData);
            config->eekboard = data->eekboard;
            config->config = xkl_config_rec_new ();
            config->config->layouts = g_new0 (char *, 2);
            config->config->layouts[0] = g_strdup (item->name);
            config->config->layouts[1] = NULL;
            config->config->variants = g_new0 (char *, 2);
            config->config->variants[0] = g_strdup (_item->name);
            config->config->variants[1] = NULL;
            g_signal_connect (action, "activate", G_CALLBACK (on_config_activate),
                              config);

            gtk_action_group_add_action (data->action_group, action);
            g_object_unref (action);

            gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                                   layout_path,
                                   variant_action_name, variant_action_name,
                                   GTK_UI_MANAGER_MENUITEM, FALSE);
            g_slice_free (XklConfigItem, _item);
        }
        g_slist_free (variants);
    }
}

static guint
create_layouts_menu (Eekboard *eekboard)
{
    CreateMenuCallbackData data;

    data.eekboard = eekboard;
    data.merge_id = gtk_ui_manager_new_merge_id (eekboard->ui_manager);
    data.action_group = eekboard->layouts_action_group;

    xkl_config_registry_foreach_layout (eekboard->registry,
                                        layout_callback,
                                        &data);
    return data.merge_id;
}

static void
on_layouts_menu (GtkAction *action, GtkWidget *window)
{
    Eekboard *eekboard = g_object_get_data (G_OBJECT(window), "eekboard");
    if (eekboard->layouts_merge_id == 0)
        eekboard->layouts_merge_id = create_layouts_menu (eekboard);
}

static void
collect_option (XklConfigRegistry *registry,
                 const XklConfigItem *option,
                 gpointer user_data)
{
    GSList **r_options = user_data;
    XklConfigItem *item;

    item = g_slice_dup (XklConfigItem, option);
    *r_options = g_slist_prepend (*r_options, item);
}

static void
option_group_callback (XklConfigRegistry   *registry,
                       const XklConfigItem *item,
                       gpointer             user_data)
{
    CreateMenuCallbackData *data = user_data;
    GtkAction *action;
    GSList *options = NULL, *head;
    gchar option_group_action_name[128], option_action_name[128];
    gchar option_group_path[128];
    SetConfigCallbackData *config;

    g_snprintf (option_group_action_name, sizeof (option_group_action_name),
                "OptionGroup %s", item->name);
    action = gtk_action_new (option_group_action_name, item->description,
                             NULL, NULL);
    gtk_action_group_add_action (data->action_group, action);
    g_object_unref (action);

    xkl_config_registry_foreach_option (registry,
                                        item->name,
                                        collect_option,
                                        &options);
    g_return_if_fail (options);

    gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                           OPTIONS_UI_PATH,
                           option_group_action_name, option_group_action_name,
                           GTK_UI_MANAGER_MENU, FALSE);
    g_snprintf (option_group_path, sizeof (option_group_path),
                OPTIONS_UI_PATH "/%s", option_group_action_name);

    for (head = options; head; head = head->next) {
        XklConfigItem *_item = head->data;
        GtkToggleAction *toggle;

        g_snprintf (option_action_name, sizeof (option_action_name),
                    "SetOption %s", _item->name);
        toggle = gtk_toggle_action_new (option_action_name,
                                        _item->description,
                                        NULL,
                                        NULL);

        config = g_slice_new (SetConfigCallbackData);
        config->eekboard = data->eekboard;
        config->config = xkl_config_rec_new ();
        config->config->options = g_new0 (char *, 2);
        config->config->options[0] = g_strdup (_item->name);
        config->config->options[1] = NULL;
        g_signal_connect (toggle, "toggled", G_CALLBACK (on_option_toggled),
                          config);

        gtk_action_group_add_action (data->action_group, GTK_ACTION(toggle));
        g_object_unref (toggle);

        gtk_ui_manager_add_ui (data->eekboard->ui_manager, data->merge_id,
                               option_group_path,
                               option_action_name, option_action_name,
                               GTK_UI_MANAGER_MENUITEM, FALSE);
        g_slice_free (XklConfigItem, _item);
    }
    g_slist_free (options);
}

static guint
create_options_menu (Eekboard *eekboard)
{
    CreateMenuCallbackData data;

    data.eekboard = eekboard;
    data.merge_id = gtk_ui_manager_new_merge_id (eekboard->ui_manager);
    data.action_group = eekboard->options_action_group;

    xkl_config_registry_foreach_option_group (eekboard->registry,
                                              option_group_callback,
                                              &data);
    return data.merge_id;
}

static void
on_options_menu (GtkAction *action, GtkWidget *window)
{
    Eekboard *eekboard = g_object_get_data (G_OBJECT(window), "eekboard");
    if (eekboard->options_merge_id == 0)
        eekboard->options_merge_id = create_options_menu (eekboard);
}

static void
create_menus (Eekboard      *eekboard,
              GtkWidget     *window)
{
    GtkActionGroup *action_group;

    action_group = gtk_action_group_new ("MenuActions");
    gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

    gtk_action_group_add_actions (action_group, action_entry,
                                  G_N_ELEMENTS (action_entry), window);
#if 0
    gtk_action_group_add_toggle_actions (action_group, toggle_action_entry,
                                         G_N_ELEMENTS (toggle_action_entry),
                                         window);
#endif

    gtk_ui_manager_insert_action_group (eekboard->ui_manager, action_group, 0);
    gtk_ui_manager_add_ui_from_string (eekboard->ui_manager, ui_description, -1, NULL);

    eekboard->countries_action_group = gtk_action_group_new ("Countries");
    gtk_ui_manager_insert_action_group (eekboard->ui_manager,
                                        eekboard->countries_action_group,
                                        -1);

    eekboard->languages_action_group = gtk_action_group_new ("Languages");
    gtk_ui_manager_insert_action_group (eekboard->ui_manager,
                                        eekboard->languages_action_group,
                                        -1);
    
    eekboard->models_action_group = gtk_action_group_new ("Models");
    gtk_ui_manager_insert_action_group (eekboard->ui_manager,
                                        eekboard->models_action_group,
                                        -1);

    eekboard->layouts_action_group = gtk_action_group_new ("Layouts");
    gtk_ui_manager_insert_action_group (eekboard->ui_manager,
                                        eekboard->layouts_action_group,
                                        -1);

    eekboard->options_action_group = gtk_action_group_new ("Options");
    gtk_ui_manager_insert_action_group (eekboard->ui_manager,
                                        eekboard->options_action_group,
                                        -1);
}

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
    clutter_container_add_actor (CLUTTER_CONTAINER(stage), actor);
    eek_element_get_bounds (EEK_ELEMENT(eekboard->keyboard), &bounds);
    clutter_actor_set_size (stage, bounds.width, bounds.height);
    eekboard->width = bounds.width;
    eekboard->height = bounds.height;
    return eekboard->widget;
}

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

    eekboard->ui_manager = gtk_ui_manager_new ();
    eekboard->engine = xkl_engine_get_instance (eekboard->display);
    eekboard->registry = xkl_config_registry_get_instance (eekboard->engine);
    xkl_config_registry_load (eekboard->registry, FALSE);

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

#ifdef ENABLE_NLS
    bindtextdomain (GETTEXT_PACKAGE, EEKBOARD_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

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
    
    create_menus (eekboard, window);
    menubar = gtk_ui_manager_get_widget (eekboard->ui_manager, "/MainMenu");
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER(vbox), widget);
    gtk_container_add (GTK_CONTAINER(window), vbox);
  
    gtk_widget_set_size_request (widget, eekboard->width, eekboard->height);
    gtk_widget_show_all (window);
    gtk_widget_set_size_request (widget, -1, -1);

    gtk_main ();

    return 0;
}
