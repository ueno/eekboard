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

#include "eekboard-device.h"

enum {
    KEY_PRESSED,
    KEY_RELEASED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

struct _EekboardDevice {
    GDBusProxy parent;
};

struct _EekboardDeviceClass {
    GDBusProxyClass parent_class;
};

G_DEFINE_TYPE (EekboardDevice, eekboard_device, G_TYPE_DBUS_PROXY);

static void
eekboard_device_real_g_signal (GDBusProxy  *self,
                              const gchar *sender_name,
                              const gchar *signal_name,
                              GVariant    *parameters)
{
    EekboardDevice *device = EEKBOARD_DEVICE (self);
    guint *keycode;

    if (g_strcmp0 (signal_name, "KeyPressed") == 0) {

        g_variant_get (parameters, "(u)", &keycode);
        g_signal_emit_by_name (device, "key-pressed", keycode);
        return;
    }

    if (g_strcmp0 (signal_name, "KeyReleased") == 0) {
        g_variant_get (parameters, "(u)", &keycode);
        g_signal_emit_by_name (device, "key-released", keycode);
        return;
    }

    g_return_if_reached ();
}

static void
eekboard_device_class_init (EekboardDeviceClass *klass)
{
    GDBusProxyClass *proxy_class = G_DBUS_PROXY_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    proxy_class->g_signal = eekboard_device_real_g_signal;

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
eekboard_device_init (EekboardDevice *device)
{
}

EekboardDevice *
eekboard_device_new (const gchar     *path,
                     GDBusConnection *connection,
                     GCancellable    *cancellable,
                     GError         **error)
{
    GInitable *initable;

    g_assert (path != NULL);
    g_assert (G_IS_DBUS_CONNECTION(connection));

    initable =
        g_initable_new (EEKBOARD_TYPE_DEVICE,
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
        return EEKBOARD_DEVICE (initable);
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
eekboard_device_set_keyboard (EekboardDevice *device, EekKeyboard *keyboard)
{
    GVariant *variant;

    variant = eek_serializable_serialize (EEK_SERIALIZABLE(keyboard));
    g_dbus_proxy_call (G_DBUS_PROXY(device),
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
eekboard_device_set_group (EekboardDevice *device,
                           gint            group)
{
    g_dbus_proxy_call (G_DBUS_PROXY(device),
                       "SetGroup",
                       g_variant_new ("(i)", group),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}

void
eekboard_device_show (EekboardDevice *device)
{
    g_dbus_proxy_call (G_DBUS_PROXY(device),
                       "Show",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}

void
eekboard_device_hide (EekboardDevice *device)
{
    g_dbus_proxy_call (G_DBUS_PROXY(device),
                       "Hide",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}

void
eekboard_device_press_key (EekboardDevice *device,
                           guint           keycode)
{
    g_dbus_proxy_call (G_DBUS_PROXY(device),
                       "PressKey",
                       g_variant_new ("(u)", keycode),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}

void
eekboard_device_release_key (EekboardDevice *device,
                             guint           keycode)
{
    g_dbus_proxy_call (G_DBUS_PROXY(device),
                       "ReleaseKey",
                       g_variant_new ("(u)", keycode),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       NULL,
                       proxy_call_async_ready_cb,
                       NULL);
}
