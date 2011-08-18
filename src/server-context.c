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

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <X11/Xatom.h>
#include <gdk/gdkx.h>

#include "eek/eek.h"

#if HAVE_CLUTTER_GTK
#include <clutter-gtk/clutter-gtk.h>
#include "eek/eek-clutter.h"
#endif
#include "eek/eek-gtk.h"
#include "eek/eek-xkl.h"

#include "server-context.h"
#include "xklutil.h"

#define CSW 640
#define CSH 480
#define DEFAULT_THEME (THEMEDIR "/default.css")

enum {
    PROP_0,
    PROP_OBJECT_PATH,
    PROP_CONNECTION,
    PROP_UI_TOOLKIT,
    PROP_LAST
};

static const gchar introspection_xml[] =
    "<node>"
    "  <interface name='org.fedorahosted.Eekboard.Context'>"
    "    <method name='AddKeyboard'>"
    "      <arg direction='in' type='s' name='keyboard'/>"
    "      <arg direction='out' type='u' name='keyboard_id'/>"
    "    </method>"
    "    <method name='RemoveKeyboard'>"
    "      <arg direction='in' type='u' name='keyboard_id'/>"
    "    </method>"
    "    <method name='SetKeyboard'>"
    "      <arg type='u' name='keyboard_id'/>"
    "    </method>"
    "    <method name='SetFullscreen'>"
    "      <arg type='b' name='fullscreen'/>"
    "    </method>"
    "    <method name='ShowKeyboard'/>"
    "    <method name='HideKeyboard'/>"
    "    <method name='SetGroup'>"
    "      <arg type='i' name='group'/>"
    "    </method>"
    "    <method name='PressKey'>"
    "      <arg type='u' name='keycode'/>"
    "    </method>"
    "    <method name='ReleaseKey'>"
    "      <arg type='u' name='keycode'/>"
    "    </method>"
    /* signals */
    "    <signal name='Enabled'/>"
    "    <signal name='Disabled'/>"
    "    <signal name='KeyPressed'>"
    "      <arg type='s' name='keyname'/>"
    "      <arg type='v' name='symbol'/>"
    "      <arg type='u' name='modifiers'/>"
    "    </signal>"
    "    <signal name='KeyboardVisibilityChanged'>"
    "      <arg type='b' name='visible'/>"
    "    </signal>"
    "    <signal name='KeyboardChanged'>"
    "      <arg type='u' name='keyboard_id'/>"
    "    </signal>"
    "    <signal name='GroupChanged'>"
    "      <arg type='i' name='group'/>"
    "    </signal>"
    "  </interface>"
    "</node>";

typedef enum {
    UI_TOOLKIT_GTK,
    UI_TOOLKIT_CLUTTER,
    UI_TOOLKIT_DEFAULT = UI_TOOLKIT_GTK
} ServerContextUIToolkitType;

typedef struct _ServerContextClass ServerContextClass;

struct _ServerContext {
    GObject parent;
    GDBusConnection *connection;
    GDBusNodeInfo *introspection_data;
    guint registration_id;
    char *object_path;
    char *client_connection;
    char *client_name;

    gboolean enabled;
    gboolean last_keyboard_visible;
    gboolean fullscreen;

    GtkWidget *window;
    GtkWidget *widget;
    guint keyboard_id;
    EekKeyboard *keyboard;
    GHashTable *keyboard_hash;

    gulong key_pressed_handler;
    gulong key_released_handler;
    gulong notify_visible_handler;

    EekKey *repeat_key;
    guint repeat_timeout_id;
    gboolean repeat_triggered;

    GSettings *settings;
    ServerContextUIToolkitType ui_toolkit;
};

struct _ServerContextClass {
    GObjectClass parent_class;
};

G_DEFINE_TYPE (ServerContext, server_context, G_TYPE_OBJECT);

static void disconnect_keyboard_signals (ServerContext         *context);
static void handle_method_call          (GDBusConnection       *connection,
                                         const gchar           *sender,
                                         const gchar           *object_path,
                                         const gchar           *interface_name,
                                         const gchar           *method_name,
                                         GVariant              *parameters,
                                         GDBusMethodInvocation *invocation,
                                         gpointer               user_data);

static const GDBusInterfaceVTable interface_vtable =
{
  handle_method_call,
  NULL,
  NULL
};

#if HAVE_CLUTTER_GTK
static void
on_allocation_changed (ClutterActor          *stage,
                       ClutterActorBox       *box,
                       ClutterAllocationFlags flags,
                       gpointer               user_data)
{
    ClutterActor *actor =
        clutter_container_find_child_by_name (CLUTTER_CONTAINER(stage),
                                              "keyboard");

    clutter_actor_set_size (actor,
                            box->x2 - box->x1,
                            box->y2 - box->y1);
}
#endif

static void
on_destroy (GtkWidget *widget, gpointer user_data)
{
    ServerContext *context = user_data;

    g_assert (widget == context->window);
    context->window = NULL;
    context->widget = NULL;
}

static void
on_notify_visible (GObject *object, GParamSpec *spec, gpointer user_data)
{
    ServerContext *context = user_data;
    gboolean visible;
    GError *error;

    g_object_get (object, "visible", &visible, NULL);

    if (context->connection && context->enabled) {
        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "KeyboardVisibilityChanged",
                                       g_variant_new ("(b)", visible),
                                       &error);
        g_assert_no_error (error);
    }
}

static void
on_realize_set_dock (GtkWidget *widget,
                     gpointer   user_data)
{
#ifdef HAVE_XDOCK
    GdkWindow *window = gtk_widget_get_window (widget);
    gint x, y, width, height, depth;
    long vals[12];

    /* set window type to dock */
    gdk_window_set_type_hint (window, GDK_WINDOW_TYPE_HINT_DOCK);
  
    /* set bottom strut */
#if GTK_CHECK_VERSION(3,0,0)
    gdk_window_get_geometry (window, &x, &y, &width, &height);
#else
    gdk_window_get_geometry (window, &x, &y, &width, &height, &depth);
#endif  /* GTK_CHECK_VERSION(3,0,0) */

    vals[0] = 0;
    vals[1] = 0;
    vals[2] = 0;
    vals[3] = height;
    vals[4] = 0;
    vals[5] = 0;
    vals[6] = 0;
    vals[7] = 0;
    vals[8] = 0;
    vals[9] = 0;
    vals[10] = x;
    vals[11] = x + width;

    XChangeProperty (GDK_WINDOW_XDISPLAY (window),
                     GDK_WINDOW_XID (window),
                     XInternAtom (GDK_WINDOW_XDISPLAY (window),
                                  "_NET_WM_STRUT_PARTIAL", False),
                     XA_CARDINAL, 32, PropModeReplace,
                     (guchar *)vals, 12);
#endif  /* HAVE_XDOCK */
}

static void
on_realize_set_non_maximizable (GtkWidget *widget,
                                gpointer   user_data)
{
    ServerContext *context = user_data;

    g_assert (context && context->window == widget);

    /* make the window not maximizable */
    gdk_window_set_functions (gtk_widget_get_window (widget),
                              GDK_FUNC_RESIZE |
                              GDK_FUNC_MOVE |
                              GDK_FUNC_MINIMIZE |
                              GDK_FUNC_CLOSE);
}

static void
set_geometry (ServerContext *context)
{
    GdkScreen *screen;
    GdkWindow *root;
    gint monitor;
    GdkRectangle rect;
    EekBounds bounds;

    screen = gdk_screen_get_default ();
    root = gtk_widget_get_root_window (context->window);
    monitor = gdk_screen_get_monitor_at_window (screen, root);
    gdk_screen_get_monitor_geometry (screen, monitor, &rect);
    eek_element_get_bounds (EEK_ELEMENT(context->keyboard), &bounds);

    g_signal_handlers_disconnect_by_func (context->window,
                                          on_realize_set_dock,
                                          context);
    g_signal_handlers_disconnect_by_func (context->window,
                                          on_realize_set_non_maximizable,
                                          context);

    if (context->fullscreen) {
        gint width = rect.width, height = rect.height / 2;

        if (width * bounds.height > height * bounds.width)
            width = (height / bounds.height) * bounds.width;
        else
            height = (width / bounds.width) * bounds.height;

        gtk_widget_set_size_request (context->widget, width, height);

        gtk_window_move (GTK_WINDOW(context->window),
                         (rect.width - width) / 2,
                         rect.height - height);

        gtk_window_set_decorated (GTK_WINDOW(context->window), FALSE);
        gtk_window_set_resizable (GTK_WINDOW(context->window), FALSE);
        gtk_window_set_opacity (GTK_WINDOW(context->window), 0.8);

        g_signal_connect_after (context->window, "realize",
                                G_CALLBACK(on_realize_set_dock),
                                context);
    } else {
        if (context->ui_toolkit == UI_TOOLKIT_CLUTTER) {
#if HAVE_CLUTTER_GTK
            ClutterActor *stage =
                gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED(context->widget));
            clutter_stage_set_user_resizable (CLUTTER_STAGE(stage), TRUE);
            clutter_stage_set_minimum_size (CLUTTER_STAGE(stage),
                                            bounds.width / 3,
                                            bounds.height / 3);
            g_signal_connect (stage,
                              "allocation-changed",
                              G_CALLBACK(on_allocation_changed),
                              NULL);
#else
            g_return_if_reached ();
#endif
        }
        gtk_widget_set_size_request (context->widget,
                                     bounds.width,
                                     bounds.height);
        gtk_window_move (GTK_WINDOW(context->window),
                         MAX(rect.width - 20 - bounds.width, 0),
                         MAX(rect.height - 40 - bounds.height, 0));
        g_signal_connect_after (context->window, "realize",
                                G_CALLBACK(on_realize_set_non_maximizable),
                                context);
    }
}

static void
update_widget (ServerContext *context)
{
    EekBounds bounds;
    EekTheme *theme;
#if HAVE_CLUTTER_GTK
    ClutterActor *stage, *actor;
    ClutterColor stage_color = { 0xff, 0xff, 0xff, 0xff };
#endif

    if (context->widget)
        gtk_widget_destroy (context->widget);

    theme = eek_theme_new (DEFAULT_THEME, NULL, NULL);
    eek_element_get_bounds (EEK_ELEMENT(context->keyboard), &bounds);
    if (context->ui_toolkit == UI_TOOLKIT_CLUTTER) {
#if HAVE_CLUTTER_GTK
        context->widget = gtk_clutter_embed_new ();
        stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED(context->widget));
        actor = eek_clutter_keyboard_new (context->keyboard);
        clutter_actor_set_name (actor, "keyboard");
        if (theme)
            eek_clutter_keyboard_set_theme (EEK_CLUTTER_KEYBOARD(actor), theme);
        clutter_container_add_actor (CLUTTER_CONTAINER(stage), actor);

        clutter_stage_set_color (CLUTTER_STAGE(stage), &stage_color);
#else
        g_return_if_reached ();
#endif
    } else {
        context->widget = eek_gtk_keyboard_new (context->keyboard);
        if (theme)
            eek_gtk_keyboard_set_theme (EEK_GTK_KEYBOARD(context->widget),
                                        theme);
    }

    if (!context->window) {
        context->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        g_signal_connect (context->window, "destroy",
                          G_CALLBACK(on_destroy), context);
        context->notify_visible_handler =
            g_signal_connect (context->window, "notify::visible",
                              G_CALLBACK(on_notify_visible), context);

        gtk_widget_set_can_focus (context->window, FALSE);
        g_object_set (G_OBJECT(context->window), "accept_focus", FALSE, NULL);
        gtk_window_set_title (GTK_WINDOW(context->window),
                              context->client_name ?
                              context->client_name :
                              _("Keyboard"));
        gtk_window_set_icon_name (GTK_WINDOW(context->window), "eekboard");
        gtk_window_set_keep_above (GTK_WINDOW(context->window), TRUE);
    }
    gtk_container_add (GTK_CONTAINER(context->window), context->widget);
    set_geometry (context);
}

static void
server_context_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    ServerContext *context = SERVER_CONTEXT(object);
    GDBusConnection *connection;
    const gchar *ui_toolkit;

    switch (prop_id) {
    case PROP_OBJECT_PATH:
        if (context->object_path)
            g_free (context->object_path);
        context->object_path = g_strdup (g_value_get_string (value));
        break;
    case PROP_CONNECTION:
        connection = g_value_get_object (value);
        if (context->connection)
            g_object_unref (context->connection);
        context->connection = g_object_ref (connection);
        break;
    case PROP_UI_TOOLKIT:
        ui_toolkit = g_value_get_string (value);
        if (g_strcmp0 (ui_toolkit, "gtk") == 0)
            context->ui_toolkit = UI_TOOLKIT_GTK;
#if HAVE_CLUTTER_GTK
        else if (g_strcmp0 (ui_toolkit, "clutter") == 0)
            context->ui_toolkit = UI_TOOLKIT_CLUTTER;
#endif  /* HAVE_CLUTTER_GTK */
        else
            g_warning ("unknown UI toolkit %s", ui_toolkit);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
server_context_dispose (GObject *object)
{
    ServerContext *context = SERVER_CONTEXT(object);

    if (context->keyboard) {
        disconnect_keyboard_signals (context);
        context->keyboard = NULL;
    }

    if (context->keyboard_hash) {
        g_hash_table_destroy (context->keyboard_hash);
        context->keyboard_hash = NULL;
    }

    if (context->window) {
        gtk_widget_destroy (context->window);
        context->window = NULL;
    }

    if (context->connection) {
        if (context->registration_id > 0) {
            g_dbus_connection_unregister_object (context->connection,
                                                 context->registration_id);
            context->registration_id = 0;
        }

        g_object_unref (context->connection);
        context->connection = NULL;
    }

    if (context->introspection_data) {
        g_dbus_node_info_unref (context->introspection_data);
        context->introspection_data = NULL;
    }

    if (context->settings) {
        g_object_unref (context->settings);
        context->settings = NULL;
    }

    G_OBJECT_CLASS (server_context_parent_class)->dispose (object);
}

static void
server_context_finalize (GObject *object)
{
    ServerContext *context = SERVER_CONTEXT(object);

    g_free (context->object_path);
    g_free (context->client_connection);
    g_free (context->client_name);

    G_OBJECT_CLASS (server_context_parent_class)->finalize (object);
}

static void
server_context_constructed (GObject *object)
{
    ServerContext *context = SERVER_CONTEXT (object);
    if (context->connection && context->object_path) {
        GError *error = NULL;

        context->registration_id = g_dbus_connection_register_object
            (context->connection,
             context->object_path,
             context->introspection_data->interfaces[0],
             &interface_vtable,
             context,
             NULL,
             &error);
    }
}

static void
server_context_class_init (ServerContextClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    gobject_class->constructed = server_context_constructed;
    gobject_class->set_property = server_context_set_property;
    gobject_class->dispose = server_context_dispose;
    gobject_class->finalize = server_context_finalize;

    pspec = g_param_spec_string ("object-path",
                                 "Object-path",
                                 "Object-path",
                                 NULL,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_OBJECT_PATH,
                                     pspec);

    pspec = g_param_spec_object ("connection",
                                 "Connection",
                                 "Connection",
                                 G_TYPE_DBUS_CONNECTION,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_CONNECTION,
                                     pspec);

    pspec = g_param_spec_string ("ui-toolkit",
                                 "UI toolkit",
                                 "UI toolkit",
                                 NULL,
                                 G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_UI_TOOLKIT,
                                     pspec);
}

static void
on_monitors_changed (GdkScreen *screen,
                     gpointer   user_data)
{
    ServerContext *context = user_data;
    if (context->window)
        set_geometry (context);
}

static void
server_context_init (ServerContext *context)
{
    GdkScreen *screen;
    GError *error;

    error = NULL;
    context->introspection_data =
        g_dbus_node_info_new_for_xml (introspection_xml, &error);
    g_assert (context->introspection_data != NULL);

    context->keyboard_hash =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               (GDestroyNotify)g_object_unref);

    context->ui_toolkit = UI_TOOLKIT_DEFAULT;

    context->settings = g_settings_new ("org.fedorahosted.eekboard");
    g_settings_bind (context->settings, "ui-toolkit",
                     context, "ui-toolkit",
                     G_SETTINGS_BIND_GET);

    screen = gdk_screen_get_default ();
    g_signal_connect (screen,
                      "monitors-changed",
                      G_CALLBACK(on_monitors_changed),
                      context);
}

static gboolean on_repeat_timeout (ServerContext *context);

static void
emit_key_pressed_dbus_signal (ServerContext *context, EekKey *key)
{
    if (context->connection && context->enabled) {
        const gchar *keyname = eek_element_get_name (EEK_ELEMENT(key));
        EekSymbol *symbol = eek_key_get_symbol_with_fallback (key, 0, 0);
        guint modifiers = eek_keyboard_get_modifiers (context->keyboard);
        GVariant *variant;
        GError *error;

        variant = eek_serializable_serialize (EEK_SERIALIZABLE(symbol));

        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "KeyPressed",
                                       g_variant_new ("(svu)",
                                                      keyname,
                                                      variant,
                                                      modifiers),
                                       &error);
        g_variant_unref (variant);
        g_assert_no_error (error);
    }
}

static gboolean
on_repeat_timeout (ServerContext *context)
{
    gint delay = g_settings_get_int (context->settings, "repeat-interval");

    emit_key_pressed_dbus_signal (context, context->repeat_key);

    context->repeat_timeout_id =
        g_timeout_add (delay,
                       (GSourceFunc)on_repeat_timeout,
                       context);

    return FALSE;
}

static gboolean
on_repeat_timeout_init (ServerContext *context)
{
    emit_key_pressed_dbus_signal (context, context->repeat_key);

    /* FIXME: clear modifiers for further key repeat; better not
       depend on modifier behavior is LATCH */
    eek_keyboard_set_modifiers (context->keyboard, 0);
    
    /* reschedule repeat timeout only when "repeat" option is set */
    if (g_settings_get_boolean (context->settings, "repeat")) {
        gint delay = g_settings_get_int (context->settings, "repeat-interval");
        context->repeat_timeout_id =
            g_timeout_add (delay,
                           (GSourceFunc)on_repeat_timeout,
                           context);
    } else
        context->repeat_timeout_id = 0;

    return FALSE;
}

static void
on_key_pressed (EekKeyboard *keyboard,
                EekKey      *key,
                gpointer     user_data)
{
    ServerContext *context = user_data;
    gint delay = g_settings_get_int (context->settings, "repeat-delay");

    if (context->repeat_timeout_id) {
        g_source_remove (context->repeat_timeout_id);
        context->repeat_timeout_id = 0;
    }

    context->repeat_key = key;
    context->repeat_timeout_id =
        g_timeout_add (delay,
                       (GSourceFunc)on_repeat_timeout_init,
                       context);
}

static void
on_key_released (EekKeyboard *keyboard,
                 EekKey      *key,
                 gpointer     user_data)
{
    ServerContext *context = user_data;

    if (context->repeat_timeout_id > 0) {
        g_source_remove (context->repeat_timeout_id);
        context->repeat_timeout_id = 0;

        /* KeyPressed signal has not been emitted in repeat handler */
        emit_key_pressed_dbus_signal (context, context->repeat_key);
    }
}

static void
disconnect_keyboard_signals (ServerContext *context)
{
    if (g_signal_handler_is_connected (context->keyboard,
                                       context->key_pressed_handler))
        g_signal_handler_disconnect (context->keyboard,
                                     context->key_pressed_handler);
    if (g_signal_handler_is_connected (context->keyboard,
                                       context->key_released_handler))
        g_signal_handler_disconnect (context->keyboard,
                                     context->key_released_handler);
}

static EekKeyboard *
create_keyboard_from_string (const gchar *string)
{
    EekKeyboard *keyboard;
    EekLayout *layout;

    if (g_str_has_prefix (string, "xkb:")) {
        XklConfigRec *rec = eekboard_xkl_config_rec_from_string (&string[4]);

        layout = eek_xkl_layout_new ();
        if (!eek_xkl_layout_set_config (EEK_XKL_LAYOUT(layout), rec)) {
            g_object_unref (layout);
            return NULL;
        }
    } else {
        gchar *path;
        GFile *file;
        GFileInputStream *input;
        GError *error;

        path = g_strdup_printf ("%s/%s.xml", KEYBOARDDIR, string);
        file = g_file_new_for_path (path);
        g_free (path);

        error = NULL;
        input = g_file_read (file, NULL, &error);
        if (input == NULL) {
            g_object_unref (file);
            return NULL;
        }
        layout = eek_xml_layout_new (G_INPUT_STREAM(input));
    }
    keyboard = eek_keyboard_new (layout, CSW, CSH);
    g_object_unref (layout);

    return keyboard;
}

static void
emit_group_changed_signal (ServerContext *context, int group)
{
    GError *error;

    error = NULL;
    g_dbus_connection_emit_signal (context->connection,
                                   NULL,
                                   context->object_path,
                                   SERVER_CONTEXT_INTERFACE,
                                   "GroupChanged",
                                   g_variant_new ("(i)", group),
                                   &error);
    g_assert_no_error (error);
}

static void
handle_method_call (GDBusConnection       *connection,
                    const gchar           *sender,
                    const gchar           *object_path,
                    const gchar           *interface_name,
                    const gchar           *method_name,
                    GVariant              *parameters,
                    GDBusMethodInvocation *invocation,
                    gpointer               user_data)
{
    ServerContext *context = user_data;

    if (g_strcmp0 (method_name, "AddKeyboard") == 0) {
        const gchar *name;
        static guint keyboard_id = 0;
        EekKeyboard *keyboard;

        g_variant_get (parameters, "(&s)", &name);
        keyboard = create_keyboard_from_string (name);

        if (keyboard == NULL) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "can't create a keyboard");
            return;
        }

        eek_keyboard_set_modifier_behavior (keyboard,
                                            EEK_MODIFIER_BEHAVIOR_LATCH);

        g_hash_table_insert (context->keyboard_hash,
                             GUINT_TO_POINTER(++keyboard_id),
                             keyboard);
        g_dbus_method_invocation_return_value (invocation,
                                               g_variant_new ("(u)",
                                                              keyboard_id));
        return;
    }

    if (g_strcmp0 (method_name, "RemoveKeyboard") == 0) {
        guint keyboard_id;

        g_variant_get (parameters, "(u)", &keyboard_id);

        if (keyboard_id == context->keyboard_id) {
            disconnect_keyboard_signals (context);
            if (context->window) {
                gtk_widget_hide (context->window);
                gtk_widget_destroy (context->widget);
            }

            context->keyboard = NULL;
        }

        g_hash_table_remove (context->keyboard_hash,
                             GUINT_TO_POINTER(keyboard_id));
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "SetKeyboard") == 0) {
        EekKeyboard *keyboard;
        guint keyboard_id;
        gint group;

        g_variant_get (parameters, "(u)", &keyboard_id);

        keyboard = g_hash_table_lookup (context->keyboard_hash,
                                        GUINT_TO_POINTER(keyboard_id));
        if (!keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "no such keyboard");
            return;
        }

        if (keyboard == context->keyboard) {
            g_dbus_method_invocation_return_value (invocation, NULL);
            return;
        }

        if (context->keyboard)
            disconnect_keyboard_signals (context);

        context->keyboard = keyboard;

        context->key_pressed_handler =
            g_signal_connect (context->keyboard, "key-pressed",
                              G_CALLBACK(on_key_pressed),
                              context);
        context->key_released_handler =
            g_signal_connect (context->keyboard, "key-released",
                              G_CALLBACK(on_key_released),
                              context);
        
        if (context->window) {
            gboolean was_visible = gtk_widget_get_visible (context->window);
            /* avoid to send KeyboardVisibilityChanged */
            g_signal_handler_block (context->window,
                                    context->notify_visible_handler);
            update_widget (context);
            if (was_visible)
                gtk_widget_show_all (context->window);
            g_signal_handler_unblock (context->window,
                                      context->notify_visible_handler);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);

        group = eek_element_get_group (EEK_ELEMENT(context->keyboard));
        emit_group_changed_signal (context, group);

        return;
    }

    if (g_strcmp0 (method_name, "SetFullscreen") == 0) {
        gboolean fullscreen;

        g_variant_get (parameters, "(b)", &fullscreen);

        if (context->fullscreen == fullscreen) {
            g_dbus_method_invocation_return_value (invocation, NULL);
            return;
        }
        context->fullscreen = fullscreen;
        if (context->window)
            set_geometry (context);

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "SetGroup") == 0) {
        gint group;

        if (!context->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        g_variant_get (parameters, "(i)", &group);
        eek_element_set_group (EEK_ELEMENT(context->keyboard), group);

        if (context->window) {
            gboolean was_visible = gtk_widget_get_visible (context->window);

            /* avoid to send KeyboardVisibilityChanged */
            g_signal_handler_block (context->window,
                                    context->notify_visible_handler);
            update_widget (context);
            if (was_visible)
                gtk_widget_show_all (context->window);
            g_signal_handler_unblock (context->window,
                                      context->notify_visible_handler);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);

        emit_group_changed_signal (context, group);

        return;
    }

    if (g_strcmp0 (method_name, "ShowKeyboard") == 0) {
        if (!context->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        if (!context->window)
            update_widget (context);
        g_assert (context->window);
        gtk_widget_show_all (context->window);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "HideKeyboard") == 0) {
        if (context->window)
            gtk_widget_hide (context->window);
        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    if (g_strcmp0 (method_name, "PressKey") == 0 ||
        g_strcmp0 (method_name, "ReleaseKey") == 0) {
        EekKey *key;
        guint keycode;

        if (!context->keyboard) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "keyboard is not set");
            return;
        }

        g_variant_get (parameters, "(u)", &keycode);
        key = eek_keyboard_find_key_by_keycode (context->keyboard, keycode);

        if (!key) {
            g_dbus_method_invocation_return_error (invocation,
                                                   G_IO_ERROR,
                                                   G_IO_ERROR_FAILED_HANDLED,
                                                   "key for %u is not found",
                                                   keycode);
            return;
        }

        if (g_strcmp0 (method_name, "PressKey") == 0) {
            g_signal_handler_block (context->keyboard,
                                    context->key_pressed_handler);
            g_signal_emit_by_name (key, "pressed");
            g_signal_handler_unblock (context->keyboard,
                                      context->key_pressed_handler);
        } else {
            g_signal_handler_block (context->keyboard,
                                    context->key_released_handler);
            g_signal_emit_by_name (key, "released");
            g_signal_handler_unblock (context->keyboard,
                                      context->key_released_handler);
        }

        g_dbus_method_invocation_return_value (invocation, NULL);
        return;
    }

    g_return_if_reached ();
}

ServerContext *
server_context_new (const gchar     *object_path,
                    GDBusConnection *connection)
{
    return g_object_new (SERVER_TYPE_CONTEXT,
                         "object-path", object_path,
                         "connection", connection,
                         NULL);
}

void
server_context_set_enabled (ServerContext *context, gboolean enabled)
{
    GError *error;

    g_return_if_fail (SERVER_IS_CONTEXT(context));
    g_return_if_fail (context->connection);

    if (context->enabled == enabled)
        return;

    context->enabled = enabled;
    if (enabled) {
        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "Enabled",
                                       NULL,
                                       &error);
        g_assert_no_error (error);
        if (context->last_keyboard_visible && context->window)
            gtk_widget_show_all (context->window);
    } else {
        error = NULL;
        g_dbus_connection_emit_signal (context->connection,
                                       NULL,
                                       context->object_path,
                                       SERVER_CONTEXT_INTERFACE,
                                       "Disabled",
                                       NULL,
                                       &error);
        g_assert_no_error (error);
        if (context->window) {
            context->last_keyboard_visible =
                gtk_widget_get_visible (context->window);
            gtk_widget_hide (context->window);
        }
    }
}

void
server_context_set_client_connection (ServerContext *context,
                                      const gchar   *client_connection)
{
    g_free (context->client_connection);
    context->client_connection = g_strdup (client_connection);
}

const gchar *
server_context_get_client_connection (ServerContext *context)
{
    return context->client_connection;
}

void
server_context_set_client_name (ServerContext *context,
                                const gchar   *client_name)
{
    g_free (context->client_name);
    context->client_name = g_strdup (client_name);
}
