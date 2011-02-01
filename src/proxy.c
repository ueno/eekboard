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

#include "proxy.h"

#define BUFSIZE 8192

enum {
    KEY_PRESSED,
    KEY_RELEASED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

struct _EekboardProxy {
    GDBusProxy parent;
};

struct _EekboardProxyClass {
    GDBusProxyClass parent_class;
};

G_DEFINE_TYPE (EekboardProxy, eekboard_proxy, G_TYPE_DBUS_PROXY);

static void
eekboard_proxy_real_g_signal (GDBusProxy  *self,
                              const gchar *sender_name,
                              const gchar *signal_name,
                              GVariant    *parameters)
{
    EekboardProxy *proxy = EEKBOARD_PROXY (self);
    guint *keycode;

    if (g_strcmp0 (signal_name, "KeyPressed") == 0) {

        g_variant_get (parameters, "(u)", &keycode);
        g_signal_emit_by_name (proxy, "key-pressed", keycode);
        return;
    }

    if (g_strcmp0 (signal_name, "KeyReleased") == 0) {
        g_variant_get (parameters, "(u)", &keycode);
        g_signal_emit_by_name (proxy, "key-released", keycode);
        return;
    }

    g_return_if_reached ();
}

static void
eekboard_proxy_class_init (EekboardProxyClass *klass)
{
    GDBusProxyClass *proxy_class = G_DBUS_PROXY_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    proxy_class->g_signal = eekboard_proxy_real_g_signal;

    signals[KEY_PRESSED] =
        g_signal_new ("key-pressed",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_UINT);

    signals[KEY_RELEASED] =
        g_signal_new ("key-released",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_UINT);
}

static void
eekboard_proxy_init (EekboardProxy *proxy)
{
}

EekboardProxy *
eekboard_proxy_new (const gchar     *path,
                    GDBusConnection *connection,
                    GCancellable    *cancellable,
                    GError         **error)
{
    GInitable *initable;

    g_assert (path != NULL);
    g_assert (G_IS_DBUS_CONNECTION(connection));

    initable =
        g_initable_new (EEKBOARD_TYPE_PROXY,
                        cancellable,
                        error,
                        "g-connection", connection,
                        "g-name", "com.redhat.eekboard.Keyboard",
                        "g-flags", G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START |
                        G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                        "g-interface-name", "com.redhat.eekboard.Keyboard",
                        "g-object-path", path,
                        NULL);
    if (initable != NULL)
        return EEKBOARD_PROXY (initable);
    return NULL;
}

static void
proxy_call_async_ready_cb (GObject      *source_object,
                           GAsyncResult *res,
                           gpointer      user_data)
{
    GError *error = NULL;
    GVariant *result;

    result = g_dbus_proxy_call_finish (G_DBUS_PROXY(source_object),
                                       res,
                                       &error);
    // g_assert_no_error (error);
    if (result)
        g_variant_unref (result);
}

void
eekboard_proxy_set_keyboard (EekboardProxy *proxy, EekKeyboard *keyboard)
{
    GString *output;
    GVariant *variant;
    gchar *data;

    output = g_string_sized_new (BUFSIZE);
    eek_keyboard_output (keyboard, output, 0);

    data = g_string_free (output, FALSE);
    variant = g_variant_new ("(s)", data);
    g_free (data);

    g_dbus_proxy_call (G_DBUS_PROXY(proxy),
                       "SetKeyboard",
                       g_variant_new ("(v)", variant),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
    g_variant_unref (variant);
}

void
eekboard_proxy_set_group (EekboardProxy *proxy, gint group)
{
    g_dbus_proxy_call (G_DBUS_PROXY(proxy),
                       "SetGroup",
                       g_variant_new ("(i)", group),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}

void
eekboard_proxy_show (EekboardProxy *proxy)
{
    g_dbus_proxy_call (G_DBUS_PROXY(proxy),
                       "Show",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}

void
eekboard_proxy_hide (EekboardProxy *proxy)
{
    g_dbus_proxy_call (G_DBUS_PROXY(proxy),
                       "Hide",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}

void
eekboard_proxy_press_key (EekboardProxy *proxy,
                          guint          keycode)
{
    g_dbus_proxy_call (G_DBUS_PROXY(proxy),
                       "PressKey",
                       g_variant_new ("(u)", keycode),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}

void
eekboard_proxy_release_key (EekboardProxy *proxy,
                            guint          keycode)
{
    g_dbus_proxy_call (G_DBUS_PROXY(proxy),
                       "ReleaseKey",
                       g_variant_new ("(u)", keycode),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}
