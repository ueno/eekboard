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
#ifndef EEK_CLUTTER_KEY_ACTOR_H
#define EEK_CLUTTER_KEY_ACTOR_H 1

#include <clutter/clutter.h>
#include "eek-key.h"

G_BEGIN_DECLS
#define EEK_TYPE_CLUTTER_KEY_ACTOR (eek_clutter_key_actor_get_type())
#define EEK_CLUTTER_KEY_ACTOR(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_CLUTTER_KEY_ACTOR, EekClutterKeyActor))
#define EEK_CLUTTER_KEY_ACTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_CLUTTER_KEY_ACTOR, EekClutterKeyActorClass))
#define EEK_IS_CLUTTER_KEY_ACTOR(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_CLUTTER_KEY_ACTOR))
#define EEK_IS_CLUTTER_KEY_ACTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_CLUTTER_KEY_ACTOR))
#define EEK_CLUTTER_KEY_ACTOR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_CLUTTER_KEY_ACTOR, EekClutterKeyActorClass))

typedef struct _EekClutterKeyActor        EekClutterKeyActor;
typedef struct _EekClutterKeyActorClass   EekClutterKeyActorClass;
typedef struct _EekClutterKeyActorPrivate EekClutterKeyActorPrivate;

struct _EekClutterKeyActor
{
    /*< private >*/
    ClutterGroup parent;

    /*< private >*/
    EekClutterKeyActorPrivate *priv;
};

struct _EekClutterKeyActorClass
{
    /*< private >*/
    ClutterGroupClass parent_class;

    /* signals */
    void (* pressed) (EekClutterKeyActor *self);
    void (* released) (EekClutterKeyActor *self);
};

GType eek_clutter_key_actor_get_type (void) G_GNUC_CONST;
ClutterActor *eek_clutter_key_actor_new (EekKey *key);

G_END_DECLS
#endif  /* EEK_CLUTTER_KEY_ACTOR_H */
