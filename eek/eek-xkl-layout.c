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
    PROP_MODEL,
    PROP_LAYOUTS,
    PROP_VARIANTS,
    PROP_OPTIONS,
    PROP_LAST
};

struct _EekXklLayoutPrivate
{
    XklEngine *engine;
    XklConfigRec *config;
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

static gboolean set_xkb_component_names (EekXklLayout *layout,
                                         XklConfigRec *config);

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
eek_xkl_layout_dispose (GObject *object)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (object);

    if (priv->config) {
        g_object_unref (priv->config);
        priv->config = NULL;
    }
    if (priv->engine) {
        g_object_unref (priv->engine);
        priv->engine = NULL;
    }
    G_OBJECT_CLASS (eek_xkl_layout_parent_class)->dispose (object);
}

static void 
eek_xkl_layout_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    switch (prop_id) 
        {
        case PROP_MODEL:
            eek_xkl_layout_set_model (EEK_XKL_LAYOUT(object),
                                      g_value_get_string (value));
            break;
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
        case PROP_MODEL:
            g_value_set_string
                (value,
                 eek_xkl_layout_get_model (EEK_XKL_LAYOUT(object)));
            break;
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

    gobject_class->dispose = eek_xkl_layout_dispose;
    gobject_class->set_property = eek_xkl_layout_set_property;
    gobject_class->get_property = eek_xkl_layout_get_property;

    /**
     * EekXklLayout:model:
     *
     * The libxklavier model name of #EekXklLayout.
     */
    pspec = g_param_spec_string ("model",
                                 "Model",
                                 "Libxklavier model",
                                 NULL,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_MODEL, pspec);

    /**
     * EekXklLayout:layouts:
     *
     * The libxklavier layout names of #EekXklLayout.
     */
    pspec = g_param_spec_boxed ("layouts",
                                "Layouts",
                                "Libxklavier layouts",
                                G_TYPE_STRV,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_LAYOUTS, pspec);

    /**
     * EekXklLayout:variants:
     *
     * The libxklavier variant names of #EekXklLayout.
     */
    pspec = g_param_spec_boxed ("variants",
                                "Variants",
                                "Libxklavier variants",
                                G_TYPE_STRV,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_VARIANTS, pspec);

    /**
     * EekXklLayout:options:
     *
     * The libxklavier option names of #EekXklLayout.
     */
    pspec = g_param_spec_boxed ("options",
                                "Options",
                                "Libxklavier options",
                                G_TYPE_STRV,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_OPTIONS, pspec);
}

/* Disabled since the current EekXklLayout implementation does not
   change the server setting. */
#if 0
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
#endif

static void
eek_xkl_layout_init (EekXklLayout *self)
{
    EekXklLayoutPrivate *priv;
    Display *display;

    priv = self->priv = EEK_XKL_LAYOUT_GET_PRIVATE (self);
    priv->config = xkl_config_rec_new ();

    display = GDK_DISPLAY_XDISPLAY (gdk_display_get_default ());
    g_return_if_fail (display);

    priv->engine = xkl_engine_get_instance (display);
    /* Disabled since the current EekXklLayout implementation does not
       change the server setting. */
#if 0
    g_signal_connect (priv->engine, "X-state-changed",
                      G_CALLBACK(on_state_changed), self);
    xkl_engine_start_listen (priv->engine, XKLL_TRACK_KEYBOARD_STATE);
#endif
    xkl_config_rec_get_from_server (priv->config, priv->engine);
    set_xkb_component_names (self, priv->config);
}

/**
 * eek_xkl_layout_new:
 *
 * Create a new #EekXklLayout.
 */
EekLayout *
eek_xkl_layout_new (void)
{
    return g_object_new (EEK_TYPE_XKL_LAYOUT, NULL);
}

G_INLINE_FUNC void
merge_xkl_config_rec (XklConfigRec *dst, XklConfigRec *src)
{
    if (src->model) {
        g_free (dst->model);
        dst->model = g_strdup (src->model);
    }
    if (src->layouts) {
        g_strfreev (dst->layouts);
        dst->layouts = g_strdupv (src->layouts);
    }
    if (src->variants) {
        g_strfreev (dst->variants);
        dst->variants = g_strdupv (src->variants);
    }
    if (src->options) {
        g_strfreev (dst->options);
        dst->options = g_strdupv (src->options);
    }
}

/**
 * eek_xkl_layout_set_config:
 * @layout: an #EekXklLayout
 * @config: Libxklavier configuration
 *
 * Reconfigure @layout with @config.
 * Returns: %TRUE if the component name is successfully set, %FALSE otherwise
 */
gboolean
eek_xkl_layout_set_config (EekXklLayout *layout,
                           XklConfigRec *config)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);
    XklConfigRec *c;

    g_return_val_if_fail (priv, FALSE);
    c = xkl_config_rec_new ();
    merge_xkl_config_rec (c, priv->config);
    merge_xkl_config_rec (c, config);
    if (set_xkb_component_names (layout, c)) {
        g_object_unref (c);
        merge_xkl_config_rec (priv->config, config);
        return TRUE;
    }
    g_object_unref (c);
    return FALSE;
}

/**
 * eek_xkl_layout_set_model:
 * @layout: an #EekXklLayout
 * @model: model name
 *
 * Set the model name of @layout configuration (in the Libxklavier terminology).
 * Returns: %TRUE if the component name is successfully set, %FALSE otherwise
 */
gboolean
eek_xkl_layout_set_model (EekXklLayout *layout,
                          const gchar  *model)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);
    XklConfigRec *config;
    gboolean success;
    
    g_return_val_if_fail (priv, FALSE);
    config = xkl_config_rec_new ();
    config->model = (gchar *)model;
    success = eek_xkl_layout_set_config (layout, config);
    g_object_unref (config);
    return success;
}

/**
 * eek_xkl_layout_set_layouts:
 * @layout: an #EekXklLayout
 * @layouts: layout names
 *
 * Set the layout names of @layout (in the Libxklavier terminology).
 * Returns: %TRUE if the component name is successfully set, %FALSE otherwise
 */
gboolean
eek_xkl_layout_set_layouts (EekXklLayout *layout,
                            gchar       **layouts)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);
    XklConfigRec *config;
    gboolean success;

    g_return_val_if_fail (priv, FALSE);
    config = xkl_config_rec_new ();
    config->layouts = layouts;
    success = eek_xkl_layout_set_config (layout, config);
    g_object_unref (config);
    return success;
}

/**
 * eek_xkl_layout_set_variants:
 * @layout: an #EekXklLayout
 * @variants: variant names
 *
 * Set the variant names of @layout (in the Libxklavier terminology).
 * Returns: %TRUE if the component name is successfully set, %FALSE otherwise
 */
gboolean
eek_xkl_layout_set_variants (EekXklLayout *layout,
                             gchar       **variants)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);
    XklConfigRec *config;
    gboolean success;

    g_return_val_if_fail (priv, FALSE);
    config = xkl_config_rec_new ();
    config->variants = variants;
    success = eek_xkl_layout_set_config (layout, config);
    g_object_unref (config);
    return success;
}

/**
 * eek_xkl_layout_set_options:
 * @layout: an #EekXklLayout
 * @options: option names
 *
 * Set the option names of @layout (in the Libxklavier terminology).
 * Returns: %TRUE if the component name is successfully set, %FALSE otherwise
 */
gboolean
eek_xkl_layout_set_options (EekXklLayout *layout,
                            gchar       **options)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);
    XklConfigRec *config;
    gboolean success;

    g_return_val_if_fail (priv, FALSE);
    config = xkl_config_rec_new ();
    config->options = options;
    success = eek_xkl_layout_set_config (layout, config);
    g_object_unref (config);
    return success;
}

/**
 * eek_xkl_layout_get_model:
 * @layout: an #EekXklLayout
 *
 * Get the model name of @layout configuration (in the Libxklavier terminology).
 */
gchar *
eek_xkl_layout_get_model (EekXklLayout *layout)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_val_if_fail (priv, NULL);
    return priv->config->model;
}

/**
 * eek_xkl_layout_get_layouts:
 * @layout: an #EekXklLayout
 *
 * Get the layout names of @layout configuration (in the Libxklavier
 * terminology).
 */
gchar **
eek_xkl_layout_get_layouts (EekXklLayout *layout)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_val_if_fail (priv, NULL);
    return priv->config->layouts;
}

/**
 * eek_xkl_layout_get_variants:
 * @layout: an #EekXklLayout
 *
 * Get the variant names of @layout configuration (in the Libxklavier
 * terminology).
 */
gchar **
eek_xkl_layout_get_variants (EekXklLayout *layout)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_val_if_fail (priv, NULL);
    return priv->config->variants;
}

/**
 * eek_xkl_layout_get_options:
 * @layout: an #EekXklLayout
 *
 * Get the option names of @layout configuration (in the Libxklavier
 * terminology).
 */
gchar **
eek_xkl_layout_get_options (EekXklLayout *layout)
{
    EekXklLayoutPrivate *priv = EEK_XKL_LAYOUT_GET_PRIVATE (layout);

    g_return_val_if_fail (priv, NULL);
    return priv->config->options;
}

static gboolean
set_xkb_component_names (EekXklLayout *layout, XklConfigRec *config)
{
    EekXklLayoutPrivate *priv = layout->priv;
    XkbComponentNamesRec names;
    gboolean success = FALSE;

#if DEBUG
    if (config->layouts) {
        gint i;

        fprintf (stderr, "layout = ");
        for (i = 0; config->layouts[i] != NULL; i++)
            fprintf (stderr, "\"%s\" ", config->layouts[i]);
        fputc ('\n', stderr);
    } else
        fprintf (stderr, "layouts = NULL\n");
    if (config->variants) {
        gint i;

        fprintf (stderr, "variant = ");
        for (i = 0; config->variants[i]; i++)
            fprintf (stderr, "\"%s\" ", config->variants[i]);
        fputc ('\n', stderr);
    } else
        fprintf (stderr, "variants = NULL\n");
    if (config->options) {
        gint i;

        fprintf (stderr, "option = ");
        for (i = 0; config->options[i]; i++)
            fprintf (stderr, "\"%s\" ", config->options[i]);
        fputc ('\n', stderr);
    } else
        fprintf (stderr, "options = NULL\n");
#endif

    /* Disabled since the current EekXklLayout implementation does not
       change the server setting. */
#if 0
    xkl_config_rec_activate (priv->engine, config);
#endif
    if (xkl_xkb_config_native_prepare (priv->engine, config, &names)) {
        success = eek_xkb_layout_set_names (EEK_XKB_LAYOUT(layout), &names);
        xkl_xkb_config_native_cleanup (priv->engine, &names);
    }
    return success;
}
