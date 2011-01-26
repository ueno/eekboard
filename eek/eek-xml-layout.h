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

struct _EekXmlLayout
{
    /*< private >*/
    EekLayout parent;

    EekXmlLayoutPrivate *priv;
};

struct _EekXmlLayoutClass
{
    /*< private >*/
    EekLayoutClass parent_class;

    /* padding */
    gpointer pdummy[24];
};

GType          eek_xml_layout_get_type   (void) G_GNUC_CONST;

EekLayout     *eek_xml_layout_new        (GInputStream *source);

void           eek_xml_layout_set_source (EekXmlLayout *layout,
                                          GInputStream *source);

GInputStream * eek_xml_layout_get_source (EekXmlLayout *layout);

G_END_DECLS
#endif  /* EEK_XML_LAYOUT_H */
