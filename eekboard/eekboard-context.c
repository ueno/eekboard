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

/**
 * SECTION:eekboard-context
 * @short_description: input context maintained by #EekboardServer.
 *
 * The #EekboardContext class provides a client access to remote input
 * context.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eekboard/eekboard-context.h"

#define I_(string) g_intern_static_string (string)

enum {
    ENABLED,
    DISABLED,
    KEY_PRESSED,
    KEY_RELEASED,
    DESTROYED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

enum {
    PROP_0,
    PROP_KEYBOARD_VISIBLE,
    PROP_LAST
};

G_DEFINE_TYPE (EekboardContext, eekboard_context, G_TYPE_DBUS_PROXY);

#define EEKBOARD_CONTEXT_GET_PRIVATE(obj)                               \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEKBOARD_TYPE_CONTEXT, EekboardContextPrivate))

struct _EekboardContextPrivate
{
    EekKeyboard *keyboard;
    GHashTable *keyboard_hash;
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
        gboolean keyboard_visible;

        g_variant_get (parameters, "(b)", &keyboard_visible);
        if (keyboard_visible != priv->keyboard_visible) {
            priv->keyboard_visible = keyboard_visible;
            g_object_notify (G_OBJECT(context), "keyboard-visible");
        }
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
eekboard_context_real_destroyed (EekboardContext *self)
{
    // g_debug ("eekboard_context_real_destroyed");
}

static void
eekboard_context_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    EekboardContextPrivate *priv = EEKBOARD_CONTEXT_GET_PRIVATE(object);
    switch (prop_id) {
    case PROP_KEYBOARD_VISIBLE:
        g_value_set_boolean (value, priv->keyboard_visible);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
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

    if (priv->keyboard_hash) {
        g_hash_table_destroy (priv->keyboard_hash);
        priv->keyboard_hash = NULL;
    }
}

static void
eekboard_context_class_init (EekboardContextClass *klass)
{
    GDBusProxyClass *proxy_class = G_DBUS_PROXY_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekboardContextPrivate));

    klass->enabled = eekboard_context_real_enabled;
    klass->disabled = eekboard_context_real_disabled;
    klass->key_pressed = eekboard_context_real_key_pressed;
    klass->key_released = eekboard_context_real_key_released;
    klass->destroyed = eekboard_context_real_destroyed;

    proxy_class->g_signal = eekboard_context_real_g_signal;

    gobject_class->get_property = eekboard_context_get_property;
    gobject_class->dispose = eekboard_context_dispose;

    /**
     * EekboardContext:keyboard-visible:
     *
     * Flag to indicate if keyboard is visible or not.
     */
    pspec = g_param_spec_boolean ("keyboard-visible",
                                  "Keyboard-visible",
                                  "Flag that indicates if keyboard is visible",
                                  FALSE,
                                  G_PARAM_READABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_KEYBOARD_VISIBLE,
                                     pspec);

    /**
     * EekboardContext::enabled:
     * @context: an #EekboardContext
     *
     * Emitted when @context is enabled.
     */
    signals[ENABLED] =
        g_signal_new (I_("enabled"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekboardContextClass, enabled),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);

    /**
     * EekboardContext::disabled:
     * @context: an #EekboardContext
     *
     * The ::disabled signal is emitted each time @context is disabled.
     */
    signals[DISABLED] =
        g_signal_new (I_("disabled"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekboardContextClass, disabled),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);

    /**
     * EekboardContext::key-pressed:
     * @context: an #EekboardContext
     * @keycode: keycode
     *
     * The ::key-pressed signal is emitted each time a key is pressed
     * in @context.
     */
    signals[KEY_PRESSED] =
        g_signal_new (I_("key-pressed"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekboardContextClass, key_pressed),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_UINT);

    /**
     * EekboardContext::key-released:
     * @context: an #EekboardContext
     * @keycode: keycode
     *
     * The ::key-released signal is emitted each time a key is released
     * in @context.
     */
    signals[KEY_RELEASED] =
        g_signal_new (I_("key-released"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekboardContextClass, key_released),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__UINT,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_UINT);

    /**
     * EekboardContext::destroyed:
     * @context: an #EekboardContext
     *
     * The ::destroyed signal is emitted each time the name of remote
     * end is vanished.
     */
    signals[DESTROYED] =
        g_signal_new (I_("destroyed"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekboardContextClass, destroyed),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);
}

static void
eekboard_context_init (EekboardContext *self)
{
    EekboardContextPrivate *priv;

    priv = self->priv = EEKBOARD_CONTEXT_GET_PRIVATE(self);
    priv->keyboard = NULL;
    priv->keyboard_visible = FALSE;
    priv->enabled = FALSE;
    priv->keyboard_hash =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               (GDestroyNotify)g_object_unref);
}

static void
context_name_vanished_callback (GDBusConnection *connection,
                                const gchar     *name,
                                gpointer         user_data)
{
    EekboardContext *context = user_data;
    g_signal_emit_by_name (context, "destroyed", NULL);
}

/**
 * eekboard_context_new:
 * @connection: a #GDBusConnection
 * @object_path: object path
 * @cancellable: a #GCancellable
 *
 * Create a D-Bus proxy of an input context maintained by
 * eekboard-server.  This function is seldom called from applications
 * since eekboard_server_create_context() calls it implicitly.
 */
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
    if (initable != NULL) {
        EekboardContext *context = EEKBOARD_CONTEXT (initable);
        gchar *name_owner = g_dbus_proxy_get_name_owner (G_DBUS_PROXY(context));

        if (name_owner == NULL) {
            g_object_unref (context);
            return NULL;
        }

        g_bus_watch_name_on_connection (connection,
                                        name_owner,
                                        G_BUS_NAME_WATCHER_FLAGS_NONE,
                                        NULL,
                                        context_name_vanished_callback,
                                        context,
                                        NULL);
        g_free (name_owner);

        return context;
    }
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

/**
 * eekboard_context_add_keyboard:
 * @context: an #EekboardContext
 * @keyboard: an #EekKeyboard
 * @cancellable: a #GCancellable
 *
 * Register @keyboard in @context.
 */
guint
eekboard_context_add_keyboard (EekboardContext *context,
                               EekKeyboard     *keyboard,
                               GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;
    GVariant *variant, *result;
    GError *error;

    g_return_val_if_fail (EEKBOARD_IS_CONTEXT(context), 0);
    g_return_val_if_fail (EEK_IS_KEYBOARD(keyboard), 0);

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);

    variant = eek_serializable_serialize (EEK_SERIALIZABLE(keyboard));

    error = NULL;
    result = g_dbus_proxy_call_sync (G_DBUS_PROXY(context),
                                     "AddKeyboard",
                                     g_variant_new ("(v)", variant),
                                     G_DBUS_CALL_FLAGS_NONE,
                                     -1,
                                     cancellable,
                                     &error);
    g_variant_unref (variant);

    if (result) {
        guint keyboard_id;

        g_variant_get (result, "(u)", &keyboard_id);
        g_variant_unref (result);

        if (keyboard_id != 0) {
            g_hash_table_insert (priv->keyboard_hash,
                                 GUINT_TO_POINTER(keyboard_id),
                                 g_object_ref (keyboard));
        }
        return keyboard_id;
    }
    return 0;
}

/**
 * eekboard_context_remove_keyboard:
 * @context: an #EekboardContext
 * @keyboard_id: keyboard ID
 * @cancellable: a #GCancellable
 *
 * Unregister the keyboard with @keyboard_id in @context.
 */
void
eekboard_context_remove_keyboard (EekboardContext *context,
                                  guint            keyboard_id,
                                  GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;
    EekKeyboard *keyboard;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);

    keyboard = g_hash_table_lookup (priv->keyboard_hash,
                                    GUINT_TO_POINTER(keyboard_id));
    if (keyboard == priv->keyboard)
        priv->keyboard = NULL;

    g_hash_table_remove (priv->keyboard_hash, GUINT_TO_POINTER(keyboard_id));

    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "RemoveKeyboard",
                       g_variant_new ("(u)", keyboard_id),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
}

/**
 * eekboard_context_set_keyboard:
 * @context: an #EekboardContext
 * @keyboard_id: keyboard ID
 * @cancellable: a #GCancellable
 *
 * Select a keyboard with ID @keyboard_id in @context.
 */
void
eekboard_context_set_keyboard (EekboardContext *context,
                               guint            keyboard_id,
                               GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;
    EekKeyboard *keyboard;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);

    keyboard = g_hash_table_lookup (priv->keyboard_hash,
                                    GUINT_TO_POINTER(keyboard_id));
    if (!keyboard || keyboard == priv->keyboard)
        return;

    priv->keyboard = keyboard;

    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "SetKeyboard",
                       g_variant_new ("(u)", keyboard_id),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
}

/**
 * eekboard_context_set_group:
 * @context: an #EekboardContext
 * @group: group number
 * @cancellable: a #GCancellable
 *
 * Set the keyboard group of @context.
 */
void
eekboard_context_set_group (EekboardContext *context,
                            gint             group,
                            GCancellable    *cancellable)
{
    EekboardContextPrivate *priv;

    g_return_if_fail (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);

    g_return_if_fail (priv->keyboard);

    eek_element_set_group (EEK_ELEMENT(priv->keyboard), group);
    g_dbus_proxy_call (G_DBUS_PROXY(context),
                       "SetGroup",
                       g_variant_new ("(i)", group),
                       G_DBUS_CALL_FLAGS_NONE,
                       -1,
                       cancellable,
                       context_async_ready_callback,
                       NULL);
}

/**
 * eekboard_context_show_keyboard:
 * @context: an #EekboardContext
 * @cancellable: a #GCancellable
 *
 * Request eekboard-server to show a keyboard set by
 * eekboard_context_set_keyboard().
 */
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

/**
 * eekboard_context_hide_keyboard:
 * @context: an #EekboardContext
 * @cancellable: a #GCancellable
 *
 * Request eekboard-server to hide a keyboard.
 */
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

/**
 * eekboard_context_press_key:
 * @context: an #EekboardContext
 * @keycode: keycode number
 * @cancellable: a #GCancellable
 *
 * Tell eekboard-server that a key identified by @keycode is pressed.
 */
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

/**
 * eekboard_context_release_key:
 * @context: an #EekboardContext
 * @keycode: keycode number
 * @cancellable: a #GCancellable
 *
 * Tell eekboard-server that a key identified by @keycode is released.
 */
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

/**
 * eekboard_context_is_keyboard_visible:
 * @context: an #EekboardContext
 *
 * Check if keyboard is visible.
 */
gboolean
eekboard_context_is_keyboard_visible (EekboardContext *context)
{
    EekboardContextPrivate *priv;

    g_assert (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    return priv->enabled && priv->keyboard_visible;
}

/**
 * eekboard_context_set_enabled:
 * @context: an #EekboardContext
 * @enabled: flag to indicate if @context is enabled
 *
 * Set @context enabled or disabled.  This function is seldom called
 * since the flag is set via D-Bus signal #EekboardContext::enabled
 * and #EekboardContext::disabled.
 */
void
eekboard_context_set_enabled (EekboardContext *context,
                              gboolean         enabled)
{
    EekboardContextPrivate *priv;

    g_assert (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    priv->enabled = enabled;
}

/**
 * eekboard_context_is_enabled:
 * @context: an #EekboardContext
 *
 * Check if @context is enabled.
 */
gboolean
eekboard_context_is_enabled (EekboardContext *context)
{
    EekboardContextPrivate *priv;

    g_assert (EEKBOARD_IS_CONTEXT(context));

    priv = EEKBOARD_CONTEXT_GET_PRIVATE (context);
    return priv->enabled;
}
