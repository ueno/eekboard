/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

/**
 * SECTION:eek-xkl-layout
 * @short_description: Layout engine using Libxklavier configuration
 *
 * The #EekXklLayout is a simple wrapper around #EekXkbLayout class
 * to use Libxklavier configuration.
 */

#include <libxklavier/xklavier.h>
#include <gdk/gdkx.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-xkl-layout.h"

#define noKBDRAW_DEBUG

static void eek_layout_iface_init (EekLayoutIface *iface);
static EekLayoutIface *parent_layout_iface;

G_DEFINE_TYPE_WITH_CODE (EekXklLayout, eek_xkl_layout, EEK_TYPE_XKB_LAYOUT,
                         G_IMPLEMENT_INTERFACE(EEK_TYPE_LAYOUT,
                                               eek_layout_iface_init));

#define EEK_XKL_LAYOUT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_XKL_LAYOUT, EekXklLayoutPrivate))

enum {
    PROP_0,
    PROP_LAYOUTS,
    PROP_VARIANTS,
    PROP_OPTIONS,
    PROP_LAST
};

struct _EekXklLayoutPrivate
{
    XklEngine *engine;
    XklConfigRec config;
};

/* from gnome-keyboard-properties-xkbpv.c:
 *  BAD STYLE: Taken from xklavier_private_xkb.h
 *  Any ideas on architectural improvements are WELCOME
 */
extern gboolean xkl_xkb_config_native_prepare (XklEngine * engine,
					       const XklConfigRec * data,
					       XkbComponentNamesPtr
					       component_names);

extern void xkl_xkb_config_native_cleanup (XklEngine * engine,
					   XkbComponentNamesPtr
					   component_names);

static void get_xkb_component_names (EekXklLayout *layout);

static gint
eek_xkl_layout_real_get_group (EekLayout *self)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (self);
    XklState *state;

    state = xkl_engine_get_current_state (priv->engine);
    g_return_val_if_fail (state, -1);
    return state->group;
}

static void
eek_layout_iface_init (EekLayoutIface *iface)
{
    parent_layout_iface = g_type_interface_peek_parent (iface);
    if (!parent_layout_iface)
        parent_layout_iface = g_type_default_interface_peek (EEK_TYPE_LAYOUT);
    iface->get_group = eek_xkl_layout_real_get_group;
}

static void
eek_xkl_layout_finalize (GObject *object)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (object);

    g_free (priv->config.layouts);
    g_free (priv->config.variants);
    g_free (priv->config.options);
    G_OBJECT_CLASS (eek_xkl_layout_parent_class)->finalize (object);
}

static void 
eek_xkl_layout_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    switch (prop_id) 
        {
        case PROP_LAYOUTS:
            eek_xkl_layout_set_layouts (EEK_XKL_LAYOUT(object),
                                        g_value_get_boxed (value));
            break;
        case PROP_VARIANTS:
            eek_xkl_layout_set_variants (EEK_XKL_LAYOUT(object),
                                         g_value_get_boxed (value));
            break;
        case PROP_OPTIONS:
            eek_xkl_layout_set_options (EEK_XKL_LAYOUT(object),
                                        g_value_get_boxed (value));
            break;
        default:
            g_object_set_property (object,
                                   g_param_spec_get_name (pspec),
                                   value);
            break;
        }
}

static void 
eek_xkl_layout_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    switch (prop_id) 
        {
        case PROP_LAYOUTS:
            g_value_set_boxed
                (value,
                 eek_xkl_layout_get_layouts (EEK_XKL_LAYOUT(object)));
            break;
        case PROP_VARIANTS:
            g_value_set_boxed
                (value,
                 eek_xkl_layout_get_variants (EEK_XKL_LAYOUT(object)));
            break;
        case PROP_OPTIONS:
            g_value_set_boxed
                (value,
                 eek_xkl_layout_get_options (EEK_XKL_LAYOUT(object)));
            break;
        default:
            g_object_get_property (object,
                                   g_param_spec_get_name (pspec),
                                   value);
            break;
        }
}

static void
eek_xkl_layout_class_init (EekXklLayoutClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_type_class_add_private (gobject_class, sizeof (EekXklLayoutPrivate));

    gobject_class->finalize = eek_xkl_layout_finalize;
    gobject_class->set_property = eek_xkl_layout_set_property;
    gobject_class->get_property = eek_xkl_layout_get_property;

    pspec = g_param_spec_boxed ("layouts",
                                "Layouts",
                                "Libxklavier layouts",
                                G_TYPE_STRV,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_LAYOUTS, pspec);

    pspec = g_param_spec_boxed ("variants",
                                "Variants",
                                "Libxklavier variants",
                                G_TYPE_STRV,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_VARIANTS, pspec);

    pspec = g_param_spec_boxed ("options",
                                "Options",
                                "Libxklavier options",
                                G_TYPE_STRV,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_OPTIONS, pspec);
}

static void
on_state_changed (XklEngine           *xklengine,
                  XklEngineStateChange type,
                  gint                 value,
                  gboolean             restore,
                  gpointer             user_data)
{
    EekLayout *layout = user_data;

    if (type == GROUP_CHANGED)
        g_signal_emit_by_name (layout, "group_changed", value);
}

static void
eek_xkl_layout_init (EekXklLayout *self)
{
    EekXklLayoutPrivate *priv;
    Display *display;

    priv = self->priv = EEK_XKL_LAYOUT_GET_PRIVATE (self);
    memset (&priv->config, 0, sizeof priv->config);

    display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    g_return_if_fail (display);

    priv->engine = xkl_engine_get_instance (display);
    g_signal_connect (priv->engine, "X-state-changed",
                      G_CALLBACK(on_state_changed), self);
    xkl_config_rec_get_from_server (&priv->config, priv->engine);
    get_xkb_component_names (self);
    xkl_engine_start_listen (priv->engine, XKLL_TRACK_KEYBOARD_STATE);
}

EekLayout *
eek_xkl_layout_new (void)
{
    return g_object_new (EEK_TYPE_XKL_LAYOUT, NULL);
}

void
eek_xkl_layout_set_config (EekXklLayout *layout,
                           gchar       **layouts,
                           gchar       **variants,
                           gchar       **options)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_if_fail (priv);
    g_strfreev (priv->config.layouts);
    priv->config.layouts = g_strdupv (layouts);
    g_strfreev (priv->config.variants);
    priv->config.variants = g_strdupv (variants);
    g_strfreev (priv->config.options);
    priv->config.options = g_strdupv (options);
    get_xkb_component_names (layout);
}

void
eek_xkl_layout_set_layouts (EekXklLayout *layout, gchar **layouts)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_if_fail (priv);
    g_strfreev (priv->config.layouts);
    priv->config.layouts = g_strdupv (layouts);
    get_xkb_component_names (layout);
}

void
eek_xkl_layout_set_variants (EekXklLayout *layout, gchar **variants)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_if_fail (priv);
    g_strfreev (priv->config.variants);
    priv->config.variants = g_strdupv (variants);
    get_xkb_component_names (layout);
}

void
eek_xkl_layout_set_options (EekXklLayout *layout, gchar **options)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_if_fail (priv);
    g_strfreev (priv->config.options);
    priv->config.options = g_strdupv (options);
    get_xkb_component_names (layout);
}

gchar **
eek_xkl_layout_get_layouts (EekXklLayout *layout)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_val_if_fail (priv, NULL);
    return priv->config.layouts;
}

gchar **
eek_xkl_layout_get_variants (EekXklLayout *layout)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_val_if_fail (priv, NULL);
    return priv->config.variants;
}

gchar **
eek_xkl_layout_get_options (EekXklLayout *layout)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_val_if_fail (priv, NULL);
    return priv->config.options;
}

static void
get_xkb_component_names (EekXklLayout *layout)
{
    EekXklLayoutPrivate *priv = layout->priv;
    XkbComponentNamesRec names;

    if (xkl_xkb_config_native_prepare (priv->engine, &priv->config, &names)) {
        g_debug ("symbols = \"%s\"", names.symbols);
        EEK_XKB_LAYOUT_GET_CLASS (layout)->
            set_names (EEK_XKB_LAYOUT(layout), &names);
        xkl_xkb_config_native_cleanup (priv->engine, &names);
    }
}
