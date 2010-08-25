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
 * SECTION:eek-clutter-keyboard
 * @short_description: #EekKeyboard that can be converted into a #ClutterActor
 */
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-clutter-keyboard.h"
#include "eek-clutter-drawing-context.h"
#include "eek-keyboard.h"
#include "eek-drawing.h"
#include "eek-theme-node.h"

G_DEFINE_TYPE (EekClutterKeyboard, eek_clutter_keyboard, EEK_TYPE_KEYBOARD);

#define EEK_CLUTTER_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_KEYBOARD, EekClutterKeyboardPrivate))


struct _EekClutterKeyboardPrivate
{
    EekClutterDrawingContext *context;
    ClutterActor *actor;
    EekThemeNode *tnode;

    guint key_press_event_handler;
    guint key_release_event_handler;
};

static void
eek_clutter_keyboard_real_set_name (EekElement *self,
                                    const gchar *name)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    EEK_ELEMENT_CLASS (eek_clutter_keyboard_parent_class)->
        set_name (self, name);

    if (priv->actor)
        clutter_actor_set_name (priv->actor, name);
}

static void
eek_clutter_keyboard_real_set_bounds (EekElement *self,
                                     EekBounds  *bounds)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    EEK_ELEMENT_CLASS (eek_clutter_keyboard_parent_class)->
        set_bounds (self, bounds);

    if (priv->actor) {
        clutter_actor_set_position (priv->actor, bounds->x, bounds->y);
        clutter_actor_set_size (priv->actor, bounds->width, bounds->height);
    }
}

static void
key_pressed_event (EekSection  *section,
                   EekKey      *key,
                   EekKeyboard *keyboard)
{
    g_signal_emit_by_name (keyboard, "key-pressed", key);
}

static void
key_released_event (EekSection  *section,
                    EekKey      *key,
                    EekKeyboard *keyboard)
{
    g_signal_emit_by_name (keyboard, "key-released", key);
}

static EekSection *
eek_clutter_keyboard_real_create_section (EekKeyboard *self)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);
    EekSection *section;
    ClutterActor *actor;

    if (!priv->context) {
        priv->context = eek_clutter_drawing_context_new ();
        g_object_ref_sink (G_OBJECT(priv->context));
    }

    section = eek_clutter_section_new (priv->context);
    g_return_val_if_fail (section, NULL);

    if (priv->tnode) {
        EekThemeNode *tnode;

        tnode = eek_theme_node_new (priv->tnode,
                                    eek_theme_node_get_theme (priv->tnode),
                                    NULL,
                                    "section",
                                    "section",
                                    "section",
                                    NULL);
        eek_clutter_section_set_theme_node (section, tnode);
    }

    g_signal_connect (section, "key-pressed",
                      G_CALLBACK(key_pressed_event), self);
    g_signal_connect (section, "key-released",
                      G_CALLBACK(key_released_event), self);

    EEK_CONTAINER_GET_CLASS(self)->add_child (EEK_CONTAINER(self),
                                              EEK_ELEMENT(section));

    actor = eek_clutter_keyboard_get_actor (EEK_CLUTTER_KEYBOARD(self));
    clutter_container_add_actor
        (CLUTTER_CONTAINER(actor),
         eek_clutter_section_get_actor (EEK_CLUTTER_SECTION(section)));

    return section;
}

static void
eek_clutter_keyboard_dispose (GObject *object)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(object);

    if (priv->context) {
        g_object_unref (G_OBJECT(priv->context));
        priv->context = NULL;
    }
    if (priv->actor) {
        ClutterActor *stage;

        stage = clutter_actor_get_stage (priv->actor);
        if (stage) {
            g_signal_handler_disconnect (stage,
                                         priv->key_press_event_handler);
            g_signal_handler_disconnect (stage,
                                         priv->key_release_event_handler);
        }
        g_object_unref (priv->actor);
        priv->actor = NULL;
    }
    if (priv->tnode) {
        g_object_unref (priv->tnode);
        priv->tnode = NULL;
    }
    G_OBJECT_CLASS (eek_clutter_keyboard_parent_class)->dispose (object);
}

static void
eek_clutter_keyboard_class_init (EekClutterKeyboardClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    EekElementClass *element_class = EEK_ELEMENT_CLASS (klass);
    EekKeyboardClass *keyboard_class = EEK_KEYBOARD_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyboardPrivate));

    keyboard_class->create_section = eek_clutter_keyboard_real_create_section;
    element_class->set_name = eek_clutter_keyboard_real_set_name;
    element_class->set_bounds = eek_clutter_keyboard_real_set_bounds;
    gobject_class->dispose = eek_clutter_keyboard_dispose;
}

static void
eek_clutter_keyboard_init (EekClutterKeyboard *self)
{
    EekClutterKeyboardPrivate *priv;

    priv = self->priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);
    priv->actor = NULL;
}

/**
 * eek_clutter_keyboard_new:
 *
 * Create a new #EekClutterKeyboard.
 */
EekKeyboard*
eek_clutter_keyboard_new (void)
{
    return g_object_new (EEK_TYPE_CLUTTER_KEYBOARD, NULL);
}

static gboolean
on_clutter_key_press_event (ClutterActor *actor,
                            ClutterEvent *event,
                            gpointer      user_data)
{
    guint keycode;
    EekKey *key;

    keycode = clutter_event_get_key_code (event);
    key = eek_keyboard_find_key_by_keycode (user_data, keycode);
    if (key) {
        g_signal_emit_by_name (key, "pressed", NULL);
        return TRUE;
    }
    return FALSE;
}

static gboolean
on_clutter_key_release_event (ClutterActor *actor,
                              ClutterEvent *event,
                              gpointer      user_data)
{
    guint keycode;
    EekKey *key;

    keycode = clutter_event_get_key_code (event);
    key = eek_keyboard_find_key_by_keycode (user_data, keycode);
    if (key) {
        g_signal_emit_by_name (key, "released", NULL);
        return TRUE;
    }
    return FALSE;    
}

static void
on_clutter_stage_resize (GObject *object,
                         GParamSpec *param_spec,
                         gpointer user_data)
{
    ClutterActor *stage = CLUTTER_ACTOR(object);
    EekClutterKeyboard *keyboard = user_data;
    GValue value = {0};
    gfloat width, height, scale;
    EekBounds bounds;

    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);
    g_object_get (G_OBJECT(stage), "width", &width, NULL);
    g_object_get (G_OBJECT(stage), "height", &height, NULL);

    g_value_init (&value, G_TYPE_DOUBLE);

    scale = width > height ? width / bounds.width : height / bounds.height;

    g_value_set_double (&value, scale);
    g_object_set_property (G_OBJECT (stage),
                           "scale-x",
                           &value);

    g_value_set_double (&value, scale);
    g_object_set_property (G_OBJECT (stage),
                           "scale-y",
                           &value);
}

static void
on_clutter_realize (ClutterActor *actor,
                    gpointer      user_data)
{
    EekClutterKeyboard *keyboard = user_data;
    EekClutterKeyboardPrivate *priv =
        EEK_CLUTTER_KEYBOARD_GET_PRIVATE(keyboard);
    ClutterActor *stage;

    stage = clutter_actor_get_stage (priv->actor);
    priv->key_press_event_handler =
        g_signal_connect (stage, "key-press-event",
                          G_CALLBACK (on_clutter_key_press_event), keyboard);
    priv->key_release_event_handler =
        g_signal_connect (stage, "key-release-event",
                          G_CALLBACK (on_clutter_key_release_event), keyboard);
    g_signal_connect (stage, "notify::width",
                      G_CALLBACK (on_clutter_stage_resize), keyboard);
    g_signal_connect (stage, "notify::height",
                      G_CALLBACK (on_clutter_stage_resize), keyboard);
}

static void
update_category_fonts (EekClutterKeyboard *keyboard)
{
    EekClutterKeyboardPrivate *priv =
        EEK_CLUTTER_KEYBOARD_GET_PRIVATE(keyboard);
    PangoContext *context;
    PangoLayout *layout;
    PangoFontDescription *fonts[EEK_KEYSYM_CATEGORY_LAST], *base_font;
    gint i;

    context = clutter_actor_get_pango_context (priv->actor);
    layout = pango_layout_new (context);
    base_font = pango_font_description_from_string ("Sans");
    pango_layout_set_font_description (layout, base_font);
    pango_font_description_free (base_font);
    eek_get_fonts (EEK_KEYBOARD(keyboard),
                   layout,
                   (PangoFontDescription **)&fonts);
    for (i = 0; i < EEK_KEYSYM_CATEGORY_LAST; i++) {
        eek_clutter_drawing_context_set_category_font (priv->context,
                                                       i,
                                                       fonts[i]);
        pango_font_description_free (fonts[i]);
    }
    g_object_unref (G_OBJECT(layout));
}

ClutterActor *
eek_clutter_keyboard_get_actor (EekClutterKeyboard *keyboard)
{
    EekClutterKeyboardPrivate *priv =
        EEK_CLUTTER_KEYBOARD_GET_PRIVATE(keyboard);

    g_return_val_if_fail (priv, NULL);
    if (!priv->actor) {
        priv->actor = clutter_group_new ();
        g_object_ref_sink (priv->actor);
        g_signal_connect (priv->actor, "realize",
                          G_CALLBACK (on_clutter_realize), keyboard);
        g_return_val_if_fail (priv->actor, NULL);

        eek_keyboard_realize (EEK_KEYBOARD(keyboard));
        update_category_fonts (keyboard);
    }
    return priv->actor;
}

void
eek_clutter_keyboard_set_theme (EekClutterKeyboard *keyboard,
                                EekTheme           *theme)
{
    EekClutterKeyboardPrivate *priv =
        EEK_CLUTTER_KEYBOARD_GET_PRIVATE(keyboard);

    g_return_if_fail (priv);
    if (priv->tnode)
        g_object_unref (priv->tnode);
    priv->tnode = eek_theme_node_new (NULL,
                                      theme,
                                      NULL,
                                      "keyboard",
                                      "keyboard",
                                      "keyboard",
                                      NULL);
}
