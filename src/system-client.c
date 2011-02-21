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

#ifdef HAVE_CSPI
#include <cspi/spi.h>
#endif  /* HAVE_CSPI */

#include <gdk/gdkx.h>

#ifdef HAVE_FAKEKEY
#include <fakekey/fakekey.h>
#endif  /* HAVE_FAKEKEY */

#include "eek/eek.h"
#include "eek/eek-xkl.h"
#include "eekboard/eekboard.h"
#include "system-client.h"

#define CSW 640
#define CSH 480

enum {
    PROP_0,
    PROP_CONNECTION,
    PROP_LAST
};

typedef struct _EekboardSystemClientClass EekboardSystemClientClass;

struct _EekboardSystemClient {
    GObject parent;

    EekboardKeyboard *keyboard;

    EekKeyboard *description;
    GdkDisplay *display;
    XklEngine *xkl_engine;
    XklConfigRegistry *xkl_config_registry;

    gulong xkl_config_changed_handler;
    gulong xkl_state_changed_handler;

    gulong key_pressed_handler;
    gulong key_released_handler;

#ifdef HAVE_CSPI
    AccessibleEventListener *focus_listener;
    AccessibleEventListener *keystroke_listener;
#endif  /* HAVE_CSPI */

#ifdef HAVE_FAKEKEY
    FakeKey *fakekey;
#endif  /* HAVE_FAKEKEY */
};

struct _EekboardSystemClientClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (EekboardSystemClient, eekboard_system_client, G_TYPE_OBJECT);

static GdkFilterReturn filter_xkl_event  (GdkXEvent                 *xev,
                                          GdkEvent                  *event,
                                          gpointer                   user_data);
static void            on_xkl_config_changed
                                         (XklEngine                 *xklengine,
                                          gpointer                   user_data);

static void            on_xkl_state_changed
                                         (XklEngine                 *xklengine,
                                          XklEngineStateChange       type,
                                          gint                       value,
                                          gboolean                   restore,
                                          gpointer                   user_data);

#ifdef HAVE_CSPI
static SPIBoolean      focus_listener_cb (const AccessibleEvent     *event,
                                          void                      *user_data);

static SPIBoolean      keystroke_listener_cb
                                         (const AccessibleKeystroke *stroke,
                                          void                      *user_data);
#endif  /* HAVE_CSPI */
static void            set_description   (EekboardSystemClient      *client);

static void
eekboard_system_client_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
    EekboardSystemClient *client = EEKBOARD_SYSTEM_CLIENT(object);
    GDBusConnection *connection;
    GError *error;

    switch (prop_id) {
    case PROP_CONNECTION:
        connection = g_value_get_object (value);
        error = NULL;
        client->keyboard = eekboard_keyboard_new ("/com/redhat/Eekboard/Keyboard",
                                             connection,
                                             NULL,
                                             &error);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eekboard_system_client_dispose (GObject *object)
{
    EekboardSystemClient *client = EEKBOARD_SYSTEM_CLIENT(object);

    eekboard_system_client_disable_xkl (client);

#ifdef HAVE_CSPI
    eekboard_system_client_disable_cspi_focus (client);
    eekboard_system_client_disable_cspi_keystroke (client);
#endif  /* HAVE_CSPI */

#ifdef HAVE_FAKEKEY
    eekboard_system_client_disable_fakekey (client);
#endif  /* HAVE_FAKEKEY */

    if (client->keyboard) {
        g_object_unref (client->keyboard);
        client->keyboard = NULL;
    }

    if (client->description) {
        g_object_unref (client->description);
        client->description = NULL;
    }

#ifdef HAVE_FAKEKEY
    if (client->fakekey) {
        client->fakekey = NULL;
    }
#endif  /* HAVE_FAKEKEY */

    if (client->display) {
        gdk_display_close (client->display);
        client->display = NULL;
    }

    G_OBJECT_CLASS (eekboard_system_client_parent_class)->dispose (object);
}

static void
eekboard_system_client_class_init (EekboardSystemClientClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->set_property = eekboard_system_client_set_property;
    gobject_class->dispose = eekboard_system_client_dispose;

    pspec = g_param_spec_object ("connection",
                                 "Connection",
                                 "Connection",
                                 G_TYPE_DBUS_CONNECTION,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_CONNECTION,
                                     pspec);
}

static void
eekboard_system_client_init (EekboardSystemClient *client)
{
    client->keyboard = NULL;
    client->display = NULL;
    client->xkl_engine = NULL;
    client->xkl_config_registry = NULL;
    client->description = NULL;
    client->key_pressed_handler = 0;
    client->key_released_handler = 0;
    client->xkl_config_changed_handler = 0;
    client->xkl_state_changed_handler = 0;
#ifdef HAVE_CSPI
    client->focus_listener = NULL;
    client->keystroke_listener = NULL;
#endif  /* HAVE_CSPI */
#ifdef HAVE_FAKEKEY
    client->fakekey = NULL;
#endif  /* HAVE_FAKEKEY */
}

gboolean
eekboard_system_client_enable_xkl (EekboardSystemClient *client)
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

    xkl_engine_start_listen (client->xkl_engine, XKLL_TRACK_KEYBOARD_STATE);

    set_description (client);

    return TRUE;
}

void
eekboard_system_client_disable_xkl (EekboardSystemClient *client)
{
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

#ifdef HAVE_CSPI
gboolean
eekboard_system_client_enable_cspi_focus (EekboardSystemClient *client)
{
    client->focus_listener = SPI_createAccessibleEventListener
        ((AccessibleEventListenerCB)focus_listener_cb,
         client);

    if (!SPI_registerGlobalEventListener (client->focus_listener,
                                          "object:state-changed:focused"))
        return FALSE;

    if (!SPI_registerGlobalEventListener (client->focus_listener,
                                          "focus:"))
        return FALSE;

    return TRUE;
}

void
eekboard_system_client_disable_cspi_focus (EekboardSystemClient *client)
{
    if (client->focus_listener) {
        SPI_deregisterGlobalEventListenerAll (client->focus_listener);
        AccessibleEventListener_unref (client->focus_listener);
        client->focus_listener = NULL;
    }
}

gboolean
eekboard_system_client_enable_cspi_keystroke (EekboardSystemClient *client)
{
    client->keystroke_listener =
        SPI_createAccessibleKeystrokeListener (keystroke_listener_cb,
                                               client);

    if (!SPI_registerAccessibleKeystrokeListener
        (client->keystroke_listener,
         SPI_KEYSET_ALL_KEYS,
         0,
         SPI_KEY_PRESSED |
         SPI_KEY_RELEASED,
         SPI_KEYLISTENER_NOSYNC))
        return FALSE;
    return TRUE;
}

void
eekboard_system_client_disable_cspi_keystroke (EekboardSystemClient *client)
{
    if (client->keystroke_listener) {
        SPI_deregisterAccessibleKeystrokeListener (client->keystroke_listener,
                                                   0);
        AccessibleKeystrokeListener_unref (client->keystroke_listener);
        client->keystroke_listener = NULL;
    }
}

static SPIBoolean
focus_listener_cb (const AccessibleEvent *event,
                   void                  *user_data)
{
    EekboardSystemClient *client = user_data;
    Accessible *accessible = event->source;
    AccessibleStateSet *state_set = Accessible_getStateSet (accessible);
    AccessibleRole role = Accessible_getRole (accessible);

    if (AccessibleStateSet_contains (state_set, SPI_STATE_EDITABLE) ||
        role == SPI_ROLE_TERMINAL) {
        switch (role) {
        case SPI_ROLE_TEXT:
        case SPI_ROLE_PARAGRAPH:
        case SPI_ROLE_PASSWORD_TEXT:
        case SPI_ROLE_TERMINAL:
        case SPI_ROLE_ENTRY:
            if (g_strcmp0 (event->type, "focus") == 0 || event->detail1 == 1)
                eekboard_keyboard_show (client->keyboard);
        default:
            ;
        }
    } else
        eekboard_keyboard_hide (client->keyboard);

    return FALSE;
}

static SPIBoolean
keystroke_listener_cb (const AccessibleKeystroke *stroke,
                       void                      *user_data)
{
    EekboardSystemClient *client = user_data;
    EekKey *key;

    /* Ignore modifiers since the keystroke listener does not called
       when a modifier key is released. */
    key = eek_keyboard_find_key_by_keycode (client->description,
                                            stroke->keycode);
    if (key) {
        EekSymbol *symbol = eek_key_get_symbol_with_fallback (key, 0, 0);
        if (symbol && eek_symbol_is_modifier (symbol))
            return FALSE;
    }

    if (stroke->type == SPI_KEY_PRESSED)
        eekboard_keyboard_press_key (client->keyboard, stroke->keycode);
    else
        eekboard_keyboard_release_key (client->keyboard, stroke->keycode);
    return TRUE;
}
#endif  /* HAVE_CSPI */

EekboardSystemClient *
eekboard_system_client_new (GDBusConnection *connection)
{
    return g_object_new (EEKBOARD_TYPE_SYSTEM_CLIENT,
                         "connection", connection,
                         NULL);
}

static GdkFilterReturn
filter_xkl_event (GdkXEvent *xev,
                  GdkEvent  *event,
                  gpointer   user_data)
{
    EekboardSystemClient *client = user_data;
    XEvent *xevent = (XEvent *)xev;

    xkl_engine_filter_events (client->xkl_engine, xevent);
    return GDK_FILTER_CONTINUE;
}

static void
on_xkl_config_changed (XklEngine *xklengine,
                       gpointer   user_data)
{
    EekboardSystemClient *client = user_data;

    set_description (client);

#ifdef HAVE_FAKEKEY
    if (client->fakekey)
        fakekey_reload_keysyms (client->fakekey);
#endif  /* HAVE_FAKEKEY */
}

static void
set_description (EekboardSystemClient *client)
{
    EekLayout *layout;
    gchar *keyboard_name;
    static gint keyboard_serial = 0;

    if (client->description)
        g_object_unref (client->description);
    layout = eek_xkl_layout_new ();
    client->description = eek_keyboard_new (layout, CSW, CSH);
    eek_keyboard_set_modifier_behavior (client->description,
                                        EEK_MODIFIER_BEHAVIOR_LATCH);

    keyboard_name = g_strdup_printf ("keyboard%d", keyboard_serial++);
    eek_element_set_name (EEK_ELEMENT(client->description), keyboard_name);

    eekboard_keyboard_set_description (client->keyboard, client->description);
}

static void
on_xkl_state_changed (XklEngine           *xklengine,
                      XklEngineStateChange type,
                      gint                 value,
                      gboolean             restore,
                      gpointer             user_data)
{
    EekboardSystemClient *client = user_data;

    if (type == GROUP_CHANGED && client->description) {
        gint group = eek_keyboard_get_group (client->description);
        if (group != value) {
            eek_keyboard_set_group (client->description, value);
            eekboard_keyboard_set_group (client->keyboard, value);
        }
    }
}

#ifdef HAVE_FAKEKEY
G_INLINE_FUNC FakeKeyModifier
get_fakekey_modifiers (EekModifierType modifiers)
{
    FakeKeyModifier retval = 0;

    if (modifiers & EEK_SHIFT_MASK)
        retval |= FAKEKEYMOD_SHIFT;
    if (modifiers & EEK_CONTROL_MASK)
        retval |= FAKEKEYMOD_CONTROL;
    if (modifiers & EEK_MOD1_MASK)
        retval |= FAKEKEYMOD_ALT;
    if (modifiers & EEK_META_MASK)
        retval |= FAKEKEYMOD_META;

    return retval;
}

static void
on_key_pressed (EekboardKeyboard *keyboard,
                guint          keycode,
                gpointer       user_data)
{
    EekboardSystemClient *client = user_data;
    EekKey *key;
    EekSymbol *symbol;
    EekModifierType modifiers;

    g_assert (client->fakekey);

    modifiers = eek_keyboard_get_modifiers (client->description);
    key = eek_keyboard_find_key_by_keycode (client->description, keycode);
    if (!key) {
        // g_debug ("Can't find key for keycode %u", keycode);
        return;
    }

    symbol = eek_key_get_symbol_with_fallback (key, 0, 0);
    if (EEK_IS_KEYSYM(symbol) && !eek_symbol_is_modifier (symbol)) {
        fakekey_send_keyevent (client->fakekey,
                               keycode,
                               True,
                               get_fakekey_modifiers (modifiers));
        fakekey_send_keyevent (client->fakekey,
                               keycode,
                               False,
                               get_fakekey_modifiers (modifiers));
    }
}

static void
on_key_released (EekboardKeyboard *keyboard,
                 guint          keycode,
                 gpointer       user_data)
{
    EekboardSystemClient *client = user_data;
    EekKey *key;

    g_assert (client->fakekey);
    fakekey_release (client->fakekey);
    key = eek_keyboard_find_key_by_keycode (client->description, keycode);
    if (!key) {
        // g_debug ("Can't find key for keycode %u", keycode);
        return;
    }
}

gboolean
eekboard_system_client_enable_fakekey (EekboardSystemClient *client)
{
    if (!client->display) {
        client->display = gdk_display_get_default ();
    }
    g_assert (client->display);

    if (!client->fakekey) {
        client->fakekey = fakekey_init (GDK_DISPLAY_XDISPLAY (client->display));
    }
    g_assert (client->fakekey);

    client->key_pressed_handler =
        g_signal_connect (client->keyboard, "key-pressed",
                          G_CALLBACK(on_key_pressed), client);
    client->key_released_handler =
        g_signal_connect (client->keyboard, "key-released",
                          G_CALLBACK(on_key_released), client);

    return TRUE;
}

void
eekboard_system_client_disable_fakekey (EekboardSystemClient *client)
{
    if (client->fakekey)
        fakekey_release (client->fakekey);

    if (g_signal_handler_is_connected (client->keyboard,
                                       client->key_pressed_handler))
        g_signal_handler_disconnect (client->keyboard,
                                     client->key_pressed_handler);
    if (g_signal_handler_is_connected (client->keyboard,
                                       client->key_released_handler))
        g_signal_handler_disconnect (client->keyboard,
                                     client->key_released_handler);
}
#endif  /* HAVE_FAKEKEY */
