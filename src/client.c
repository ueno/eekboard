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

#include "eek/eek.h"
#include "eek/eek-xkl.h"
#include "eekboard/eekboard-client.h"
#include "eekboard/eekboard-xklutil.h"
#include "client.h"

#include <string.h>

#define CSW 640
#define CSH 480

#define IBUS_INTERFACE_PANEL    "org.freedesktop.IBus.Panel"

enum {
    PROP_0,
    PROP_CONNECTION,
    PROP_EEKBOARD,
    PROP_CONTEXT,
    PROP_KEYBOARDS,
    PROP_LAST
};

typedef struct _ClientClass ClientClass;

struct _Client {
    GObject parent;

    EekboardClient *eekboard;
    EekboardContext *context;

    GSList *keyboards;
    GSList *keyboards_head;

    XklEngine *xkl_engine;
    XklConfigRegistry *xkl_config_registry;

    gulong xkl_config_changed_handler;
    gulong xkl_state_changed_handler;

    gulong key_activated_handler;

    gboolean follows_focus;
    guint hide_keyboard_timeout_id;

    GDBusConnection *ibus_connection;
    guint ibus_focus_message_filter;

#ifdef HAVE_ATSPI
    AtspiAccessible *acc;
    AtspiDeviceListener *keystroke_listener;
#endif  /* HAVE_ATSPI */

#ifdef HAVE_XTEST
    guint modifier_keycodes[8]; 
    XkbDescRec *xkb;
#endif  /* HAVE_XTEST */

    GSettings *settings;
};

struct _ClientClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (Client, client, G_TYPE_OBJECT);

#if ENABLE_FOCUS_LISTENER
#define IS_KEYBOARD_VISIBLE(client) (!client->follows_focus)
#else  /* ENABLE_FOCUS_LISTENER */
#define IS_KEYBOARD_VISIBLE(client) TRUE
#endif  /* !ENABLE_FOCUS_LISTENER */

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
static gboolean        set_keyboards        (Client                 *client,
                                             const gchar * const    *keyboard);
static gboolean        set_keyboards_from_xkl
                                            (Client                 *client);
#ifdef HAVE_XTEST
static void            update_modifier_keycodes
                                            (Client                 *client);
#endif  /* HAVE_XTEST */

static void
client_set_property (GObject      *object,
                     guint         prop_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
    Client *client = CLIENT(object);
    GDBusConnection *connection;

    switch (prop_id) {
    case PROP_CONNECTION:
        connection = g_value_get_object (value);

        client->eekboard = eekboard_client_new (connection, NULL);
        if (client->eekboard != NULL) {
            client->context =
                eekboard_client_create_context (client->eekboard,
                                                "eekboard",
                                                NULL);
            if (client->context == NULL) {
                g_object_unref (client->eekboard);
                client->eekboard = NULL;
            } else {
                eekboard_client_push_context (client->eekboard,
                                              client->context,
                                              NULL);
                g_settings_bind (client->settings, "keyboards",
                                 client, "keyboards",
                                 G_SETTINGS_BIND_GET);
            }
        }
        break;
    case PROP_KEYBOARDS:
        client_set_keyboards (client, g_value_get_boxed (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
client_get_property (GObject    *object,
                     guint       prop_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
    Client *client = CLIENT(object);

    switch (prop_id) {
    case PROP_EEKBOARD:
        g_value_set_object (value, client->eekboard);
        break;
    case PROP_CONTEXT:
        g_value_set_object (value, client->context);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
client_dispose (GObject *object)
{
    Client *client = CLIENT(object);

    client_disable_xkl (client);

#ifdef HAVE_ATSPI
    client_disable_atspi_focus (client);
    client_disable_atspi_keystroke (client);
#endif  /* HAVE_ATSPI */

    client_disable_ibus_focus (client);
    if (client->ibus_connection) {
        g_object_unref (client->ibus_connection);
        client->ibus_connection = NULL;
    }

#ifdef HAVE_XTEST
    client_disable_xtest (client);
#endif  /* HAVE_XTEST */

    if (client->context) {
        g_object_unref (client->context);
        client->context = NULL;
    }

    if (client->eekboard) {
        g_object_unref (client->eekboard);
        client->eekboard = NULL;
    }

    if (client->settings) {
        g_object_unref (client->settings);
        client->settings = NULL;
    }

    G_OBJECT_CLASS (client_parent_class)->dispose (object);
}

static void
client_finalize (GObject *object)
{
    Client *client = CLIENT(object);

    g_slist_free (client->keyboards);
    G_OBJECT_CLASS (client_parent_class)->finalize (object);
}

static void
client_class_init (ClientClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->set_property = client_set_property;
    gobject_class->get_property = client_get_property;
    gobject_class->dispose = client_dispose;
    gobject_class->finalize = client_finalize;

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
                                 EEKBOARD_TYPE_CLIENT,
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

    pspec = g_param_spec_boxed ("keyboards",
                                "Keyboards",
                                "Keyboards",
                                G_TYPE_STRV,
                                G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_KEYBOARDS,
                                     pspec);
}

static void
client_init (Client *client)
{
    client->settings = g_settings_new ("org.fedorahosted.eekboard");
}

gboolean
client_set_keyboards (Client              *client,
                      const gchar * const *keyboards)
{
    gboolean retval;
    retval = set_keyboards (client, keyboards);
    if (retval && IS_KEYBOARD_VISIBLE (client))
        eekboard_client_show_keyboard (client->eekboard, NULL);
    return retval;
}

gboolean
client_enable_xkl (Client *client)
{
    GdkDisplay *display = gdk_display_get_default ();
    gboolean retval;

    g_assert (display);

    if (!client->xkl_engine) {
        client->xkl_engine =
            xkl_engine_get_instance (GDK_DISPLAY_XDISPLAY (display));
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

    retval = set_keyboards_from_xkl (client);
    if (IS_KEYBOARD_VISIBLE (client))
        eekboard_client_show_keyboard (client->eekboard, NULL);

    return retval;
}

void
client_disable_xkl (Client *client)
{
    if (client->xkl_engine) {
        xkl_engine_stop_listen (client->xkl_engine, XKLL_TRACK_KEYBOARD_STATE);

        if (g_signal_handler_is_connected (client->xkl_engine,
                                           client->xkl_config_changed_handler))
            g_signal_handler_disconnect (client->xkl_engine,
                                         client->xkl_config_changed_handler);
        if (g_signal_handler_is_connected (client->xkl_engine,
                                           client->xkl_state_changed_handler))
            g_signal_handler_disconnect (client->xkl_engine,
                                         client->xkl_state_changed_handler);
        client->xkl_engine = NULL;
    }
}

#ifdef HAVE_ATSPI
gboolean
client_enable_atspi_focus (Client *client)
{
    GError *error;

    error = NULL;
    if (!atspi_event_listener_register_from_callback
        ((AtspiEventListenerCB)focus_listener_cb,
         client,
         NULL,
         "object:state-changed:focused",
         &error)) {
        g_warning ("can't register object:state-changed:focused handler: %s",
                   error->message);
        g_error_free (error);
        return FALSE;
    }

    error = NULL;
    if (!atspi_event_listener_register_from_callback
        ((AtspiEventListenerCB)focus_listener_cb,
         client,
         NULL,
         "focus:",
         &error)) {
        g_warning ("can't register focus: handler: %s",
                   error->message);
        g_error_free (error);
        return FALSE;
    }

    client->follows_focus = TRUE;
    return TRUE;
}

void
client_disable_atspi_focus (Client *client)
{
    GError *error;

    client->follows_focus = FALSE;

    error = NULL;
    if (!atspi_event_listener_deregister_from_callback
        ((AtspiEventListenerCB)focus_listener_cb,
         client,
         "object:state-changed:focused",
         &error)) {
        g_warning ("can't deregister object:state-changed:focused handler: %s",
                   error->message);
        g_error_free (error);
    }

    error = NULL;
    if (!atspi_event_listener_deregister_from_callback
        ((AtspiEventListenerCB)focus_listener_cb,
         client,
         "focus:",
         &error)) {
        g_warning ("can't deregister focus: handler: %s",
                   error->message);
        g_error_free (error);
    }
}

gboolean
client_enable_atspi_keystroke (Client *client)
{
    GError *error;

    client->keystroke_listener =
        atspi_device_listener_new ((AtspiDeviceListenerCB)keystroke_listener_cb,
                                   client,
                                   NULL);

    error = NULL;
    if (!atspi_register_keystroke_listener
        (client->keystroke_listener,
         NULL,
         0,
         ATSPI_KEY_PRESSED,
         ATSPI_KEYLISTENER_NOSYNC,
         &error)) {
        g_warning ("can't register keystroke listener for key press: %s",
                   error->message);
        g_error_free (error);
        return FALSE;
    }

    error = NULL;
    if (!atspi_register_keystroke_listener
        (client->keystroke_listener,
         NULL,
         0,
         ATSPI_KEY_RELEASED,
         ATSPI_KEYLISTENER_NOSYNC,
         &error)) {
        g_warning ("can't register keystroke listener for key release: %s",
                   error->message);
        g_error_free (error);
        return FALSE;
    }
    return TRUE;
}

void
client_disable_atspi_keystroke (Client *client)
{
    if (client->keystroke_listener) {
        GError *error;

        error = NULL;
        if (!atspi_deregister_keystroke_listener
            (client->keystroke_listener,
             NULL,
             0,
             ATSPI_KEY_PRESSED,
             &error)) {
            g_warning ("can't deregister keystroke listener for key press: %s",
                       error->message);
            g_error_free (error);
        }

        error = NULL;
        if (!atspi_deregister_keystroke_listener (client->keystroke_listener,
                                                  NULL,
                                                  0,
                                                  ATSPI_KEY_RELEASED,
                                                  &error)) {
            g_warning ("can't deregister keystroke listener for key release: %s",
                       error->message);
            g_error_free (error);
        }

        g_object_unref (client->keystroke_listener);
        client->keystroke_listener = NULL;
    }
}

static void
focus_listener_cb (const AtspiEvent *event,
                   void             *user_data)
{
    Client *client = user_data;
    AtspiAccessible *accessible = event->source;
    AtspiStateSet *state_set = atspi_accessible_get_state_set (accessible);
    AtspiRole role;
    GError *error;

    error = NULL;
    role = atspi_accessible_get_role (accessible, &error);
    if (role == ATSPI_ROLE_INVALID) {
        g_warning ("can't get accessible role: %s",
                   error->message);
        g_error_free (error);
        return;
    }

    if (atspi_state_set_contains (state_set, ATSPI_STATE_EDITABLE) ||
        role == ATSPI_ROLE_TERMINAL) {
        switch (role) {
        case ATSPI_ROLE_TEXT:
        case ATSPI_ROLE_PARAGRAPH:
        case ATSPI_ROLE_PASSWORD_TEXT:
        case ATSPI_ROLE_TERMINAL:
            if (strncmp (event->type, "focus", 5) == 0 || event->detail1 == 1) {
                client->acc = accessible;
                eekboard_client_show_keyboard (client->eekboard, NULL);
            } else if (g_settings_get_boolean (client->settings, "auto-hide") &&
                       event->detail1 == 0 && accessible == client->acc) {
                client->acc = NULL;
                eekboard_client_hide_keyboard (client->eekboard, NULL);
            }
            break;
        case ATSPI_ROLE_ENTRY:
            if (strncmp (event->type, "focus", 5) == 0 || event->detail1 == 1) {
                client->acc = accessible;
                eekboard_client_show_keyboard (client->eekboard, NULL);
            } else if (g_settings_get_boolean (client->settings, "auto-hide") &&
                       event->detail1 == 0) {
                client->acc = NULL;
                eekboard_client_hide_keyboard (client->eekboard, NULL);
            }
            break;
            
        default:
            ;
        }
    } else {
        eekboard_client_hide_keyboard (client->eekboard, NULL);
    }
}

static gboolean
keystroke_listener_cb (const AtspiDeviceEvent *stroke,
                       void                   *user_data)
{
    Client *client = user_data;

    switch (stroke->type) {
    case ATSPI_KEY_PRESSED:
        eekboard_context_press_keycode (client->context, stroke->hw_code, NULL);
        break;
    case ATSPI_KEY_RELEASED:
        eekboard_context_release_keycode (client->context, stroke->hw_code, NULL);
        break;
    default:
        g_return_val_if_reached (FALSE);
    }
    return TRUE;
}
#endif  /* HAVE_ATSPI */

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
    if (!g_dbus_connection_send_message (connection,
                                         message,
                                         G_DBUS_SEND_MESSAGE_FLAGS_NONE,
                                         NULL,
                                         &error)) {
        g_warning ("can't register match rule %s: %s",
                   match_rule, error->message);
        g_error_free (error);
    }
    g_object_unref (message);
}

static gboolean
on_hide_keyboard_timeout (Client *client)
{
    eekboard_client_hide_keyboard (client->eekboard, NULL);
    client->hide_keyboard_timeout_id = 0;
    return FALSE;
}

static GDBusMessage *
focus_message_filter (GDBusConnection *connection,
                      GDBusMessage    *message,
                      gboolean         incoming,
                      gpointer         user_data)
{
    Client *client = user_data;

    if (incoming &&
        g_strcmp0 (g_dbus_message_get_interface (message),
                   IBUS_INTERFACE_PANEL) == 0) {
        const gchar *member = g_dbus_message_get_member (message);

        if (g_strcmp0 (member, "FocusIn") == 0) {
            if (client->hide_keyboard_timeout_id > 0) {
                g_source_remove (client->hide_keyboard_timeout_id);
                client->hide_keyboard_timeout_id = 0;
            }
            eekboard_client_show_keyboard (client->eekboard, NULL);
        } else if (g_settings_get_boolean (client->settings, "auto-hide") &&
                   g_strcmp0 (member, "FocusOut") == 0) {
            guint delay;
            g_settings_get (client->settings, "auto-hide-delay", "u", &delay);
            client->hide_keyboard_timeout_id =
                g_timeout_add (delay,
                               (GSourceFunc)on_hide_keyboard_timeout,
                               client);
        }
    }

    return message;
}

static void
_ibus_connect_focus_handlers (GDBusConnection *connection, gpointer user_data)
{
    Client *client = user_data;

    add_match_rule (connection,
                    "type='method_call',"
                    "interface='" IBUS_INTERFACE_PANEL "',"
                    "member='FocusIn'");
    add_match_rule (connection,
                    "type='method_call',"
                    "interface='" IBUS_INTERFACE_PANEL "',"
                    "member='FocusOut'");
    client->ibus_focus_message_filter =
        g_dbus_connection_add_filter (connection,
                                      focus_message_filter,
                                      client,
                                      NULL);
}

gboolean
client_enable_ibus_focus (Client *client)
{
    if (client->ibus_connection == NULL) {
        const gchar *ibus_address;
        GError *error;

        ibus_address = g_getenv ("IBUS_ADDRESS");
        if (ibus_address == NULL) {
            g_warning ("Can't get IBus address; set IBUS_ADDRESS");
            return FALSE;
        }

        error = NULL;
        client->ibus_connection =
            g_dbus_connection_new_for_address_sync (ibus_address,
                                                    G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION,
                                                    NULL,
                                                    NULL,
                                                    &error);
        if (client->ibus_connection == NULL) {
            g_warning ("Can't open connection to IBus: %s", error->message);
            g_error_free (error);
            return FALSE;
        }
    }
    _ibus_connect_focus_handlers (client->ibus_connection, client);

    client->follows_focus = TRUE;
    return TRUE;
}

void
client_disable_ibus_focus (Client *client)
{
    client->follows_focus = FALSE;

    if (client->ibus_connection) {
        if (client->ibus_focus_message_filter != 0) {
            g_dbus_connection_remove_filter (client->ibus_connection,
                                             client->ibus_focus_message_filter);
        }
        g_object_unref (client->ibus_connection);
        client->ibus_connection = NULL;
    }
}

Client *
client_new (GDBusConnection *connection)
{
    Client *client = g_object_new (TYPE_CLIENT,
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
    Client *client = user_data;
    XEvent *xevent = (XEvent *)xev;

    xkl_engine_filter_events (client->xkl_engine, xevent);
    return GDK_FILTER_CONTINUE;
}

static void
on_xkl_config_changed (XklEngine *xklengine,
                       gpointer   user_data)
{
    Client *client = user_data;
    gboolean retval;

    retval = set_keyboards_from_xkl (client);
    g_return_if_fail (retval);

#ifdef HAVE_XTEST
    update_modifier_keycodes (client);
#endif  /* HAVE_XTEST */
}

static gboolean
set_keyboards (Client              *client,
               const gchar * const *keyboards)
{
    guint keyboard_id;
    const gchar * const *p;
    GSList *head;

    g_return_val_if_fail (keyboards != NULL, FALSE);
    g_return_val_if_fail (client->context, FALSE);

    if (client->keyboards) {
        for (head = client->keyboards; head; head = head->next) {
            eekboard_context_remove_keyboard (client->context,
                                              GPOINTER_TO_UINT(head->data),
                                              NULL);
        }
        g_slist_free (client->keyboards);
        client->keyboards = NULL;
    }

    for (p = keyboards; *p != NULL; p++) {
        keyboard_id = eekboard_context_add_keyboard (client->context, *p, NULL);
        if (keyboard_id == 0) {
            g_warning ("can't add keyboard %s", *p);
            continue;
        }
        client->keyboards = g_slist_prepend (client->keyboards,
                                             GUINT_TO_POINTER(keyboard_id));
    }

    client->keyboards = g_slist_reverse (client->keyboards);
    client->keyboards_head = client->keyboards;

    /* select the first keyboard */
    eekboard_context_set_keyboard (client->context,
                                   GPOINTER_TO_UINT(client->keyboards_head->data),
                                   NULL);
    return TRUE;
}

static gboolean
set_keyboards_from_xkl (Client *client)
{
    XklConfigRec *rec;
    gchar *layout, *keyboard;
    guint keyboard_id;

    rec = xkl_config_rec_new ();
    xkl_config_rec_get_from_server (rec, client->xkl_engine);
    layout = eekboard_xkl_config_rec_to_string (rec);
    g_object_unref (rec);

    keyboard = g_strdup_printf ("xkb:%s", layout);
    g_free (layout);

    keyboard_id = eekboard_context_add_keyboard (client->context,
                                                 keyboard,
                                                 NULL);
    g_free (keyboard);
    if (keyboard_id == 0)
        return FALSE;
    eekboard_context_set_keyboard (client->context, keyboard_id, NULL);

    return TRUE;
}

static void
on_xkl_state_changed (XklEngine           *xklengine,
                      XklEngineStateChange type,
                      gint                 value,
                      gboolean             restore,
                      gpointer             user_data)
{
    Client *client = user_data;

    if (type == GROUP_CHANGED)
        eekboard_context_set_group (client->context, value, NULL);
}

#ifdef HAVE_XTEST
/* The following functions for keyboard mapping change are direct
   translation of the code in Caribou (in libcaribou/xadapter.vala):

   - get_replaced_keycode (Caribou: get_reserved_keycode)
   - replace_keycode
   - get_keycode_from_gdk_keymap (Caribou: best_keycode_keyval_match)
*/
static guint
get_replaced_keycode (Client *client)
{
    guint keycode;

    for (keycode = client->xkb->max_key_code;
         keycode >= client->xkb->min_key_code;
         --keycode) {
        guint offset = client->xkb->map->key_sym_map[keycode].offset;
        if (client->xkb->map->key_sym_map[keycode].kt_index[0] == XkbOneLevelIndex &&
            client->xkb->map->syms[offset] != NoSymbol) {
            return keycode;
        }
    }

    return 0;
}

/* Replace keysym assigned to KEYCODE to KEYSYM.  Both args are used
   as in-out.  If KEYCODE points to 0, this function picks a keycode
   from the current map and replace the associated keysym to KEYSYM.
   In that case, the replaced keycode is stored in KEYCODE and the old
   keysym is stored in KEYSYM.  If otherwise (KEYCODE points to
   non-zero keycode), it simply changes the current map with the
   specified KEYCODE and KEYSYM. */
static gboolean
replace_keycode (Client *client,
                 guint   keycode,
                 guint  *keysym)
{
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    guint old_keysym;
    int keysyms_per_keycode;
    KeySym *syms;

    g_return_val_if_fail (client->xkb->min_key_code <= keycode &&
                          keycode <= client->xkb->max_key_code,
                          FALSE);
    g_return_val_if_fail (keysym != NULL, FALSE);

    syms = XGetKeyboardMapping (xdisplay, keycode, 1, &keysyms_per_keycode);
    old_keysym = syms[0];
    syms[0] = *keysym;
    XChangeKeyboardMapping (xdisplay, keycode, 1, syms, 1);
    XSync (xdisplay, False);
    XFree (syms);
    *keysym = old_keysym;

    return TRUE;
}

static gboolean
get_keycode_from_gdk_keymap (Client *client,
                             guint           keysym,
                             guint          *keycode,
                             guint          *modifiers)
{
    GdkKeymap *keymap = gdk_keymap_get_default ();
    GdkKeymapKey *keys, *best_match = NULL;
    gint n_keys, i;

    if (!gdk_keymap_get_entries_for_keyval (keymap, keysym, &keys, &n_keys))
        return FALSE;

    for (i = 0; i < n_keys; i++)
        if (keys[i].group == eekboard_context_get_group (client->context, NULL))
            best_match = &keys[i];

    if (!best_match) {
        g_free (keys);
        return FALSE;
    }

    *keycode = best_match->keycode;
    *modifiers = best_match->level == 1 ? EEK_SHIFT_MASK : 0;

    g_free (keys);
    return TRUE;
}

static void
send_fake_modifier_key_event (Client         *client,
                              EekModifierType modifiers,
                              gboolean        is_pressed)
{
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    gint i;

    for (i = 0; i < G_N_ELEMENTS(client->modifier_keycodes); i++) {
        if (modifiers & (1 << i)) {
            guint keycode = client->modifier_keycodes[i];

            g_return_if_fail (keycode > 0);

            XTestFakeKeyEvent (xdisplay,
                               keycode,
                               is_pressed,
                               CurrentTime);
            XSync (xdisplay, False);
        }
    }
}

static void
send_fake_key_event (Client  *client,
                     guint    xkeysym,
                     guint    keyboard_modifiers)
{
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    EekModifierType modifiers;
    guint keycode;
    guint old_keysym = xkeysym;

    g_return_if_fail (xkeysym > 0);

    modifiers = 0;
    if (!get_keycode_from_gdk_keymap (client, xkeysym, &keycode, &modifiers)) {
        keycode = get_replaced_keycode (client);
        if (keycode == 0) {
            g_warning ("no available keycode to replace");
            return;
        }

        if (!replace_keycode (client, keycode, &old_keysym)) {
            g_warning ("failed to lookup X keysym %X", xkeysym);
            return;
        }
    }

    /* Clear level shift modifiers */
    keyboard_modifiers &= ~EEK_SHIFT_MASK;
    keyboard_modifiers &= ~EEK_LOCK_MASK;
    /* FIXME: may need to remap ISO_Level3_Shift and NumLock */
#if 0
    keyboard_modifiers &= ~EEK_MOD5_MASK;
    keyboard_modifiers &= ~client->alt_gr_mask;
    keyboard_modifiers &= ~client->num_lock_mask;
#endif

    modifiers |= keyboard_modifiers;

    send_fake_modifier_key_event (client, modifiers, TRUE);
    XTestFakeKeyEvent (xdisplay, keycode, TRUE, 20);
    XSync (xdisplay, False);
    XTestFakeKeyEvent (xdisplay, keycode, FALSE, 20);
    XSync (xdisplay, False);
    send_fake_modifier_key_event (client, modifiers, FALSE);

    if (old_keysym != xkeysym)
        replace_keycode (client, keycode, &old_keysym);
}

static void
send_fake_key_events (Client    *client,
                      EekSymbol *symbol,
                      guint      keyboard_modifiers)
{
    /* Ignore modifier keys */
    if (eek_symbol_is_modifier (symbol))
        return;

    /* If symbol is a text, convert chars in it to keysym */
    if (EEK_IS_TEXT(symbol)) {
        const gchar *utf8 = eek_text_get_text (EEK_TEXT(symbol));
        glong items_written;
        gunichar *ucs4 = g_utf8_to_ucs4_fast (utf8, -1, &items_written);
        gint i;

        for (i = 0; i < items_written; i++) {
            guint xkeysym;
            EekKeysym *keysym;
            gchar *name;

            name = g_strdup_printf ("U%04X", ucs4[i]);
            xkeysym = XStringToKeysym (name);
            g_free (name);

            keysym = eek_keysym_new (xkeysym);
            send_fake_key_events (client,
                                  EEK_SYMBOL(keysym),
                                  keyboard_modifiers);
        }
        g_free (ucs4);
        return;
    }

    if (EEK_IS_KEYSYM(symbol)) {
        guint xkeysym = eek_keysym_get_xkeysym (EEK_KEYSYM(symbol));
        send_fake_key_event (client, xkeysym, keyboard_modifiers);
    }
}

static void
on_key_activated (EekboardContext *context,
                  guint            keycode,
                  EekSymbol       *symbol,
                  guint            modifiers,
                  gpointer         user_data)
{
    Client *client = user_data;

    if (g_strcmp0 (eek_symbol_get_name (symbol), "cycle-keyboard") == 0) {
        client->keyboards_head = g_slist_next (client->keyboards_head);
        if (client->keyboards_head == NULL)
            client->keyboards_head = client->keyboards;
        eekboard_context_set_keyboard (client->context,
                                       GPOINTER_TO_UINT(client->keyboards_head->data),
                                       NULL);
        return;
    }

    if (g_strcmp0 (eek_symbol_get_name (symbol), "preferences") == 0) {
        gchar *argv[2];
        GError *error;

        argv[0] = g_build_filename (LIBEXECDIR, "eekboard-setup", NULL);
        argv[1] = NULL;

        error = NULL;
        if (!g_spawn_async (NULL, argv, NULL, 0, NULL, NULL, NULL, &error)) {
            g_warning ("can't spawn %s: %s", argv[0], error->message);
            g_error_free (error);
        }
        g_free (argv[0]);
        return;
    }

    send_fake_key_events (client, symbol, modifiers);
}

static void
update_modifier_keycodes (Client *client)
{
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    XModifierKeymap *mods;
    gint i, j;

    mods = XGetModifierMapping (xdisplay);
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
    XFreeModifiermap (mods);
}

gboolean
client_enable_xtest (Client *client)
{
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    int opcode, event_base, error_base, major_version, minor_version;

    g_assert (display);

    if (!XTestQueryExtension (xdisplay,
                              &event_base, &error_base,
                              &major_version, &minor_version)) {
        g_warning ("XTest extension is not available");
        return FALSE;
    }

    if (!XkbQueryExtension (xdisplay,
                            &opcode, &event_base, &error_base,
                            &major_version, &minor_version)) {
        g_warning ("Xkb extension is not available");
        return FALSE;
    }

    if (!client->xkb)
        client->xkb = XkbGetMap (xdisplay, XkbKeySymsMask, XkbUseCoreKbd);
    g_assert (client->xkb);

    update_modifier_keycodes (client);

    client->key_activated_handler =
        g_signal_connect (client->context, "key-activated",
                          G_CALLBACK(on_key_activated), client);

    return TRUE;
}

void
client_disable_xtest (Client *client)
{
    if (client->xkb) {
        XkbFreeKeyboard (client->xkb, 0, TRUE);	/* free_all = TRUE */
        client->xkb = NULL;
    }
}
#endif  /* HAVE_XTEST */
