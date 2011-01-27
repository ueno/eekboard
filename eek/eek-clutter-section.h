/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
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
#ifndef EEK_CLUTTER_SECTION_H
#define EEK_CLUTTER_SECTION_H 1

#include <clutter/clutter.h>
#include "eek-section.h"
#include "eek-clutter-renderer.h"

G_BEGIN_DECLS
#define EEK_TYPE_CLUTTER_SECTION (eek_clutter_section_get_type())
#define EEK_CLUTTER_SECTION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_CLUTTER_SECTION, EekClutterSection))
#define EEK_CLUTTER_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_CLUTTER_SECTION, EekClutterSectionClass))
#define EEK_IS_CLUTTER_SECTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_CLUTTER_SECTION))
#define EEK_IS_CLUTTER_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_CLUTTER_SECTION))
#define EEK_CLUTTER_SECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_CLUTTER_SECTION, EekClutterSectionClass))

typedef struct _EekClutterSection        EekClutterSection;
typedef struct _EekClutterSectionClass   EekClutterSectionClass;
typedef struct _EekClutterSectionPrivate EekClutterSectionPrivate;

struct _EekClutterSection
{
    /*< private >*/
    ClutterGroup parent;

    EekClutterSectionPrivate *priv;
};

struct _EekClutterSectionClass
{
    /*< private >*/
    ClutterGroupClass parent_class;

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType         eek_clutter_section_get_type (void) G_GNUC_CONST;
ClutterActor *eek_clutter_section_new      (EekSection         *section,
                                            EekClutterRenderer *renderer);

G_END_DECLS
#endif  /* EEK_CLUTTER_SECTION_H */
