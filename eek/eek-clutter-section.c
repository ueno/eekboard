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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-clutter-section.h"
#include "eek-clutter-key.h"

enum {
    PROP_0,
    PROP_SECTION,
    PROP_RENDERER,
    PROP_LAST
};

G_DEFINE_TYPE (EekClutterSection, eek_clutter_section, CLUTTER_TYPE_GROUP);

#define EEK_CLUTTER_SECTION_GET_PRIVATE(obj)                           \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CLUTTER_SECTION, EekClutterSectionPrivate))

struct _EekClutterSectionPrivate
{
    EekSection *section;
    EekClutterRenderer *renderer;
};

struct _CreateKeyCallbackData {
    ClutterActor *actor;
    EekClutterRenderer *renderer;
};
typedef struct _CreateKeyCallbackData CreateKeyCallbackData;

static void
create_key (EekElement *element, gpointer user_data)
{
    CreateKeyCallbackData *data = user_data;
    ClutterActor *key;

    key = eek_clutter_key_new (EEK_KEY(element), data->renderer);
    clutter_container_add_actor (CLUTTER_CONTAINER(data->actor), key);
}

static void
eek_clutter_section_real_realize (ClutterActor *self)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);
    CreateKeyCallbackData data;
    EekBounds bounds;
    gdouble scale;

    eek_element_get_bounds (EEK_ELEMENT(priv->section), &bounds);
    scale = eek_renderer_get_scale (EEK_RENDERER(priv->renderer));
    clutter_actor_set_position (self, bounds.x * scale, bounds.y * scale);
    clutter_actor_set_rotation (self,
                                CLUTTER_Z_AXIS,
                                eek_section_get_angle (priv->section),
                                0.0f, 0.0f, 0.0f);

    data.actor = self;
    data.renderer = priv->renderer;
    eek_container_foreach_child (EEK_CONTAINER(priv->section),
                                 create_key,
                                 &data);
}

static void
eek_clutter_section_real_get_preferred_width (ClutterActor *self,
                                              float         for_height,
                                              float        *min_width_p,
                                              float        *natural_width_p)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);
    EekBounds bounds;
    gdouble scale;

    scale = eek_renderer_get_scale (EEK_RENDERER(priv->renderer));
    eek_element_get_bounds (EEK_ELEMENT(priv->section), &bounds);
    *min_width_p = 0.0f;
    *natural_width_p = bounds.width * scale;
}

static void
eek_clutter_section_real_get_preferred_height (ClutterActor *self,
                                               float         for_width,
                                               float        *min_height_p,
                                               float        *natural_height_p)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(self);
    EekBounds bounds;
    gdouble scale;

    scale = eek_renderer_get_scale (EEK_RENDERER(priv->renderer));
    eek_element_get_bounds (EEK_ELEMENT(priv->section), &bounds);
    *min_height_p = 0.0f;
    *natural_height_p = bounds.height * scale;
}

static void
eek_clutter_section_real_allocate (ClutterActor          *self,
                                   const ClutterActorBox *box,
                                   ClutterAllocationFlags flags)
{
    CLUTTER_ACTOR_CLASS (eek_clutter_section_parent_class)->
        allocate (self, box, flags);
}

static void
eek_clutter_section_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_SECTION:
        priv->section = g_value_get_object (value);
        g_object_ref_sink (priv->section);
        break;
    case PROP_RENDERER:
        priv->renderer = g_value_get_object (value);
        g_object_ref (priv->renderer);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_clutter_section_dispose (GObject *object)
{
    EekClutterSectionPrivate *priv = EEK_CLUTTER_SECTION_GET_PRIVATE(object);

    if (priv->renderer) {
        g_object_unref (priv->renderer);
        priv->renderer = NULL;
    }

    if (priv->section && g_object_is_floating (priv->section)) {
        g_object_unref (priv->section);
        priv->section = NULL;
    }

    G_OBJECT_CLASS (eek_clutter_section_parent_class)->dispose (object);
}

static void
eek_clutter_section_class_init (EekClutterSectionClass *klass)
{
    ClutterActorClass *actor_class = CLUTTER_ACTOR_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_type_class_add_private (gobject_class, sizeof (EekClutterSectionPrivate));

    actor_class->realize =
        eek_clutter_section_real_realize;
    actor_class->get_preferred_width =
        eek_clutter_section_real_get_preferred_width;
    actor_class->get_preferred_height =
        eek_clutter_section_real_get_preferred_height;
    actor_class->allocate = eek_clutter_section_real_allocate;

    gobject_class->set_property = eek_clutter_section_set_property;
    gobject_class->dispose = eek_clutter_section_dispose;

    pspec = g_param_spec_object ("section",
                                 "Section",
                                 "Section",
                                 EEK_TYPE_SECTION,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_SECTION,
                                     pspec);

    pspec = g_param_spec_object ("renderer",
                                 "Renderer",
                                 "Renderer",
                                 EEK_TYPE_RENDERER,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE);
    g_object_class_install_property (gobject_class,
                                     PROP_RENDERER,
                                     pspec);
}

static void
eek_clutter_section_init (EekClutterSection *self)
{
    EekClutterSectionPrivate *priv;
    priv = self->priv = EEK_CLUTTER_SECTION_GET_PRIVATE (self);
    priv->section = NULL;
    priv->renderer = NULL;
}

ClutterActor *
eek_clutter_section_new (EekSection         *section,
                         EekClutterRenderer *renderer)
{
    return g_object_new (EEK_TYPE_CLUTTER_SECTION,
                         "section", section,
                         "renderer", renderer,
                         NULL);
}
