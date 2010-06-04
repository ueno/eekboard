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
#ifndef EEK_SIMPLE_SECTION_H
#define EEK_SIMPLE_SECTION_H 1

#include "eek-simple-key.h"
#include "eek-section.h"

G_BEGIN_DECLS
#define EEK_TYPE_SIMPLE_SECTION (eek_simple_section_get_type())
#define EEK_SIMPLE_SECTION(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_SIMPLE_SECTION, EekSimpleSection))
#define EEK_SIMPLE_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_SIMPLE_SECTION, EekSimpleSectionClass))
#define EEK_IS_SIMPLE_SECTION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_SIMPLE_SECTION))
#define EEK_IS_SIMPLE_SECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_SIMPLE_SECTION))
#define EEK_SIMPLE_SECTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_SIMPLE_SECTION, EekSimpleSectionClass))

typedef struct _EekSimpleSection        EekSimpleSection;
typedef struct _EekSimpleSectionClass   EekSimpleSectionClass;
typedef struct _EekSimpleSectionPrivate EekSimpleSectionPrivate;

struct _EekSimpleSection
{
    /*< private >*/
    GInitiallyUnowned parent;

    /*< private >*/
    EekSimpleSectionPrivate *priv;
};

struct _EekSimpleSectionClass
{
    /*< private >*/
    GInitiallyUnownedClass parent_class;
};

GType       eek_simple_section_get_type (void) G_GNUC_CONST;

G_END_DECLS
#endif  /* EEK_SIMPLE_SECTION_H */
