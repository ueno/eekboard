/* 
 * Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2011 Red Hat, Inc.
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

#if !defined(__EEK_H_INSIDE__) && !defined(EEK_COMPILATION)
#error "Only <eek/eek.h> can be included directly."
#endif

#ifndef EEK_XML_LAYOUT_H
#define EEK_XML_LAYOUT_H 1

#include <gio/gio.h>
#include "eek-layout.h"

G_BEGIN_DECLS

#define EEK_TYPE_XML_LAYOUT (eek_xml_layout_get_type())
#define EEK_XML_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_XML_LAYOUT, EekXmlLayout))
#define EEK_XML_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_XML_LAYOUT, EekXmlLayoutClass))
#define EEK_IS_XML_LAYOUT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_XML_LAYOUT))
#define EEK_IS_XML_LAYOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_XML_LAYOUT))
#define EEK_XML_LAYOUT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_XML_LAYOUT, EekXmlLayoutClass))

typedef struct _EekXmlLayout        EekXmlLayout;
typedef struct _EekXmlLayoutClass   EekXmlLayoutClass;
typedef struct _EekXmlLayoutPrivate EekXmlLayoutPrivate;

/**
 * EekXmlLayout:
 *
 * The #EekXmlLayout structure contains only private data and should
 * only be accessed using the provided API.
 */
struct _EekXmlLayout
{
    /*< private >*/
    EekLayout parent;

    EekXmlLayoutPrivate *priv;
};

/**
 * EekXmlLayoutClass:
 */
struct _EekXmlLayoutClass
{
    /*< private >*/
    EekLayoutClass parent_class;

    /* padding */
    gpointer pdummy[24];
};

struct _EekXmlKeyboardDesc
{
    gchar *id;
    gchar *name;
    gchar *geometry;
    gchar *symbols;
    gchar *language;
    gchar *longname;
};
typedef struct _EekXmlKeyboardDesc EekXmlKeyboardDesc;

GType               eek_xml_layout_get_type    (void) G_GNUC_CONST;
EekLayout          *eek_xml_layout_new         (const gchar        *id,
                                                GError            **error);
GList              *eek_xml_list_keyboards     (void);

EekXmlKeyboardDesc *eek_xml_keyboard_desc_copy (EekXmlKeyboardDesc *desc);
void                eek_xml_keyboard_desc_free (EekXmlKeyboardDesc *desc);

G_END_DECLS
#endif  /* EEK_XML_LAYOUT_H */
