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
 * @short_description: a #ClutterActor displaying #EekKeyboard
 */
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-clutter-keyboard.h"
#include "eek-clutter-section.h"
#include "eek-clutter-renderer.h"
#include "eek-keyboard.h"

G_DEFINE_TYPE (EekClutterKeyboard, eek_clutter_keyboard, CLUTTER_TYPE_GROUP);

#define EEK_CLUTTER_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_KEYBOARD, EekClutterKeyboardPrivate))

struct _EekClutterKeyboardPrivate
{
    EekKeyboard *keyboard;
    EekClutterRenderer *renderer;
};

static void
eek_clutter_keyboard_real_get_preferred_width (ClutterActor *self,
                                               float         for_height,
                                               float        *min_width_p,
                                               float        *natural_width_p)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);
    gdouble width;

    eek_renderer_get_size (EEK_RENDERER(priv->renderer), &width, NULL);
    *min_width_p = 0.0f;
    *natural_width_p = width;
}

static void
eek_clutter_keyboard_real_get_preferred_height (ClutterActor *self,
                                                float         for_width,
                                                float        *min_height_p,
                                                float        *natural_height_p)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);
    gdouble height;

    eek_renderer_get_size (EEK_RENDERER(priv->renderer), NULL, &height);
    *min_height_p = 0.0f;
    *natural_height_p = height;
}

static void
eek_clutter_keyboard_real_allocate (ClutterActor          *self,
                                    const ClutterActorBox *box,
                                    ClutterAllocationFlags flags)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);

    g_assert (priv->renderer);
    eek_renderer_set_allocation_size (EEK_RENDERER(priv->renderer),
                                      box->x2 - box->x1,
                                      box->y2 - box->y1);

    CLUTTER_ACTOR_CLASS (eek_clutter_keyboard_parent_class)->
        allocate (self, box, flags);
}

static void
eek_clutter_keyboard_dispose (GObject *object)
{
    EekClutterKeyboardPrivate *priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(object);

    if (priv->renderer) {
        g_object_unref (G_OBJECT(priv->renderer));
        priv->renderer = NULL;
    }

    if (priv->keyboard && g_object_is_floating (priv->keyboard)) {
        g_object_unref (priv->keyboard);
        priv->keyboard = NULL;
    }

    G_OBJECT_CLASS (eek_clutter_keyboard_parent_class)->dispose (object);
}

static void
eek_clutter_keyboard_class_init (EekClutterKeyboardClass *klass)
{
    ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekClutterKeyboardPrivate));

    actor_class->get_preferred_width =
        eek_clutter_keyboard_real_get_preferred_width;
    actor_class->get_preferred_height =
        eek_clutter_keyboard_real_get_preferred_height;
    actor_class->allocate = eek_clutter_keyboard_real_allocate;

    gobject_class->dispose = eek_clutter_keyboard_dispose;
}

static void
eek_clutter_keyboard_init (EekClutterKeyboard *self)
{
    EekClutterKeyboardPrivate *priv;

    priv = self->priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(self);
    priv->keyboard = NULL;
    priv->renderer = NULL;
}

struct _CreateSectionCallbackData {
    ClutterActor *actor;
    EekClutterRenderer *renderer;
};
typedef struct _CreateSectionCallbackData CreateSectionCallbackData;

static void
create_section (EekElement *element, gpointer user_data)
{
    CreateSectionCallbackData *data = user_data;
    ClutterActor *section;
    
    section = eek_clutter_section_new (EEK_SECTION(element), data->renderer);
    clutter_container_add_actor (CLUTTER_CONTAINER(data->actor), section);
}

/**
 * eek_clutter_keyboard_new:
 * @keyboard: an #EekKeyboard
 *
 * Create a new #ClutterActor displaying @keyboard.
 * Returns: a #ClutterActor
 */
ClutterActor *
eek_clutter_keyboard_new (EekKeyboard *keyboard)
{
    ClutterActor *actor;
    EekClutterKeyboardPrivate *priv;
    PangoContext *pcontext;
    CreateSectionCallbackData data;
    EekBounds bounds;
    gdouble scale;
    PangoFontDescription *font;

    actor = g_object_new (EEK_TYPE_CLUTTER_KEYBOARD, NULL);
    priv = EEK_CLUTTER_KEYBOARD_GET_PRIVATE(actor);
    priv->keyboard = g_object_ref_sink (keyboard);

    pcontext = clutter_actor_get_pango_context (actor);
    font = pango_font_description_from_string ("Sans 48px");
    pango_context_set_font_description (pcontext, font);
    pango_font_description_free (font);

    priv->renderer = eek_clutter_renderer_new (priv->keyboard, pcontext);

    eek_element_get_bounds (EEK_ELEMENT(priv->keyboard), &bounds);
    eek_renderer_set_allocation_size (EEK_RENDERER(priv->renderer),
                                      bounds.width,
                                      bounds.height);
    scale = eek_renderer_get_scale (EEK_RENDERER(priv->renderer));
    clutter_actor_set_position (actor, bounds.x * scale, bounds.y * scale);

    data.actor = actor;
    data.renderer = priv->renderer;

    eek_container_foreach_child (EEK_CONTAINER(priv->keyboard),
                                 create_section,
                                 &data);

    return actor;
}
