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
#include "eekboard/eekboard-client.h"
#include "eekboard/eekboard-xklutil.h"
#include "client.h"
#include "preferences-dialog.h"

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

typedef struct _ClientClass ClientClass;

struct _Client {
    GObject parent;

    EekboardClient *eekboard;
    EekboardContext *context;

    GSList *keyboards;
    XklEngine *xkl_engine;
    XklConfigRegistry *xkl_config_registry;

    gulong xkl_config_changed_handler;
    gulong xkl_state_changed_handler;

    gulong key_pressed_handler;
    gulong key_released_handler;

    gboolean follows_focus;
    guint hide_keyboard_timeout_id;

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
static gboolean        set_keyboard         (Client         *client,
                                             const gchar            *keyboard);
static gboolean        set_keyboard_from_xkl (Client         *client);
#ifdef HAVE_XTEST
static void            update_modifier_keycodes
(Client         *client);
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
            } else
                eekboard_client_push_context (client->eekboard,
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
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
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

#ifdef HAVE_IBUS
    client_disable_ibus_focus (client);
    if (client->ibus_bus) {
        g_object_unref (client->ibus_bus);
        client->ibus_bus = NULL;
    }
#endif  /* HAVE_IBUS */

#ifdef HAVE_XTEST
    client_disable_xtest (client);
#endif  /* HAVE_XTEST */

    if (client->context) {
        if (client->eekboard) {
            eekboard_client_pop_context (client->eekboard, NULL);
        }

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

    if (client->keyboards) {
        GSList *next = client->keyboards->next;
        /* client->keyboards is a ring; break it before free */
        client->keyboards->next = NULL;
        g_slist_free (next);
    }

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
}

static void
client_init (Client *client)
{
    client->settings = g_settings_new ("org.fedorahosted.eekboard");
}

gboolean
client_set_keyboard (Client *client,
                     const gchar    *keyboard)
{
    gboolean retval;
    retval = set_keyboard (client, keyboard);
    if (retval && IS_KEYBOARD_VISIBLE (client))
        eekboard_context_show_keyboard (client->context, NULL);
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

    retval = set_keyboard_from_xkl (client);
    if (IS_KEYBOARD_VISIBLE (client))
        eekboard_context_show_keyboard (client->context, NULL);

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
client_disable_atspi_focus (Client *client)
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
client_enable_atspi_keystroke (Client *client)
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
client_disable_atspi_keystroke (Client *client)
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
    Client *client = user_data;
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

static gboolean
on_hide_keyboard_timeout (Client *client)
{
    eekboard_context_hide_keyboard (client->context, NULL);
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
            eekboard_context_show_keyboard (client->context, NULL);
        } else if (g_settings_get_boolean (client->settings, "auto-hide") &&
                   g_strcmp0 (member, "FocusOut") == 0) {
            gint delay = g_settings_get_int (client->settings,
                                             "auto-hide-delay");
            client->hide_keyboard_timeout_id =
                g_timeout_add (delay,
                               (GSourceFunc)on_hide_keyboard_timeout,
                               client);
        }
    }

    return message;
}

static void
_ibus_connect_focus_handlers (IBusBus *bus, gpointer user_data)
{
    Client *client = user_data;
    GDBusConnection *connection;

    connection = ibus_bus_get_connection (bus);
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
    if (!client->ibus_bus) {
        client->ibus_bus = ibus_bus_new ();
        g_object_ref (client->ibus_bus);
        g_signal_connect (client->ibus_bus, "connected",
                          G_CALLBACK(_ibus_connect_focus_handlers),
                          client);
    }

    if (ibus_bus_is_connected (client->ibus_bus))
        _ibus_connect_focus_handlers (client->ibus_bus, client);

    client->follows_focus = TRUE;
    return TRUE;
}

void
client_disable_ibus_focus (Client *client)
{
    GDBusConnection *connection;

    client->follows_focus = FALSE;

    if (client->ibus_bus) {
        if (client->ibus_focus_message_filter != 0) {
            connection = ibus_bus_get_connection (client->ibus_bus);
            g_dbus_connection_remove_filter (connection,
                                             client->ibus_focus_message_filter);
        }
        g_object_unref (client->ibus_bus);
        client->ibus_bus = NULL;
    }
}
#endif  /* HAVE_ATSPI */

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

    retval = set_keyboard_from_xkl (client);
    g_return_if_fail (retval);

#ifdef HAVE_XTEST
    update_modifier_keycodes (client);
#endif  /* HAVE_XTEST */
}

static gboolean
set_keyboard (Client *client,
              const gchar *keyboard)
{
    GSList *keyboards = NULL;
    gchar **strv, **p;

    g_return_val_if_fail (keyboard != NULL, FALSE);
    g_return_val_if_fail (*keyboard != '\0', FALSE);

    if (client->keyboards)
        g_slist_free (client->keyboards);

    strv = g_strsplit (keyboard, ",", -1);
    for (p = strv; *p != NULL; p++) {
        guint keyboard_id;

        keyboard_id = eekboard_context_add_keyboard (client->context,
                                                     *p,
                                                     NULL);
        if (keyboard_id == 0)
            return FALSE;
        keyboards = g_slist_prepend (keyboards,
                                     GUINT_TO_POINTER(keyboard_id));
    }
    g_strfreev (strv);

    /* make a cycle */
    keyboards = g_slist_reverse (keyboards);
    g_slist_last (keyboards)->next = keyboards;
    client->keyboards = keyboards;

    /* select the first keyboard */
    eekboard_context_set_keyboard (client->context,
                                   GPOINTER_TO_UINT(keyboards->data),
                                   NULL);
    return TRUE;
}

static gboolean
set_keyboard_from_xkl (Client *client)
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
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    gint i;

    for (i = client->xkb->max_key_code; i >= client->xkb->min_key_code; --i)
        if (client->xkb->map->key_sym_map[i].kt_index[0] == XkbOneLevelIndex &&
            XKeycodeToKeysym (xdisplay, i, 0) != 0)
            return i;

    return XKeysymToKeycode (xdisplay, 0x0023); /* XK_numbersign */
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
                 guint          *keycode,
                 guint          *keysym)
{
    GdkDisplay *display = gdk_display_get_default ();
    Display *xdisplay = GDK_DISPLAY_XDISPLAY (display);
    guint offset;
    XkbMapChangesRec changes;
    guint replaced_keycode, replaced_keysym;

    g_assert (keycode != NULL);
    g_assert (keysym != NULL && *keysym != 0);

    replaced_keycode = get_replaced_keycode (client);
    if (replaced_keycode == 0)
        return FALSE;
    replaced_keysym = XKeycodeToKeysym (xdisplay, replaced_keycode, 0);
    XFlush (xdisplay);

    offset = client->xkb->map->key_sym_map[replaced_keycode].offset;
    client->xkb->map->syms[offset] = *keysym;

    changes.changed = XkbKeySymsMask;
    changes.first_key_sym = replaced_keycode;
    changes.num_key_syms = 1;

    XkbChangeMap (xdisplay, client->xkb, &changes);
    XFlush (xdisplay);

    *keycode = replaced_keycode;
    *keysym = replaced_keysym;

    return TRUE;
}

static gboolean
get_keycode_from_gdk_keymap (Client *client,
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
        if (keys[i].group == eekboard_context_get_group (client->context, NULL))
            best_match = &keys[i];

    *keycode = best_match->keycode;
    *modifiers = best_match->level == 1 ? EEK_SHIFT_MASK : 0;

    g_free (keys);

    return TRUE;
}

static void
send_fake_modifier_key_event (Client *client,
                              EekModifierType modifiers,
                              gboolean        is_pressed)
{
    GdkDisplay *display = gdk_display_get_default ();
    gint i;

    for (i = 0; i < G_N_ELEMENTS(client->modifier_keycodes); i++) {
        if (modifiers & (1 << i)) {
            guint keycode = client->modifier_keycodes[i];

            g_return_if_fail (keycode > 0);

            XTestFakeKeyEvent (GDK_DISPLAY_XDISPLAY (display),
                               keycode,
                               is_pressed,
                               CurrentTime);
        }
    }
}

static void
send_fake_key_event (Client *client,
                     EekSymbol      *symbol,
                     guint           keyboard_modifiers,
                     gboolean        is_pressed)
{
    GdkDisplay *display = gdk_display_get_default ();
    EekModifierType modifiers;
    guint xkeysym;
    guint keycode, replaced_keysym = 0;

    /* Ignore special keys and modifiers */
    if (!EEK_IS_KEYSYM(symbol) || eek_symbol_is_modifier (symbol))
        return;

    xkeysym = eek_keysym_get_xkeysym (EEK_KEYSYM(symbol));
    g_return_if_fail (xkeysym > 0);

    modifiers = 0;
    if (!get_keycode_from_gdk_keymap (client, xkeysym, &keycode, &modifiers)) {
        keycode = 0;
        replaced_keysym = xkeysym;
        if (!replace_keycode (client, &keycode, &replaced_keysym)) {
            g_warning ("failed to lookup X keysym %X", xkeysym);
            return;
        }
    }
    
    /* Clear level shift modifiers */
    keyboard_modifiers &= ~EEK_SHIFT_MASK;
    keyboard_modifiers &= ~EEK_LOCK_MASK;
    //keyboard_modifiers &= ~eek_keyboard_get_alt_gr_mask (client->keyboard);

    modifiers |= keyboard_modifiers;

    send_fake_modifier_key_event (client, modifiers, is_pressed);
    XSync (GDK_DISPLAY_XDISPLAY (display), False);

    keycode = XKeysymToKeycode (GDK_DISPLAY_XDISPLAY (display),
                                xkeysym);
    g_return_if_fail (keycode > 0);

    XTestFakeKeyEvent (GDK_DISPLAY_XDISPLAY (display),
                       keycode,
                       is_pressed,
                       CurrentTime);
    XSync (GDK_DISPLAY_XDISPLAY (display), False);

    if (replaced_keysym)
        replace_keycode (client, &keycode, &replaced_keysym);
}

static void
on_key_pressed (EekboardContext *context,
                const gchar     *keyname,
                EekSymbol       *symbol,
                guint            modifiers,
                gpointer         user_data)
{
    Client *client = user_data;

    if (g_strcmp0 (eek_symbol_get_name (symbol), "cycle-keyboard") == 0) {
        client->keyboards = g_slist_next (client->keyboards);
        eekboard_context_set_keyboard (client->context,
                                       GPOINTER_TO_UINT(client->keyboards->data),
                                       NULL);
        return;
    }

    if (g_strcmp0 (eek_symbol_get_name (symbol), "preferences") == 0) {
        PreferencesDialog *dialog = preferences_dialog_new ();
        preferences_dialog_run (dialog);
    }


    send_fake_key_event (client, symbol, modifiers, TRUE);
    send_fake_key_event (client, symbol, modifiers, FALSE);
}

static void
update_modifier_keycodes (Client *client)
{
    GdkDisplay *display = gdk_display_get_default ();
    XModifierKeymap *mods;
    gint i, j;

    mods = XGetModifierMapping (GDK_DISPLAY_XDISPLAY (display));
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
    int opcode, event_base, error_base, major_version, minor_version;

    g_assert (display);

    if (!XTestQueryExtension (GDK_DISPLAY_XDISPLAY (display),
                              &event_base, &error_base,
                              &major_version, &minor_version)) {
        g_warning ("XTest extension is not available");
        return FALSE;
    }

    if (!XkbQueryExtension (GDK_DISPLAY_XDISPLAY (display),
                            &opcode, &event_base, &error_base,
                            &major_version, &minor_version)) {
        g_warning ("Xkb extension is not available");
        return FALSE;
    }

    if (!client->xkb)
        client->xkb = XkbGetMap (GDK_DISPLAY_XDISPLAY (display),
                                 XkbKeySymsMask,
                                 XkbUseCoreKbd);
    g_assert (client->xkb);

    update_modifier_keycodes (client);

    client->key_pressed_handler =
        g_signal_connect (client->context, "key-pressed",
                          G_CALLBACK(on_key_pressed), client);

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
