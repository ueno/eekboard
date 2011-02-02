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

/**
 * SECTION:eek-xml-layout
 * @short_description: Layout engine which loads layout information from XML
 */

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-xml-layout.h"
#include "eek-keyboard.h"
#include "eek-section.h"
#include "eek-key.h"
#include "eek-keysym.h"

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
    GHashTable *outline_hash;
};

#define BUFSIZE	8192

struct _ParseCallbackData {
    GSList *element_stack;
    GString *text;
    EekLayout *layout;

    EekKeyboard *keyboard;
    EekSection *section;
    EekKey *key;
    gint num_columns;
    EekOrientation orientation;
    GSList *points;
    GSList *symbols;
    guint keyval;
    gint groups, levels;
    EekOutline outline;
    gchar *oref;
    GHashTable *key_oref_hash;
    GHashTable *oref_outline_hash;
};
typedef struct _ParseCallbackData ParseCallbackData;

static const gchar *valid_path_list[] = {
    "keyboard",
    "bounds/keyboard",
    "section/keyboard",
    "outline/keyboard",
    "bounds/section/keyboard",
    "angle/section/keyboard",
    "row/section/keyboard",
    "columns/row/section/keyboard",
    "orientation/row/section/keyboard",
    "key/section/keyboard",
    "bounds/key/section/keyboard",
    "outline-ref/key/section/keyboard",
    "symbols/key/section/keyboard",
    "groups/symbols/key/section/keyboard",
    "levels/symbols/key/section/keyboard",
    "keysym/symbols/key/section/keyboard",
    "custom/symbols/key/section/keyboard",
    "text/symbols/key/section/keyboard",
    "icon/symbols/key/section/keyboard",
    "invalid/symbols/key/section/keyboard",
    "index/key/section/keyboard",
    "point/outline/keyboard"
};

static gchar *
strjoin_slist (GSList *slist, const gchar *delimiter)
{
    GString *string = g_string_sized_new (64);

    if (slist == NULL)
        return g_strdup ("");
    else
        for (; slist; slist = g_slist_next (slist)) {
            g_string_append (string, slist->data);
            if (g_slist_next (slist))
                g_string_append (string, delimiter);
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

    head = g_slist_prepend (element_stack, (gchar *)element_name);
    element_path = strjoin_slist (head, "/");
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
    gint column = -1, row = -1, groups = -1, levels = -1;
    guint keyval = EEK_INVALID_KEYSYM;
    gchar *name = NULL, *id = NULL, *version = NULL;

    validate (element_name, data->element_stack, error);
    if (error && *error)
        return;

    while (*names && *values) {
        if (g_strcmp0 (*names, "column") == 0)
            column = strtol (*values, NULL, 10);
        else if (g_strcmp0 (*names, "row") == 0)
            row = strtol (*values, NULL, 10);
        else if (g_strcmp0 (*names, "id") == 0)
            id = g_strdup (*values);
        else if (g_strcmp0 (*names, "name") == 0)
            name = g_strdup (*values);
        else if (g_strcmp0 (*names, "keyval") == 0)
            keyval = strtoul (*values, NULL, 10);
        else if (g_strcmp0 (*names, "version") == 0)
            version = g_strdup (*values);
        else if (g_strcmp0 (*names, "groups") == 0)
            groups = strtol (*values, NULL, 10);
        else if (g_strcmp0 (*names, "levels") == 0)
            levels = strtol (*values, NULL, 10);
        names++;
        values++;
    }

    if (g_strcmp0 (element_name, "keyboard") == 0) {
        data->keyboard = g_object_new (EEK_TYPE_KEYBOARD,
                                       "layout", data->layout,
                                       NULL);
        if (id)
            eek_element_set_name (EEK_ELEMENT(data->keyboard), id);
        goto out;
    }

    if (g_strcmp0 (element_name, "section") == 0) {
        data->section = eek_keyboard_create_section (data->keyboard);
        if (id)
            eek_element_set_name (EEK_ELEMENT(data->section), id);
        goto out;
    }

    if (g_strcmp0 (element_name, "key") == 0) {
        data->key = eek_section_create_key (data->section, column, row);
        if (id) {
            eek_element_set_name (EEK_ELEMENT(data->key), id);
            if (g_str_has_prefix (id, "keycode"))
                eek_key_set_keycode (data->key, strtoul (id + 7, NULL, 10));
        }
        goto out;
    }

    if (g_strcmp0 (element_name, "symbols") == 0) {
        data->groups = groups;
        data->levels = levels;
        data->symbols = NULL;
        goto out;
    }

    if (g_strcmp0 (element_name, "keysym") == 0)
        data->keyval = keyval;

    if (g_strcmp0 (element_name, "outline") == 0) {
        data->oref = g_strdup (id);
        goto out;
    }
 out:
    g_free (name);
    g_free (id);
    g_free (version);

    data->element_stack = g_slist_prepend (data->element_stack,
                                           g_strdup (element_name));
    data->text->len = 0;
}

static void
end_element_callback (GMarkupParseContext *pcontext,
                      const gchar         *element_name,
                      gpointer             user_data,
                      GError             **error)
{
    ParseCallbackData *data = user_data;
    GSList *head = data->element_stack;
    gchar *text, **strv;
    gint i;

    g_free (head->data);
    data->element_stack = g_slist_next (data->element_stack);
    g_slist_free1 (head);

    text = g_strndup (data->text->str, data->text->len);

    if (g_strcmp0 (element_name, "section") == 0) {
        data->section = NULL;
        goto out;
    }

    if (g_strcmp0 (element_name, "key") == 0) {
        data->key = NULL;
        goto out;
    }

    if (g_strcmp0 (element_name, "symbols") == 0) {
        gint num_symbols = data->groups * data->levels;
        EekSymbolMatrix *matrix = eek_symbol_matrix_new (data->groups,
                                                         data->levels);

        head = data->symbols = g_slist_reverse (data->symbols);
        for (i = 0; i < num_symbols; i++) {
            if (head && head->data) {
                matrix->data[i] = head->data;
                head = g_slist_next (head);
            } else
                matrix->data[i] = NULL;
        }
        g_slist_free (data->symbols);
        data->symbols = NULL;

        eek_key_set_symbol_matrix (data->key, matrix);
        eek_symbol_matrix_free (matrix);
        goto out;
    }

    if (g_strcmp0 (element_name, "outline") == 0) {
        EekOutline *outline = g_slice_new (EekOutline);
        
        outline->num_points = g_slist_length (data->points);
        outline->points = g_slice_alloc0 (sizeof (EekPoint) *
                                          outline->num_points);
        for (head = data->points = g_slist_reverse (data->points), i = 0;
             head;
             head = g_slist_next (head), i++) {
            memcpy (&outline->points[i], head->data, sizeof (EekPoint));
            g_slice_free1 (sizeof (EekPoint), head->data);
        }
        g_slist_free (data->points);
        data->points = NULL;

        g_hash_table_insert (data->oref_outline_hash,
                             g_strdup (data->oref),
                             outline);
        g_free (data->oref);
        goto out;
    }

    if (g_strcmp0 (element_name, "point") == 0) {
        EekPoint *point;

        strv = g_strsplit (text, ",", -1);

        if (g_strv_length (strv) != 2) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                         "invalid format for %s \"%s\"",
                         element_name,
                         text);
            goto out;
        }
            
        point = g_slice_new (EekPoint);
        point->x = g_strtod (strv[0], NULL);
        point->y = g_strtod (strv[1], NULL);

        g_strfreev (strv);

        data->points = g_slist_prepend (data->points, point);
        goto out;
    }

    if (g_strcmp0 (element_name, "bounds") == 0) {
        EekBounds bounds;

        strv = g_strsplit (text, ",", -1);

        if (g_strv_length (strv) != 4) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                         "invalid format for %s \"%s\"",
                         element_name,
                         text);
            goto out;
        }
            
        bounds.x = g_strtod (strv[0], NULL);
        bounds.y = g_strtod (strv[1], NULL);
        bounds.width = g_strtod (strv[2], NULL);
        bounds.height = g_strtod (strv[3], NULL);

        g_strfreev (strv);

        if (g_strcmp0 (data->element_stack->data, "keyboard") == 0)
            eek_element_set_bounds (EEK_ELEMENT(data->keyboard), &bounds);
        else if (g_strcmp0 (data->element_stack->data, "section") == 0)
            eek_element_set_bounds (EEK_ELEMENT(data->section), &bounds);
        else if (g_strcmp0 (data->element_stack->data, "key") == 0)
            eek_element_set_bounds (EEK_ELEMENT(data->key), &bounds);

        goto out;
    }

    if (g_strcmp0 (element_name, "angle") == 0) {
        eek_section_set_angle (data->section, strtol (text, NULL, 10));
        goto out;
    }

    if (g_strcmp0 (element_name, "orientation") == 0) {
        data->orientation = strtol (text, NULL, 10);
        goto out;
    }

    if (g_strcmp0 (element_name, "columns") == 0) {
        data->num_columns = strtol (text, NULL, 10);
        goto out;
    }

    if (g_strcmp0 (element_name, "row") == 0) {
        eek_section_add_row (data->section,
                             data->num_columns,
                             data->orientation);
        data->num_columns = 0;
        data->orientation = EEK_ORIENTATION_HORIZONTAL;
        goto out;
    }

    if (g_strcmp0 (element_name, "keysym") == 0) {
        EekKeysym *keysym;

        if (data->keyval != EEK_INVALID_KEYSYM) {
            keysym = eek_keysym_new (data->keyval);
            //g_debug ("%u %s", data->keyval, eek_symbol_get_label (EEK_SYMBOL(keysym)));
        } else
            keysym = eek_keysym_new_from_name (text);
        data->symbols = g_slist_prepend (data->symbols, keysym);
        goto out;
    }

    if (g_strcmp0 (element_name, "invalid") == 0) {
        data->symbols = g_slist_prepend (data->symbols, NULL);
        goto out;
    }

    if (g_strcmp0 (element_name, "outline-ref") == 0) {
        g_hash_table_insert (data->key_oref_hash, data->key, g_strdup (text));
        goto out;
    }

 out:
    g_free (text);
}

static void
text_callback (GMarkupParseContext *pcontext,
               const gchar         *text,
               gsize                text_len,  
               gpointer             user_data,
               GError             **error)
{
    ParseCallbackData *data = user_data;
    g_string_append_len (data->text, text, text_len);
}

static const GMarkupParser parser = {
    start_element_callback,
    end_element_callback,
    text_callback,
    0,
    0
};

static void
outline_free (gpointer data)
{
    EekOutline *outline = data;
    g_slice_free1 (sizeof (EekPoint) * outline->num_points, outline->points);
    g_boxed_free (EEK_TYPE_OUTLINE, outline);
}

static void scale_bounds_callback (EekElement *element,
                                   gpointer    user_data);

static void
scale_bounds (EekElement *element,
              gdouble     scale)
{
    EekBounds bounds;

    eek_element_get_bounds (element, &bounds);
    bounds.x *= scale;
    bounds.y *= scale;
    bounds.width *= scale;
    bounds.height *= scale;
    eek_element_set_bounds (element, &bounds);

    if (EEK_IS_CONTAINER(element))
        eek_container_foreach_child (EEK_CONTAINER(element),
                                     scale_bounds_callback,
                                     &scale);
}

static void
scale_bounds_callback (EekElement *element,
                       gpointer    user_data)
{
    scale_bounds (element, *(gdouble *)user_data);
}

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
    EekBounds bounds;
    gdouble scale;
    GHashTableIter iter;
    gpointer k, v;

    g_return_val_if_fail (priv->source, NULL);

    memset (&data, 0, sizeof data);
    data.layout = self;
    data.text = g_string_sized_new (BUFSIZE);
    data.key_oref_hash = g_hash_table_new_full (g_direct_hash,
                                                g_direct_equal,
                                                NULL,
                                                g_free);
    data.oref_outline_hash = g_hash_table_new_full (g_str_hash,
                                                    g_str_equal,
                                                    g_free,
                                                    outline_free);

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
    if (error)
        g_warning ("%s", error->message);

    error = NULL;
    g_markup_parse_context_end_parse (pcontext, &error);
    if (error)
        g_warning ("%s", error->message);

    g_markup_parse_context_free (pcontext);

    if (!data.keyboard)
        goto out;

    g_hash_table_iter_init (&iter, data.key_oref_hash);
    while (g_hash_table_iter_next (&iter, &k, &v)) {
        EekOutline *outline = g_hash_table_lookup (data.oref_outline_hash, v);
        g_assert (outline);
        eek_key_set_outline (EEK_KEY(k), outline);
    }

    eek_element_get_bounds (EEK_ELEMENT(data.keyboard), &bounds);
    scale = initial_width * bounds.height < initial_height * bounds.width ?
        initial_width / bounds.width :
        initial_height / bounds.height;

    g_hash_table_iter_init (&iter, data.oref_outline_hash);
    while (g_hash_table_iter_next (&iter, &k, &v)) {
        EekOutline *outline = v;
        gint i;

        for (i = 0; i < outline->num_points; i++) {
            outline->points[i].x *= scale;
            outline->points[i].y *= scale;
        }
    }

    scale_bounds (EEK_ELEMENT(data.keyboard), scale);

 out:
    g_string_free (data.text, TRUE);
    if (data.key_oref_hash)
        g_hash_table_destroy (data.key_oref_hash);
    priv->outline_hash = data.oref_outline_hash;

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
    G_OBJECT_CLASS (eek_xml_layout_parent_class)->dispose (object);
}

static void
eek_xml_layout_finalize (GObject *object)
{
    EekXmlLayoutPrivate *priv = EEK_XML_LAYOUT_GET_PRIVATE (object);

    if (priv->outline_hash)
        g_hash_table_unref (priv->outline_hash);

    G_OBJECT_CLASS (eek_xml_layout_parent_class)->finalize (object);
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
    gobject_class->finalize = eek_xml_layout_finalize;

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
