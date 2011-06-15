/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
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

#include <libxklavier/xklavier.h>

#ifdef HAVE_ATSPI
#include <dbus/dbus.h>
#include <atspi/atspi.h>
#endif  /* HAVE_ATSPI */

#include <gdk/gdkx.h>

#ifdef HAVE_XTEST
#include <X11/extensions/XTest.h>
#include <X11/XKBlib.h>
#endif  /* HAVE_XTEST */

#ifdef HAVE_IBUS
#include <ibus.h>
#endif  /* HAVE_IBUS */

#include "eek/eek.h"
#include "eek/eek-xkl.h"
#include "eekboard/eekboard.h"
#include "client.h"
#include "xklutil.h"

#include <string.h>

#define CSW 640
#define CSH 480

enum {
    PROP_0,
    PROP_CONNECTION,
    PROP_EEKBOARD,
    PROP_CONTEXT,
    PROP_LAST
};

typedef struct _EekboardClientClass EekboardClientClass;

struct _EekboardClient {
    GObject parent;

    EekboardEekboard *eekboard;
    EekboardContext *context;

    EekKeyboard *keyboard;
    GdkDisplay *display;
    XklEngine *xkl_engine;
    XklConfigRegistry *xkl_config_registry;
    gboolean use_xkl_layout;
    gint group;

    gulong xkl_config_changed_handler;
    gulong xkl_state_changed_handler;

    gulong key_pressed_handler;
    gulong key_released_handler;

    gboolean follows_focus;

#ifdef HAVE_ATSPI
    AtspiAccessible *acc;
    AtspiDeviceListener *keystroke_listener;
#endif  /* HAVE_ATSPI */

#ifdef HAVE_IBUS
    IBusBus *ibus_bus;
    guint ibus_focus_message_filter;
#endif  /* HAVE_IBUS */

#ifdef HAVE_XTEST
    KeyCode modifier_keycodes[8]; 
    KeyCode reserved_keycode;
    KeySym reserved_keysym;
    XkbDescRec *xkb;
#endif  /* HAVE_XTEST */

    GSettings *settings;
};

struct _EekboardClientClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (EekboardClient, eekboard_client, G_TYPE_OBJECT);

static GdkFilterReturn filter_xkl_event     (GdkXEvent              *xev,
                                             GdkEvent               *event,
                                             gpointer                user_data);
static void            on_xkl_config_changed (XklEngine              *xklengine,
                                              gpointer                user_data);

static void            on_xkl_state_changed (XklEngine              *xklengine,
                                             XklEngineStateChange    type,
                                             gint                    value,
                                             gboolean                restore,
                                             gpointer                user_data);

#ifdef HAVE_ATSPI
static void            focus_listener_cb    (const AtspiEvent       *event,
                                             void                   *user_data);

static gboolean        keystroke_listener_cb (const AtspiDeviceEvent *stroke,
                                              void                   *user_data);
#endif  /* HAVE_ATSPI */
static gboolean        set_keyboard         (EekboardClient         *client,
                                             gboolean                show,
                                             EekLayout              *layout);
static gboolean        set_keyboard_from_xkl (EekboardClient         *client,
                                              gboolean                show,
                                              const gchar            *model,
                                              const gchar            *layouts,
                                              const gchar            *options);
#ifdef HAVE_XTEST
static void            update_modifier_keycodes
                                            (EekboardClient         *client);
static gboolean        get_keycode_for_keysym
                                            (EekboardClient         *client,
                                             guint                   keysym,
                                             guint                  *keycode,
                                             guint                  *modifiers);
static gboolean        get_keycode_for_keysym_replace
                                            (EekboardClient         *client,
                                             guint                   keysym,
                                             guint                  *keycode,
                                             guint                  *modifiers);
#endif  /* HAVE_XTEST */

static void
eekboard_client_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
    EekboardClient *client = EEKBOARD_CLIENT(object);
    GDBusConnection *connection;

    switch (prop_id) {
    case PROP_CONNECTION:
        connection = g_value_get_object (value);

        client->eekboard = eekboard_eekboard_new (connection, NULL);
        if (client->eekboard != NULL) {
            client->context =
                eekboard_eekboard_create_context (client->eekboard,
                                                  "eekboard",
                                                  NULL);
            if (client->context == NULL) {
                g_object_unref (client->eekboard);
                client->eekboard = NULL;
            } else
                eekboard_eekboard_push_context (client->eekboard,
                                                client->context,
                                                NULL);
        }
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eekboard_client_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    EekboardClient *client = EEKBOARD_CLIENT(object);

    switch (prop_id) {
    case PROP_EEKBOARD:
        g_value_set_object (value, client->eekboard);
        break;
    case PROP_CONTEXT:
        g_value_set_object (value, client->context);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eekboard_client_dispose (GObject *object)
{
    EekboardClient *client = EEKBOARD_CLIENT(object);

    eekboard_client_disable_xkl (client);

#ifdef HAVE_ATSPI
    eekboard_client_disable_atspi_focus (client);
    eekboard_client_disable_atspi_keystroke (client);
#endif  /* HAVE_ATSPI */

#ifdef HAVE_IBUS
    eekboard_client_disable_ibus_focus (client);
    if (client->ibus_bus) {
        g_object_unref (client->ibus_bus);
        client->ibus_bus = NULL;
    }
#endif  /* HAVE_IBUS */

#ifdef HAVE_XTEST
    eekboard_client_disable_xtest (client);
#endif  /* HAVE_XTEST */

    if (client->context) {
        if (client->eekboard) {
            eekboard_eekboard_pop_context (client->eekboard, NULL);
        }

        g_object_unref (client->context);
        client->context = NULL;
    }

    if (client->eekboard) {
        g_object_unref (client->eekboard);
        client->eekboard = NULL;
    }

    if (client->keyboard) {
        g_object_unref (client->keyboard);
        client->keyboard = NULL;
    }

    if (client->display) {
        gdk_display_close (client->display);
        client->display = NULL;
    }

    if (client->settings) {
        g_object_unref (client->settings);
        client->settings = NULL;
    }

    G_OBJECT_CLASS (eekboard_client_parent_class)->dispose (object);
}

static void
eekboard_client_class_init (EekboardClientClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->set_property = eekboard_client_set_property;
    gobject_class->get_property = eekboard_client_get_property;
    gobject_class->dispose = eekboard_client_dispose;

    pspec = g_param_spec_object ("connection",
                                 "Connection",
                                 "Connection",
                                 G_TYPE_DBUS_CONNECTION,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_CONNECTION,
                                     pspec);

    pspec = g_param_spec_object ("eekboard",
                                 "Eekboard",
                                 "Eekboard",
                                 EEKBOARD_TYPE_EEKBOARD,
                                 G_PARAM_READABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_EEKBOARD,
                                     pspec);

    pspec = g_param_spec_object ("context",
                                 "Context",
                                 "Context",
                                 EEKBOARD_TYPE_CONTEXT,
                                 G_PARAM_READABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_CONTEXT,
                                     pspec);
}

static void
eekboard_client_init (EekboardClient *client)
{
    client->eekboard = NULL;
    client->context = NULL;
    client->display = NULL;
    client->xkl_engine = NULL;
    client->xkl_config_registry = NULL;
    client->keyboard = NULL;
    client->key_pressed_handler = 0;
    client->key_released_handler = 0;
    client->xkl_config_changed_handler = 0;
    client->xkl_state_changed_handler = 0;
#if ENABLE_FOCUS_LISTENER
    client->follows_focus = FALSE;
#endif  /* ENABLE_FOCUS_LISTENER */
#ifdef HAVE_ATSPI
    client->keystroke_listener = NULL;
#endif  /* HAVE_ATSPI */
#ifdef HAVE_IBUS
    client->ibus_bus = NULL;
    client->ibus_focus_message_filter = 0;
#endif  /* HAVE_IBUS */
    client->settings = g_settings_new ("org.fedorahosted.eekboard");
}

gboolean
eekboard_client_load_keyboard_from_xkl (EekboardClient *client,
                                        const gchar    *model,
                                        const gchar    *layouts,
                                        const gchar    *options)
{
    client->use_xkl_layout = TRUE;

#if ENABLE_FOCUS_LISTENER
    return set_keyboard_from_xkl (client,
                                  !client->follows_focus,
                                  model,
                                  layouts,
                                  options);
#else  /* ENABLE_FOCUS_LISTENER */
    return set_keyboard_from_xkl (client,
                                  TRUE,
                                  model,
                                  layouts,
                                  options);
#endif  /* !ENABLE_FOCUS_LISTENER */
}

gboolean
eekboard_client_enable_xkl (EekboardClient *client)
{
    if (!client->display) {
        client->display = gdk_display_get_default ();
    }
    g_assert (client->display);

    if (!client->xkl_engine) {
        client->xkl_engine =
            xkl_engine_get_instance (GDK_DISPLAY_XDISPLAY (client->display));
    }
    g_assert (client->xkl_engine);

    if (!client->xkl_config_registry) {
        client->xkl_config_registry =
            xkl_config_registry_get_instance (client->xkl_engine);
        xkl_config_registry_load (client->xkl_config_registry, FALSE);
    }

    client->xkl_config_changed_handler =
        g_signal_connect (client->xkl_engine, "X-config-changed",
                          G_CALLBACK(on_xkl_config_changed), client);
    client->xkl_state_changed_handler =
        g_signal_connect (client->xkl_engine, "X-state-changed",
                          G_CALLBACK(on_xkl_state_changed), client);

    gdk_window_add_filter (NULL,
                           (GdkFilterFunc) filter_xkl_event,
                           client);
    gdk_window_add_filter (gdk_get_default_root_window (),
                           (GdkFilterFunc) filter_xkl_event,
                           client);

    client->use_xkl_layout = FALSE;

    xkl_engine_start_listen (client->xkl_engine, XKLL_TRACK_KEYBOARD_STATE);

    return TRUE;
}

void
eekboard_client_disable_xkl (EekboardClient *client)
{
    client->use_xkl_layout = FALSE;

    if (client->xkl_engine)
        xkl_engine_stop_listen (client->xkl_engine, XKLL_TRACK_KEYBOARD_STATE);
    if (g_signal_handler_is_connected (client->xkl_engine,
                                       client->xkl_config_changed_handler))
        g_signal_handler_disconnect (client->xkl_engine,
                                     client->xkl_config_changed_handler);
    if (g_signal_handler_is_connected (client->xkl_engine,
                                       client->xkl_state_changed_handler))
        g_signal_handler_disconnect (client->xkl_engine,
                                     client->xkl_state_changed_handler);
}

#ifdef HAVE_ATSPI
gboolean
eekboard_client_enable_atspi_focus (EekboardClient *client)
{
    GError *error;

    error = NULL;
    if (!atspi_event_listener_register_from_callback
        ((AtspiEventListenerCB)focus_listener_cb,
         client,
         NULL,
         "object:state-changed:focused",
         &error))
        return FALSE;

    error = NULL;
    if (!atspi_event_listener_register_from_callback
        ((AtspiEventListenerCB)focus_listener_cb,
         client,
         NULL,
         "focus:",
         &error))
        return FALSE;

    client->follows_focus = TRUE;
    return TRUE;
}

void
eekboard_client_disable_atspi_focus (EekboardClient *client)
{
    GError *error;

    client->follows_focus = FALSE;

    error = NULL;
    atspi_event_listener_deregister_from_callback
        ((AtspiEventListenerCB)focus_listener_cb,
         client,
         "object:state-changed:focused",
         &error);

    error = NULL;
    atspi_event_listener_deregister_from_callback
        ((AtspiEventListenerCB)focus_listener_cb,
         client,
         "focus:",
         &error);
}

gboolean
eekboard_client_enable_atspi_keystroke (EekboardClient *client)
{
    GError *error;

    client->keystroke_listener =
        atspi_device_listener_new ((AtspiDeviceListenerCB)keystroke_listener_cb,
                                   NULL,
                                   client);

    error = NULL;
    if (!atspi_register_keystroke_listener
        (client->keystroke_listener,
         NULL,
         0,
         ATSPI_KEY_PRESSED,
         ATSPI_KEYLISTENER_NOSYNC,
         &error))
        return FALSE;

    error = NULL;
    if (!atspi_register_keystroke_listener
        (client->keystroke_listener,
         NULL,
         0,
         ATSPI_KEY_RELEASED,
         ATSPI_KEYLISTENER_NOSYNC,
         &error))
        return FALSE;
    return TRUE;
}

void
eekboard_client_disable_atspi_keystroke (EekboardClient *client)
{
    if (client->keystroke_listener) {
        GError *error;

        error = NULL;
        atspi_deregister_keystroke_listener (client->keystroke_listener,
                                             NULL,
                                             0,
                                             ATSPI_KEY_PRESSED,
                                             &error);

        error = NULL;
        atspi_deregister_keystroke_listener (client->keystroke_listener,
                                             NULL,
                                             0,
                                             ATSPI_KEY_RELEASED,
                                             &error);

        g_object_unref (client->keystroke_listener);
        client->keystroke_listener = NULL;
    }
}

static void
focus_listener_cb (const AtspiEvent *event,
                   void             *user_data)
{
    EekboardClient *client = user_data;
    AtspiAccessible *accessible = event->source;
    AtspiStateSet *state_set = atspi_accessible_get_state_set (accessible);
    AtspiRole role;
    GError *error;

    error = NULL;
    role = atspi_accessible_get_role (accessible, &error);
    if (error)
        return;

    if (atspi_state_set_contains (state_set, ATSPI_STATE_EDITABLE) ||
        role == ATSPI_ROLE_TERMINAL) {
        switch (role) {
        case ATSPI_ROLE_TEXT:
        case ATSPI_ROLE_PARAGRAPH:
        case ATSPI_ROLE_PASSWORD_TEXT:
        case ATSPI_ROLE_TERMINAL:
            if (strncmp (event->type, "focus", 5) == 0 || event->detail1 == 1) {
                client->acc = accessible;
                eekboard_context_show_keyboard (client->context, NULL);
            } else if (g_settings_get_boolean (client->settings, "auto-hide") &&
                       event->detail1 == 0 && accessible == client->acc) {
                client->acc = NULL;
                eekboard_context_hide_keyboard (client->context, NULL);
            }
            break;
        case ATSPI_ROLE_ENTRY:
            if (strncmp (event->type, "focus", 5) == 0 || event->detail1 == 1) {
                client->acc = accessible;
                eekboard_context_show_keyboard (client->context, NULL);
            } else if (g_settings_get_boolean (client->settings, "auto-hide") &&
                       event->detail1 == 0) {
                client->acc = NULL;
                eekboard_context_hide_keyboard (client->context, NULL);
            }
            break;
            
        default:
            ;
        }
    } else {
        eekboard_context_hide_keyboard (client->context, NULL);
    }
}

static gboolean
keystroke_listener_cb (const AtspiDeviceEvent *stroke,
                       void                   *user_data)
{
    EekboardClient *client = user_data;
    EekKey *key;

    /* Ignore modifiers since the keystroke listener does not called
       when a modifier key is released. */
    key = eek_keyboard_find_key_by_keycode (client->keyboard,
                                            stroke->hw_code);
    if (key) {
        EekSymbol *symbol = eek_key_get_symbol_with_fallback (key, 0, 0);
        if (symbol && eek_symbol_is_modifier (symbol))
            return FALSE;
    }

    if (stroke->type == ATSPI_KEY_PRESSED) {
        eekboard_context_press_key (client->context, stroke->hw_code, NULL);
    } else {
        eekboard_context_release_key (client->context, stroke->hw_code, NULL);
    }

    return TRUE;
}
#endif  /* HAVE_ATSPI */

#ifdef HAVE_IBUS
static void
add_match_rule (GDBusConnection *connection,
                const gchar     *match_rule)
{
  GError *error;
  GDBusMessage *message;

  message = g_dbus_message_new_method_call ("org.freedesktop.DBus", /* name */
                                            "/org/freedesktop/DBus", /* path */
                                            "org.freedesktop.DBus", /* interface */
                                            "AddMatch");
  g_dbus_message_set_body (message, g_variant_new ("(s)", match_rule));
  error = NULL;
  g_dbus_connection_send_message (connection,
                                  message,
                                  G_DBUS_SEND_MESSAGE_FLAGS_NONE,
                                  NULL,
                                  &error);
  g_object_unref (message);
}

static GDBusMessage *
focus_message_filter (GDBusConnection *connection,
                      GDBusMessage    *message,
                      gboolean         incoming,
                      gpointer         user_data)
{
    EekboardClient *client = user_data;

    if (incoming &&
        g_strcmp0 (g_dbus_message_get_interface (message),
                   IBUS_INTERFACE_INPUT_CONTEXT) == 0) {
        const gchar *member = g_dbus_message_get_member (message);

        if (g_strcmp0 (member, "FocusIn") == 0) {
            eekboard_context_show_keyboard (client->context, NULL);
        } else if (g_settings_get_boolean (client->settings, "auto-hide") &&
                   g_strcmp0 (member, "FocusOut") == 0) {
            eekboard_context_hide_keyboard (client->context, NULL);
        }
    }

    return message;
}

gboolean
eekboard_client_enable_ibus_focus (EekboardClient *client)
{
    GDBusConnection *connection;
    GError *error;

    client->ibus_bus = ibus_bus_new ();
    connection = ibus_bus_get_connection (client->ibus_bus);
    add_match_rule (connection,
                    "type='method_call',"
                    "interface='" IBUS_INTERFACE_INPUT_CONTEXT "',"
                    "member='FocusIn'");
    add_match_rule (connection,
                    "type='method_call',"
                    "interface='" IBUS_INTERFACE_INPUT_CONTEXT "',"
                    "member='FocusOut'");
    client->ibus_focus_message_filter =
        g_dbus_connection_add_filter (connection,
                                      focus_message_filter,
                                      client,
                                      NULL);
    client->follows_focus = TRUE;
    return TRUE;
}

void
eekboard_client_disable_ibus_focus (EekboardClient *client)
{
    GDBusConnection *connection;

    client->follows_focus = FALSE;

    connection = ibus_bus_get_connection (client->ibus_bus);
    g_dbus_connection_remove_filter (connection,
                                     client->ibus_focus_message_filter);
}
#endif  /* HAVE_ATSPI */

EekboardClient *
eekboard_client_new (GDBusConnection *connection)
{
    EekboardClient *client = g_object_new (EEKBOARD_TYPE_CLIENT,
                                           "connection", connection,
                                           NULL);
    if (client->context)
        return client;
    return NULL;
}

static GdkFilterReturn
filter_xkl_event (GdkXEvent *xev,
                  GdkEvent  *event,
                  gpointer   user_data)
{
    EekboardClient *client = user_data;
    XEvent *xevent = (XEvent *)xev;

    xkl_engine_filter_events (client->xkl_engine, xevent);
    return GDK_FILTER_CONTINUE;
}

static void
on_xkl_config_changed (XklEngine *xklengine,
                       gpointer   user_data)
{
    EekboardClient *client = user_data;
    gboolean retval;

    if (client->use_xkl_layout) {
        retval = set_keyboard_from_xkl (client, FALSE, NULL, NULL, NULL);
        g_return_if_fail (retval);
    }

#ifdef HAVE_XTEST
    update_modifier_keycodes (client);
#endif  /* HAVE_XTEST */
}

static gboolean
set_keyboard (EekboardClient *client,
              gboolean        show,
              EekLayout      *layout)
{
    gchar *keyboard_name;
    static gint keyboard_serial = 0;
    guint keyboard_id;

    client->keyboard = eek_keyboard_new (layout, CSW, CSH);
    eek_keyboard_set_modifier_behavior (client->keyboard,
                                        EEK_MODIFIER_BEHAVIOR_LATCH);

    keyboard_name = g_strdup_printf ("keyboard%d", keyboard_serial++);
    eek_element_set_name (EEK_ELEMENT(client->keyboard), keyboard_name);
    g_free (keyboard_name);

    keyboard_id = eekboard_context_add_keyboard (client->context,
                                                 client->keyboard,
                                                 NULL);
    eekboard_context_set_keyboard (client->context, keyboard_id, NULL);
    if (show)
        eekboard_context_show_keyboard (client->context, NULL);
    return TRUE;
}

static gboolean
set_keyboard_from_xkl (EekboardClient *client,
                       gboolean        show,
                       const gchar    *model,
                       const gchar    *layouts,
                       const gchar    *options)
{
    EekLayout *layout;
    gboolean retval;

    if (client->keyboard)
        g_object_unref (client->keyboard);
    layout = eek_xkl_layout_new ();

    if (model) {
        if (!eek_xkl_layout_set_model (EEK_XKL_LAYOUT(layout), model)) {
            g_object_unref (layout);
            return FALSE;
        }
    }

    if (layouts) {
        XklConfigRec *rec;

        rec = eekboard_xkl_config_rec_new_from_string (layouts);
        if (!eek_xkl_layout_set_layouts (EEK_XKL_LAYOUT(layout),
                                         rec->layouts)) {
            g_object_unref (rec);
            g_object_unref (layout);
            return FALSE;
        }

        if (!eek_xkl_layout_set_variants (EEK_XKL_LAYOUT(layout),
                                          rec->variants)) {
            g_object_unref (rec);
            g_object_unref (layout);
            return FALSE;
        }            

        g_object_unref (rec);
    }

    if (options) {
        gchar **_options;

        _options = g_strsplit (options, ",", -1);
        if (!eek_xkl_layout_set_options (EEK_XKL_LAYOUT(layout), _options)) {
            g_strfreev (_options);
            g_object_unref (layout);
            return FALSE;
        }
    }

    retval = set_keyboard (client, show, layout);
    g_object_unref (layout);
    return retval;
}

static void
on_xkl_state_changed (XklEngine           *xklengine,
                      XklEngineStateChange type,
                      gint                 value,
                      gboolean             restore,
                      gpointer             user_data)
{
    EekboardClient *client = user_data;

    if (type == GROUP_CHANGED && client->keyboard) {
        if (client->use_xkl_layout) {
            gint group = eek_element_get_group (EEK_ELEMENT(client->keyboard));
            if (group != value) {
                eekboard_context_set_group (client->context, value, NULL);
            }
        }
        client->group = value;
    }
}

#ifdef HAVE_XTEST
static void
send_fake_modifier_key_event (EekboardClient *client,
                              EekModifierType modifiers,
                              gboolean        is_pressed)
{
    gint i;

    for (i = 0; i < G_N_ELEMENTS(client->modifier_keycodes); i++) {
        if (modifiers & (1 << i)) {
            guint keycode = client->modifier_keycodes[i];

            g_return_if_fail (keycode > 0);

            XTestFakeKeyEvent (GDK_DISPLAY_XDISPLAY (client->display),
                               keycode,
                               is_pressed,
                               CurrentTime);
        }
    }
}

static void
send_fake_key_event (EekboardClient *client,
                     EekKey         *key,
                     gboolean        is_pressed)
{
    EekSymbol *symbol;
    EekModifierType keyboard_modifiers, modifiers;
    guint xkeysym;
    guint keycode;

    symbol = eek_key_get_symbol_with_fallback (key, 0, 0);

    /* Ignore special keys and modifiers */
    if (!EEK_IS_KEYSYM(symbol) || eek_symbol_is_modifier (symbol))
        return;

    xkeysym = eek_keysym_get_xkeysym (EEK_KEYSYM(symbol));
    g_return_if_fail (xkeysym > 0);

    if (!get_keycode_for_keysym (client, xkeysym, &keycode, &modifiers)) {
        g_warning ("failed to lookup X keysym %X", xkeysym);
        return;
    }
    
    /* Clear level shift modifiers */
    keyboard_modifiers = eek_keyboard_get_modifiers (client->keyboard);
    keyboard_modifiers &= ~EEK_SHIFT_MASK;
    keyboard_modifiers &= ~EEK_LOCK_MASK;
    keyboard_modifiers &= ~eek_keyboard_get_alt_gr_mask (client->keyboard);

    modifiers |= keyboard_modifiers;

    send_fake_modifier_key_event (client, modifiers, is_pressed);
    XSync (GDK_DISPLAY_XDISPLAY (client->display), False);

    keycode = XKeysymToKeycode (GDK_DISPLAY_XDISPLAY (client->display),
                                xkeysym);
    g_return_if_fail (keycode > 0);

    XTestFakeKeyEvent (GDK_DISPLAY_XDISPLAY (client->display),
                       keycode,
                       is_pressed,
                       CurrentTime);
    XSync (GDK_DISPLAY_XDISPLAY (client->display), False);
}

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer     user_data)
{
    EekboardClient *client = user_data;
    send_fake_key_event (client, key, TRUE);
    send_fake_key_event (client, key, FALSE);
}

static void
on_key_released (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer     user_data)
{
}

static void
update_modifier_keycodes (EekboardClient *client)
{
    XModifierKeymap *mods;
    gint i, j;

    mods = XGetModifierMapping (GDK_DISPLAY_XDISPLAY (client->display));
    for (i = 0; i < 8; i++) {
        client->modifier_keycodes[i] = 0;
        for (j = 0; j < mods->max_keypermod; j++) {
            KeyCode keycode = mods->modifiermap[mods->max_keypermod * i + j];
            if (keycode != 0) {
                client->modifier_keycodes[i] = keycode;
                break;
            }
        }
    }
}

/* The following functions for keyboard mapping change are direct
   translation of the code in Caribou (in libcaribou/xadapter.vala):

   - get_reserved_keycode
   - reset_reserved
   - get_keycode_for_keysym_replace (Caribou: replace_keycode)
   - get_keycode_for_keysym_best (Caribou: best_keycode_keyval_match)
   - get_keycode_for_keysym (Caribou: keycode_for_keyval)
*/
static guint
get_reserved_keycode (EekboardClient *client)
{
    Display *display = GDK_DISPLAY_XDISPLAY (client->display);
    gint i;

    for (i = client->xkb->max_key_code; i >= client->xkb->min_key_code; --i) {
        if (client->xkb->map->key_sym_map[i].kt_index[0] == XkbOneLevelIndex) {
            if (XKeycodeToKeysym (display, i, 0) != 0) {
                gdk_error_trap_push ();
                XGrabKey (display, i, 0,
                          gdk_x11_get_default_root_xwindow (), TRUE,
                          GrabModeSync, GrabModeSync);
                XFlush (display);
                XUngrabKey (display, i, 0,
                            gdk_x11_get_default_root_xwindow ());
                if (gdk_error_trap_pop () == 0)
                    return i;
            }
        }
    }

    return XKeysymToKeycode (display, 0x0023); /* XK_numbersign */
}

static gboolean
reset_reserved (gpointer user_data)
{
    EekboardClient *client = user_data;
    guint keycode, modifiers;

    get_keycode_for_keysym_replace (client,
                                    client->reserved_keysym,
                                    &keycode,
                                    &modifiers);
    return FALSE;
}

static gboolean
get_keycode_for_keysym_replace (EekboardClient *client,
                                guint           keysym,
                                guint          *keycode,
                                guint          *modifiers)
{
    Display *display = GDK_DISPLAY_XDISPLAY (client->display);
    guint offset;
    XkbMapChangesRec changes;

    if (client->reserved_keycode == 0) {
        client->reserved_keycode = get_reserved_keycode (client);
        client->reserved_keysym =
            XKeycodeToKeysym (display,
                              client->reserved_keycode,
                              0);
    }
    XFlush (display);

    offset = client->xkb->map->key_sym_map[client->reserved_keycode].offset;
    client->xkb->map->syms[offset] = keysym;

    memset (&changes, 0, sizeof changes);
    changes.changed = XkbKeySymsMask;
    changes.first_key_sym = client->reserved_keycode;
    changes.num_key_syms = 1;

    XkbChangeMap (display, client->xkb, &changes);
    XFlush (display);

    *keycode = client->reserved_keycode;
    *modifiers = 0;

    if (keysym != client->reserved_keysym)
        g_timeout_add (500, reset_reserved, client);

    return TRUE;
}

static gboolean
get_keycode_for_keysym_best (EekboardClient *client,
                             guint           keysym,
                             guint          *keycode,
                             guint          *modifiers)
{
    GdkKeymap *keymap = gdk_keymap_get_default ();
    GdkKeymapKey *keys, *best_match;
    gint n_keys, i;

    if (!gdk_keymap_get_entries_for_keyval (keymap, keysym, &keys, &n_keys))
        return FALSE;

    for (i = 0; i < n_keys; i++)
        if (keys[i].group == client->group)
            best_match = &keys[i];

    *keycode = best_match->keycode;
    *modifiers = best_match->level == 1 ? EEK_SHIFT_MASK : 0;

    g_free (keys);

    return TRUE;
}

static gboolean
get_keycode_for_keysym (EekboardClient *client,
                        guint           keysym,
                        guint          *keycode,
                        guint          *modifiers)
{
    if (get_keycode_for_keysym_best (client, keysym, keycode, modifiers))
        return TRUE;

    if (get_keycode_for_keysym_replace (client, keysym, keycode, modifiers))
        return TRUE;

    return FALSE;
}

gboolean
eekboard_client_enable_xtest (EekboardClient *client)
{
    int opcode, event_base, error_base, major_version, minor_version;

    if (!client->display) {
        client->display = gdk_display_get_default ();
    }
    g_assert (client->display);

    if (!XTestQueryExtension (GDK_DISPLAY_XDISPLAY (client->display),
                              &event_base, &error_base,
                              &major_version, &minor_version)) {
        g_warning ("XTest extension is not available");
        return FALSE;
    }

    if (!XkbQueryExtension (GDK_DISPLAY_XDISPLAY (client->display),
                            &opcode, &event_base, &error_base,
                            &major_version, &minor_version)) {
        g_warning ("Xkb extension is not available");
        return FALSE;
    }

    if (!client->xkb)
        client->xkb = XkbGetMap (GDK_DISPLAY_XDISPLAY (client->display),
                                 XkbKeySymsMask,
                                 XkbUseCoreKbd);
    g_assert (client->xkb);

    client->reserved_keycode = 0;
    update_modifier_keycodes (client);

    client->key_pressed_handler =
        g_signal_connect (client->keyboard, "key-pressed",
                          G_CALLBACK(on_key_pressed), client);
    client->key_released_handler =
        g_signal_connect (client->keyboard, "key-released",
                          G_CALLBACK(on_key_released), client);

    return TRUE;
}

void
eekboard_client_disable_xtest (EekboardClient *client)
{
    if (client->xkb) {
        XkbFreeKeyboard (client->xkb, 0, TRUE);	/* free_all = TRUE */
        client->xkb = NULL;
    }

    if (g_signal_handler_is_connected (client->keyboard,
                                       client->key_pressed_handler))
        g_signal_handler_disconnect (client->keyboard,
                                     client->key_pressed_handler);
    if (g_signal_handler_is_connected (client->keyboard,
                                       client->key_released_handler))
        g_signal_handler_disconnect (client->keyboard,
                                     client->key_released_handler);
}

gboolean
eekboard_client_load_keyboard_from_file (EekboardClient *client,
                                         const gchar    *keyboard_file)
{
    GFile *file;
    GFileInputStream *input;
    GError *error;
    EekLayout *layout;
    gboolean retval;

    file = g_file_new_for_path (keyboard_file);

    error = NULL;
    input = g_file_read (file, NULL, &error);
    if (input == NULL)
        return FALSE;

    layout = eek_xml_layout_new (G_INPUT_STREAM(input));
    g_object_unref (input);
#if ENABLE_FOCUS_LISTENER
    retval = set_keyboard (client, !client->follows_focus, layout);
#else  /* ENABLE_FOCUS_LISTENER */
    retval = set_keyboard (client, TRUE, layout);
#endif  /* !ENABLE_FOCUS_LISTENER */
    g_object_unref (layout);
    return retval;
}

#endif  /* HAVE_XTEST */
