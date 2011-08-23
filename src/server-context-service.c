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

#if HAVE_CLUTTER_GTK
#include <clutter-gtk/clutter-gtk.h>
#include "eek/eek-clutter.h"
#endif
#include "eek/eek-gtk.h"

#include "server-context-service.h"

enum {
    PROP_0,
    PROP_UI_TOOLKIT,
    PROP_LAST
};

typedef enum {
    UI_TOOLKIT_GTK,
    UI_TOOLKIT_CLUTTER,
    UI_TOOLKIT_DEFAULT = UI_TOOLKIT_GTK
} UIToolkitType;

typedef struct _ServerContextServiceClass ServerContextServiceClass;

struct _ServerContextService {
    EekboardContextService parent;

    gboolean was_visible;

    GtkWidget *window;
    GtkWidget *widget;

    gulong notify_visible_handler;

    GSettings *settings;
    UIToolkitType ui_toolkit;
};

struct _ServerContextServiceClass {
    EekboardContextServiceClass parent_class;
};

G_DEFINE_TYPE (ServerContextService, server_context_service, EEKBOARD_TYPE_CONTEXT_SERVICE);

static void update_widget (ServerContextService *context);
static void set_geometry  (ServerContextService *context);

static void
on_monitors_changed (GdkScreen *screen,
                     gpointer   user_data)
{
    ServerContextService *context = user_data;
    if (context->window)
        set_geometry (context);
}

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
    ServerContextService *context = user_data;

    g_assert (widget == context->window);
    context->window = NULL;
    context->widget = NULL;
}

static void
on_notify_keyboard (GObject    *object,
                    GParamSpec *spec,
                    gpointer    user_data)
{
    ServerContextService *context = user_data;
    const EekKeyboard *keyboard;

    keyboard = eekboard_context_service_get_keyboard (EEKBOARD_CONTEXT_SERVICE(context));
    if (context->window) {
        if (keyboard == NULL) {
            gtk_widget_hide (context->window);
            gtk_widget_destroy (context->widget);
        } else {
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
    }
}
        
static void
on_notify_fullscreen (GObject    *object,
                      GParamSpec *spec,
                      gpointer    user_data)
{
    ServerContextService *context = user_data;
    if (context->window)
        set_geometry (context);
}

static void
on_notify_visible (GObject *object, GParamSpec *spec, gpointer user_data)
{
    ServerContextService *context = user_data;
    gboolean visible;

    g_object_get (object, "visible", &visible, NULL);
    g_object_set (context, "visible", visible, NULL);
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
    ServerContextService *context = user_data;

    g_assert (context && context->window == widget);

    /* make the window not maximizable */
    gdk_window_set_functions (gtk_widget_get_window (widget),
                              GDK_FUNC_RESIZE |
                              GDK_FUNC_MOVE |
                              GDK_FUNC_MINIMIZE |
                              GDK_FUNC_CLOSE);
}

static void
set_geometry (ServerContextService *context)
{
    GdkScreen *screen;
    GdkWindow *root;
    gint monitor;
    GdkRectangle rect;
    const EekKeyboard *keyboard;
    EekBounds bounds;

    screen = gdk_screen_get_default ();
    root = gtk_widget_get_root_window (context->window);
    monitor = gdk_screen_get_monitor_at_window (screen, root);
    gdk_screen_get_monitor_geometry (screen, monitor, &rect);
    keyboard = eekboard_context_service_get_keyboard (EEKBOARD_CONTEXT_SERVICE(context));
    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);

    g_signal_handlers_disconnect_by_func (context->window,
                                          on_realize_set_dock,
                                          context);
    g_signal_handlers_disconnect_by_func (context->window,
                                          on_realize_set_non_maximizable,
                                          context);

    if (eekboard_context_service_get_fullscreen (EEKBOARD_CONTEXT_SERVICE(context))) {
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
update_widget (ServerContextService *context)
{
    const EekKeyboard *keyboard;
    const gchar *client_name;
    EekBounds bounds;
    gchar *theme_name, *theme_path;
    EekTheme *theme;
    
#if HAVE_CLUTTER_GTK
    ClutterActor *stage, *actor;
    ClutterColor stage_color = { 0xff, 0xff, 0xff, 0xff };
#endif

    if (context->widget)
        gtk_widget_destroy (context->widget);

    theme_name = g_settings_get_string (context->settings, "theme");
    theme_path = g_strdup_printf ("%s/%s.css", THEMEDIR, theme_name);
    g_free (theme_name);

    theme = eek_theme_new (theme_path, NULL, NULL);
    g_free (theme_path);

    keyboard = eekboard_context_service_get_keyboard (EEKBOARD_CONTEXT_SERVICE(context));
    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);
    if (context->ui_toolkit == UI_TOOLKIT_CLUTTER) {
#if HAVE_CLUTTER_GTK
        context->widget = gtk_clutter_embed_new ();
        stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED(context->widget));
        actor = eek_clutter_keyboard_new (keyboard);
        clutter_actor_set_name (actor, "keyboard");
        eek_clutter_keyboard_set_theme (EEK_CLUTTER_KEYBOARD(actor), theme);
        g_object_unref (theme);
        clutter_container_add_actor (CLUTTER_CONTAINER(stage), actor);

        clutter_stage_set_color (CLUTTER_STAGE(stage), &stage_color);
#else
        g_return_if_reached ();
#endif
    } else {
        context->widget = eek_gtk_keyboard_new (keyboard);
        eek_gtk_keyboard_set_theme (EEK_GTK_KEYBOARD(context->widget), theme);
        g_object_unref (theme);
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
        client_name = eekboard_context_service_get_client_name (EEKBOARD_CONTEXT_SERVICE(context));
        gtk_window_set_title (GTK_WINDOW(context->window),
                              client_name ? client_name : _("Keyboard"));
        gtk_window_set_icon_name (GTK_WINDOW(context->window), "eekboard");
        gtk_window_set_keep_above (GTK_WINDOW(context->window), TRUE);
    }
    gtk_container_add (GTK_CONTAINER(context->window), context->widget);
    set_geometry (context);
}

static void
server_context_service_real_show_keyboard (EekboardContextService *_context)
{
    ServerContextService *context = SERVER_CONTEXT_SERVICE(_context);

    if (!context->window)
        update_widget (context);
    g_assert (context->window);
    gtk_widget_show_all (context->window);
}

static void
server_context_service_real_hide_keyboard (EekboardContextService *_context)
{
    ServerContextService *context = SERVER_CONTEXT_SERVICE(_context);

    if (context->window)
        gtk_widget_hide (context->window);
}

static void
server_context_service_real_enabled (EekboardContextService *_context)
{
    ServerContextService *context = SERVER_CONTEXT_SERVICE(_context);

    if (context->was_visible && context->window)
        gtk_widget_show_all (context->window);
}

static void
server_context_service_real_disabled (EekboardContextService *_context)
{
    ServerContextService *context = SERVER_CONTEXT_SERVICE(_context);

    if (context->window) {
        context->was_visible =
            gtk_widget_get_visible (context->window);
        gtk_widget_hide (context->window);
    }
}

static void
server_context_service_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
    ServerContextService *context = SERVER_CONTEXT_SERVICE(object);
    const gchar *ui_toolkit;

    switch (prop_id) {
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
server_context_service_dispose (GObject *object)
{
    ServerContextService *context = SERVER_CONTEXT_SERVICE(object);

    if (context->window) {
        gtk_widget_destroy (context->window);
        context->window = NULL;
    }

    G_OBJECT_CLASS (server_context_service_parent_class)->dispose (object);
}

static void
server_context_service_class_init (ServerContextServiceClass *klass)
{
    EekboardContextServiceClass *context_class = EEKBOARD_CONTEXT_SERVICE_CLASS(klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    context_class->show_keyboard = server_context_service_real_show_keyboard;
    context_class->hide_keyboard = server_context_service_real_hide_keyboard;
    context_class->enabled = server_context_service_real_enabled;
    context_class->disabled = server_context_service_real_disabled;

    gobject_class->set_property = server_context_service_set_property;
    gobject_class->dispose = server_context_service_dispose;

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
server_context_service_init (ServerContextService *context)
{
    GdkScreen *screen;

    screen = gdk_screen_get_default ();
    g_signal_connect (screen,
                      "monitors-changed",
                      G_CALLBACK(on_monitors_changed),
                      context);
    g_signal_connect (context,
                      "notify::keyboard",
                      G_CALLBACK(on_notify_keyboard),
                      context);
    g_signal_connect (context,
                      "notify::fullscreen",
                      G_CALLBACK(on_notify_fullscreen),
                      context);

    context->settings = g_settings_new ("org.fedorahosted.eekboard");
    g_settings_bind (context->settings, "ui-toolkit",
                     context, "ui-toolkit",
                     G_SETTINGS_BIND_GET);
}

ServerContextService *
server_context_service_new (const gchar     *client_name,
                            const gchar     *object_path,
                            GDBusConnection *connection)
{
    return g_object_new (SERVER_TYPE_CONTEXT_SERVICE,
                         "client-name", client_name,
                         "object-path", object_path,
                         "connection", connection,
                         NULL);
}
