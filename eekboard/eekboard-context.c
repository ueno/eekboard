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

#include "eekboard/eekboard-context.h"

enum {
    ENABLED,
    DISABLED,
    KEY_PRESSED,
    KEY_RELEASED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (EekboardContext, eekboard_context, G_TYPE_DBUS_PROXY);

#define EEKBOARD_CONTEXT_GET_PRIVATE(obj)                               \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEKBOARD_TYPE_CONTEXT, EekboardContextPrivate))

struct _EekboardContextPrivate
{
    EekKeyboard *keyboard;
    gboolean keyboard_visible;
    gboolean enabled;
};

static void
eekboard_context_real_g_signal (GDBusProxy  *self,
                                const gchar *sender_name,
                                const gchar *signal_name,
                                GVariant    *parameters)
{
    EekboardContext *context = EEKBOARD_CONTEXT (self);
    EekboardContextPrivate *priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);

    if (g_strcmp0 (signal_name, "Enabled") == 0) {
        g_signal_emit_by_name (context, "enabled");
        return;
    }

    if (g_strcmp0 (signal_name, "Disabled") == 0) {
        g_signal_emit_by_name (context, "disabled");
        return;
    }

    if (g_strcmp0 (signal_name, "KeyPressed") == 0) {
        guint keycode;
        g_variant_get (parameters, "(u)", &keycode);
        g_signal_emit_by_name (context, "key-pressed", keycode);
        return;
    }

    if (g_strcmp0 (signal_name, "KeyReleased") == 0) {
        guint keycode;
        g_variant_get (parameters, "(u)", &keycode);
        g_signal_emit_by_name (context, "key-released", keycode);
        return;
    }

    if (g_strcmp0 (signal_name, "KeyboardVisibilityChanged") == 0) {
        g_variant_get (parameters, "(b)", &priv->keyboard_visible);
        return;
    }

    g_return_if_reached ();
}

static void
eekboard_context_real_enabled (EekboardContext *self)
{
    EekboardContextPrivate *priv = EEKBOARD_CONTEXT_GET_PRIVATE(self);
    priv->enabled = TRUE;
}

static void
eekboard_context_real_disabled (EekboardContext *self)
{
    EekboardContextPrivate *priv = EEKBOARD_CONTEXT_GET_PRIVATE(self);
    priv->enabled = FALSE;
}

static void
eekboard_context_real_key_pressed (EekboardContext *self,
                                   guint            keycode)
{
    EekboardContextPrivate *priv = EEKBOARD_CONTEXT_GET_PRIVATE(self);
    if (priv->keyboard) {
        EekKey *key = eek_keyboard_find_key_by_keycode (priv->keyboard,
                                                        keycode);
        g_signal_emit_by_name (key, "pressed");
    }
}

static void
eekboard_context_real_key_released (EekboardContext *self,
                                    guint             keycode)
{
    EekboardContextPrivate *priv = EEKBOARD_CONTEXT_GET_PRIVATE(self);
    if (priv->keyboard) {
        EekKey *key = eek_keyboard_find_key_by_keycode (priv->keyboard,
                                                        keycode);
        g_signal_emit_by_name (key, "released");
    }
}

static void
eekboard_context_dispose (GObject *self)
{
    EekboardContextPrivate *priv = EEKBOARD_CONTEXT_GET_PRIVATE (self);
    
    if (priv->keyboard) {
        g_object_unref (priv->keyboard);
        priv->keyboard = NULL;
    }
}

static void
eekboard_context_class_init (EekboardContextClass *klass)
{
    GDBusProxyClass *proxy_class = G_DBUS_PROXY_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekboardContextPrivate));

    klass->enabled = eekboard_context_real_enabled;
    klass->disabled = eekboard_context_real_disabled;
    klass->key_pressed = eekboard_context_real_key_pressed;
    klass->key_released = eekboard_context_real_key_released;

    proxy_class->g_signal = eekboard_context_real_g_signal;

    gobject_class->dispose = eekboard_context_dispose;

    signals[ENABLED] =
        g_signal_new ("enabled",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekboardContextClass, enabled),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);

    signals[DISABLED] =
        g_signal_new ("disabled",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekboardContextClass, disabled),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);

    signals[KEY_PRESSED] =
        g_signal_new ("key-pressed",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekboardContextClass, key_pressed),
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
                      G_STRUCT_OFFSET(EekboardContextClass, key_released),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_UINT);
}

static void
eekboard_context_init (EekboardContext *self)
{
    EekboardContextPrivate *priv;

    priv = self->priv = EEKBOARD_CONTEXT_GET_PRIVATE(self);
    priv->keyboard = NULL;
    priv->keyboard_visible = FALSE;
    priv->enabled = FALSE;
}

EekboardContext *
eekboard_context_new (GDBusConnection *connection,
                      const gchar     *object_path,
                      GCancellable    *cancellable)
{
    GInitable *initable;
    GError *error;

    g_assert (object_path != NULL);
    g_assert (G_IS_DBUS_CONNECTION(connection));

    error = NULL;
    initable =
        g_initable_new (EEKBOARD_TYPE_CONTEXT,
                        cancellable,
                        &error,
                        "g-name", "com.redhat.Eekboard.Server",
                        "g-connection", connection,
                        "g-interface-name", "com.redhat.Eekboard.Context",
                        "g-object-path", object_path,
                        NULL);
    if (initable != NULL)
        return EEKBOARD_CONTEXT (initable);
    return NULL;
}

static void
context_async_ready_callback (GObject      *source_object,
                              GAsyncResult *res,
                              gpointer      user_data)
{
    GError *error = NULL;
    GVariant *result;

    result = g_dbus_proxy_call_finish (G_DBUS_PROXY(source_object),
                                       res,
                                       &error);
    if (result)
        g_variant_unref (result);
}

void
eekboard_context_set_keyboard (EekboardContext *context,
                               EekKeyboard     *keyboard,
                               GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;
    GVariant *variant;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE(context);
    if (priv->keyboard)
        g_object_unref (priv->keyboard);
    priv->keyboard = g_object_ref (keyboard);

    variant = eek_serializable_serialize (EEK_SERIALIZABLE(priv->keyboard));
    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "SetKeyboard",
                       g_variant_new ("(v)", variant),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
    g_variant_unref (variant);
}

void
eekboard_context_set_group (EekboardContext *context,
                            gint             group,
                            GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);

    g_return_if_fail (priv->keyboard);
    g_return_if_fail (group >= 0);

    eek_keyboard_set_group (priv->keyboard, group);
    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "SetGroup",
                       g_variant_new ("(i)", group),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
}

void
eekboard_context_show_keyboard (EekboardContext *context,
                                GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    if (!priv->enabled)
        return;

    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "ShowKeyboard",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
}

void
eekboard_context_hide_keyboard (EekboardContext *context,
                                GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    if (!priv->enabled)
        return;

    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "HideKeyboard",
                       NULL,
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
}

void
eekboard_context_press_key (EekboardContext *context,
                            guint            keycode,
                            GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    if (!priv->enabled)
        return;

    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "PressKey",
                       g_variant_new ("(u)", keycode),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
}

void
eekboard_context_release_key (EekboardContext *context,
                              guint            keycode,
                              GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    if (!priv->enabled)
        return;

    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "ReleaseKey",
                       g_variant_new ("(u)", keycode),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
}

gboolean
eekboard_context_is_keyboard_visible (EekboardContext *context)
{
    EekboardContextPrivate *priv;

    g_assert (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    return priv->enabled && priv->keyboard_visible;
}

void
eekboard_context_set_enabled (EekboardContext *context,
                              gboolean         enabled)
{
    EekboardContextPrivate *priv;

    g_assert (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    priv->enabled = enabled;
}
