/**
 * SECTION:eek-xml-layout
 * @short_description: Layout engine which loads layout information from XML
 */

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-xml-layout.h"
#include "eek-keyboard.h"
#include "eek-section.h"

enum {
    PROP_0,
    PROP_SOURCE,
    PROP_LAST
};

G_DEFINE_TYPE (EekXmlLayout, eek_xml_layout, EEK_TYPE_LAYOUT);

#define EEK_XML_LAYOUT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_XML_LAYOUT, EekXmlLayoutPrivate))

struct _EekXmlLayoutPrivate
{
    GInputStream *source;
};

#define BUFSIZE	8192

struct _ParseCallbackData {
    GSList *element_stack;
    GString *text;
    EekLayout *layout;
    EekKeyboard *keyboard;
    EekSection *section;
    EekKey *key;
};
typedef struct _ParseCallbackData ParseCallbackData;

static const gchar *valid_path_list[] = {
    "keyboard",
    "keyboard/bounds",
    "keyboard/section",
    "keyboard/outline",
    "section/bounds",
    "section/angle",
    "section/key",
    "bounds/key",
    "outline/key",
    "keysyms/key",
    "keycode/key",
    "index/key",
    "groups/keysyms",
    "levels/keysyms",
    "keysym/keysyms",
    "point/outline/keyboard"
};

static gchar *
join_element_names (GSList *element_names)
{
    GString *string = g_string_sized_new (64);

    if (element_names == NULL)
        return g_strdup ("");
    else
        for (; element_names; element_names = g_slist_next (element_names)) {
            g_string_append (string, element_names->data);
            if (g_slist_next (element_names))
                g_string_append (string, "/");
        }
    return g_string_free (string, FALSE);
}

static void
validate (const gchar  *element_name,
          GSList       *element_stack,
          GError      **error)
{
    gint i;
    gchar *element_path;
    GSList *head;

    head = g_slist_prepend (element_stack, element_name);
    element_path = join_element_names (head);
    g_slist_free1 (head);

    for (i = 0; i < G_N_ELEMENTS(valid_path_list); i++) {
        if (*valid_path_list[i] == '@')
            continue;
        if (g_strcmp0 (element_path, valid_path_list[i]) == 0)
            break;
    }

    if (i == G_N_ELEMENTS(valid_path_list)) {
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                     "%s cannot appear under %s",
                     element_name,
                     element_path);
        g_free (element_path);
        return;
    }
    g_free (element_path);
}

static void
start_element_callback (GMarkupParseContext *pcontext,
                        const gchar         *element_name,
                        const gchar        **attribute_names,
                        const gchar        **attribute_values,
                        gpointer             user_data,
                        GError             **error)
{
    ParseCallbackData *data = user_data;
    const gchar **names = attribute_names;
    const gchar **values = attribute_values;
    gint column = -1, row = -1;
    gchar *name = NULL;

    validate (element_name, data->element_stack, error);
    if (error && *error)
        return;

    while (*names && *values) {
        if (g_strcmp0 (*names, "column") == 0)
            column = strtol (*values, NULL, 10);
        else if (g_strcmp0 (*names, "row") == 0)
            row = strtol (*values, NULL, 10);
        else if (g_strcmp0 (*names, "name") == 0)
            name = g_strdup (*values);
        names++;
        values++;
    }

    if (g_strcmp0 (element_name, "keyboard") == 0) {
        data->keyboard = g_object_new (EEK_TYPE_KEYBOARD,
                                       "layout", data->layout,
                                       NULL);
        if (name)
            eek_element_set_name (EEK_ELEMENT(data->keyboard), name);
    } else if (g_strcmp0 (element_name, "section") == 0) {
        data->section = eek_keyboard_create_section (data->keyboard);
        if (name)
            eek_element_set_name (EEK_ELEMENT(data->section), name);
    } else if (g_strcmp0 (element_name, "key") == 0) {
        data->key = eek_section_create_key (data->section, column, row);
        if (name)
            eek_element_set_name (EEK_ELEMENT(data->key), name);
    }
    g_free (name);

    data->element_stack = g_slist_prepend (data->element_stack,
                                           g_strdup (element_name));
}

static void
end_element_callback (GMarkupParseContext *pcontext,
                      const gchar         *element_name,
                      gpointer             user_data,
                      GError             **error)
{
    ParseCallbackData *data = user_data;
    GSList *head = data->element_stack;

    g_free (head->data);
    data->element_stack = g_slist_next (data->element_stack);
    g_slist_free1 (head);
}

static void
text_callback (GMarkupParseContext *pcontext,
               const gchar         *text,
               gsize                text_len,  
               gpointer             user_data,
               GError             **error)
{
}

static const GMarkupParser parser = {
    start_element_callback,
    end_element_callback,
    text_callback,
    0,
    0
};

static EekKeyboard *
eek_xml_layout_real_create_keyboard (EekLayout *self,
                                     gdouble    initial_width,
                                     gdouble    initial_height)
{
    EekXmlLayoutPrivate *priv = EEK_XML_LAYOUT_GET_PRIVATE (self);
    GMarkupParseContext *pcontext;
    GError *error;
    gchar buffer[BUFSIZE];
    ParseCallbackData data;

    g_return_val_if_fail (priv->source, NULL);

    data.element_stack = NULL;
    data.text = g_string_sized_new (BUFSIZE);
    data.keyboard = NULL;
    pcontext = g_markup_parse_context_new (&parser, 0, &data, NULL);
    while (1) {
        gssize nread;
                
        error = NULL;
        nread = g_input_stream_read (G_INPUT_STREAM(priv->source),
                                     buffer, sizeof buffer, NULL,
                                     &error);
        if (nread <= 0)
            break;

        error = NULL;
        if (!g_markup_parse_context_parse (pcontext, buffer, nread, &error))
            break;
    }

    error = NULL;
    g_markup_parse_context_end_parse (pcontext, &error);
    g_markup_parse_context_free (pcontext);
    g_string_free (data.text, TRUE);

    return data.keyboard;
}

static void
eek_xml_layout_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    switch (prop_id) {
    case PROP_SOURCE:
        eek_xml_layout_set_source (EEK_XML_LAYOUT(object),
                                   g_value_get_object (value));
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_xml_layout_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    switch (prop_id) {
    case PROP_SOURCE:
        g_value_set_object (value,
                            eek_xml_layout_get_source (EEK_XML_LAYOUT(object)));
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_xml_layout_dispose (GObject *object)
{
    EekXmlLayoutPrivate *priv = EEK_XML_LAYOUT_GET_PRIVATE (object);

    if (priv->source) {
        g_object_unref (priv->source);
        priv->source = NULL;
    }
}

static void
eek_xml_layout_class_init (EekXmlLayoutClass *klass)
{
    EekLayoutClass *layout_class = EEK_LAYOUT_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec *pspec;

    g_type_class_add_private (gobject_class, sizeof (EekXmlLayoutPrivate));

    layout_class->create_keyboard = eek_xml_layout_real_create_keyboard;

    gobject_class->set_property = eek_xml_layout_set_property;
    gobject_class->get_property = eek_xml_layout_get_property;
    gobject_class->dispose = eek_xml_layout_dispose;

    pspec = g_param_spec_object ("source",
				 "Source",
				 "XML source input stram",
				 G_TYPE_INPUT_STREAM,
				 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_SOURCE, pspec);



}

static void
eek_xml_layout_init (EekXmlLayout *self)
{
    EekXmlLayoutPrivate *priv;

    priv = self->priv = EEK_XML_LAYOUT_GET_PRIVATE (self);
    priv->source = NULL;
}

EekLayout *
eek_xml_layout_new (GInputStream *source)
{
    return g_object_new (EEK_TYPE_XML_LAYOUT, "source", source, NULL);
}

void
eek_xml_layout_set_source (EekXmlLayout *layout,
                           GInputStream *source)
{
    EekXmlLayoutPrivate *priv;

    g_return_if_fail (EEK_IS_XML_LAYOUT(layout));
    g_return_if_fail (G_IS_INPUT_STREAM(source));

    priv = EEK_XML_LAYOUT_GET_PRIVATE(layout);
    if (priv->source)
        g_object_unref (priv->source);
    priv->source = g_object_ref (source);
}

GInputStream *
eek_xml_layout_get_source (EekXmlLayout *layout)
{
    EekXmlLayoutPrivate *priv;

    g_assert (EEK_IS_XML_LAYOUT(layout));
    priv = EEK_XML_LAYOUT_GET_PRIVATE(layout);
    return priv->source;
}
