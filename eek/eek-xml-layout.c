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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>

#include "eek-xml-layout.h"
#include "eek-keyboard.h"
#include "eek-section.h"
#include "eek-key.h"
#include "eek-keysym.h"
#include "eek-text.h"

enum {
    PROP_0,
    PROP_ID,
    PROP_LAST
};

static void initable_iface_init (GInitableIface *initable_iface);

G_DEFINE_TYPE_WITH_CODE (EekXmlLayout, eek_xml_layout, EEK_TYPE_LAYOUT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                initable_iface_init));

#define EEK_XML_LAYOUT_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_XML_LAYOUT, EekXmlLayoutPrivate))

struct _EekXmlLayoutPrivate
{
    gchar *id;
    EekXmlKeyboardDesc *desc;
};

G_DEFINE_BOXED_TYPE(EekXmlKeyboardDesc, eek_xml_keyboard_desc, eek_xml_keyboard_desc_copy, eek_xml_keyboard_desc_free);

#define BUFSIZE	8192

static GList        *parse_keyboards (const gchar         *path,
                                      GError             **error);
static GList        *parse_prerequisites
                                     (const gchar         *path,
                                      GError             **error);
static gboolean      parse_geometry  (const gchar         *path,
                                      EekKeyboard         *keyboard,
                                      GError             **error);
static gboolean      parse_symbols_with_prerequisites
                                     (const gchar         *name,
                                      EekKeyboard         *keyboard,
                                      GList             **loaded,
                                      GError             **error);
static gboolean      parse_symbols   (const gchar         *path,
                                      EekKeyboard         *keyboard,
                                      GError             **error);

static void          scale_keyboard  (EekKeyboard         *keyboard,
                                      gdouble              width,
                                      gdouble              height);

static gboolean      validate        (const gchar        **valid_path_list,
                                      gsize                valid_path_list_len,
                                      const gchar         *element_name,
                                      GSList              *element_stack,
                                      GError             **error);

static gboolean      parse           (GMarkupParseContext *pcontext,
                                      GInputStream        *input,
                                      GError             **error);
static const gchar * get_attribute   (const gchar        **names,
                                      const gchar        **values,
                                      const gchar         *name);

static void
keyboard_desc_free (EekXmlKeyboardDesc *desc)
{
    g_free (desc->id);
    g_free (desc->name);
    g_free (desc->geometry);
    g_free (desc->symbols);
    g_free (desc->longname);
    g_free (desc->language);
    g_slice_free (EekXmlKeyboardDesc, desc);
}

struct _KeyboardsParseData {
    GSList *element_stack;

    GList *keyboards;
};
typedef struct _KeyboardsParseData KeyboardsParseData;

static KeyboardsParseData *
keyboards_parse_data_new (void)
{
    return g_slice_new0 (KeyboardsParseData);
}

static void
keyboards_parse_data_free (KeyboardsParseData *data)
{
    g_list_free_full (data->keyboards, (GDestroyNotify) keyboard_desc_free);
    g_slice_free (KeyboardsParseData, data);
}

static const gchar *keyboards_valid_path_list[] = {
    "keyboards",
    "keyboard/keyboards",
};

static void
keyboards_start_element_callback (GMarkupParseContext *pcontext,
                                  const gchar         *element_name,
                                  const gchar        **attribute_names,
                                  const gchar        **attribute_values,
                                  gpointer             user_data,
                                  GError             **error)
{
    KeyboardsParseData *data = user_data;

    if (!validate (keyboards_valid_path_list,
                   G_N_ELEMENTS (keyboards_valid_path_list),
                   element_name,
                   data->element_stack,
                   error))
        return;

    if (g_strcmp0 (element_name, "keyboard") == 0) {
        EekXmlKeyboardDesc *desc = g_slice_new0 (EekXmlKeyboardDesc);
        const gchar *attribute;

        data->keyboards = g_list_prepend (data->keyboards, desc);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "id");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"id\" attribute for \"keyboard\"");
            return;
        }
        desc->id = g_strdup (attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "name");
        if (attribute)
            desc->name = g_strdup (attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "geometry");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"geometry\" attribute for \"keyboard\"");
            return;
        }
        desc->geometry = g_strdup (attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "symbols");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"symbols\" attribute for \"keyboard\"");
            goto out;
        }
        desc->symbols = g_strdup (attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "longname");
        if (attribute)
            desc->longname = g_strdup (attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "language");
        if (attribute)
            desc->language = g_strdup (attribute);
    }

 out:
    data->element_stack = g_slist_prepend (data->element_stack,
                                           g_strdup (element_name));
}

static void
keyboards_end_element_callback (GMarkupParseContext *pcontext,
                                const gchar         *element_name,
                                gpointer             user_data,
                                GError             **error)
{
    KeyboardsParseData *data = user_data;
    GSList *head = data->element_stack;

    g_free (head->data);
    data->element_stack = g_slist_next (data->element_stack);
    g_slist_free1 (head);
}

static const GMarkupParser keyboards_parser = {
    keyboards_start_element_callback,
    keyboards_end_element_callback,
    0,
    0,
    0
};

struct _GeometryParseData {
    GSList *element_stack;

    EekBounds bounds;
    EekKeyboard *keyboard;
    EekSection *section;
    EekKey *key;
    gint num_columns;
    EekOrientation orientation;
    gdouble corner_radius;
    GSList *points;
    gchar *name;
    EekOutline outline;
    gchar *oref;

    GHashTable *key_oref_hash;
    GHashTable *oref_outline_hash;
};
typedef struct _GeometryParseData GeometryParseData;

static GeometryParseData *
geometry_parse_data_new (EekKeyboard *keyboard)
{
    GeometryParseData *data = g_slice_new0 (GeometryParseData);

    data->keyboard = g_object_ref (keyboard);
    data->key_oref_hash =
        g_hash_table_new_full (g_direct_hash,
                               g_direct_equal,
                               NULL,
                               g_free);
    data->oref_outline_hash =
        g_hash_table_new_full (g_str_hash,
                               g_str_equal,
                               g_free,
                               (GDestroyNotify)eek_outline_free);
    return data;
}

static void
geometry_parse_data_free (GeometryParseData *data)
{
    g_object_unref (data->keyboard);
    g_hash_table_destroy (data->key_oref_hash);
    g_hash_table_destroy (data->oref_outline_hash);
    g_slice_free (GeometryParseData, data);
}

static const gchar *geometry_valid_path_list[] = {
    "geometry",
    "bounds/geometry",
    "section/geometry",
    "outline/geometry",
    "bounds/section/geometry",
    "row/section/geometry",
    "key/section/geometry",
    "bounds/key/section/geometry",
    "point/outline/geometry",
};

static void
geometry_start_element_callback (GMarkupParseContext *pcontext,
                                 const gchar         *element_name,
                                 const gchar        **attribute_names,
                                 const gchar        **attribute_values,
                                 gpointer             user_data,
                                 GError             **error)
{
    GeometryParseData *data = user_data;
    const gchar *attribute;

    if (!validate (geometry_valid_path_list,
                   G_N_ELEMENTS (geometry_valid_path_list),
                   element_name,
                   data->element_stack,
                   error)) {
        return;
    }

    if (g_strcmp0 (element_name, "bounds") == 0) {
        EekBounds bounds;

        attribute = get_attribute (attribute_names, attribute_values, "x");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"x\" attribute for \"bounds\"");
            return;
        }
        bounds.x = g_strtod (attribute, NULL);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "y");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"y\" attribute for \"bounds\"");
            return;
        }
        bounds.y = g_strtod (attribute, NULL);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "width");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"width\" attribute for \"bounds\"");
            return;
        }
        bounds.width = g_strtod (attribute, NULL);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "height");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"height\" attribute for \"bounds\"");
            return;
        }
        bounds.height = g_strtod (attribute, NULL);

        if (g_strcmp0 (data->element_stack->data, "geometry") == 0)
            eek_element_set_bounds (EEK_ELEMENT(data->keyboard), &bounds);
        else if (g_strcmp0 (data->element_stack->data, "section") == 0)
            eek_element_set_bounds (EEK_ELEMENT(data->section), &bounds);
        else if (g_strcmp0 (data->element_stack->data, "key") == 0)
            eek_element_set_bounds (EEK_ELEMENT(data->key), &bounds);

        goto out;
    }

    if (g_strcmp0 (element_name, "section") == 0) {
        data->section = eek_keyboard_create_section (data->keyboard);
        attribute = get_attribute (attribute_names, attribute_values,
                                   "id");
        if (attribute != NULL)
            eek_element_set_name (EEK_ELEMENT(data->section), attribute);
        attribute = get_attribute (attribute_names, attribute_values,
                                   "angle");
        if (attribute != NULL) {
            gint angle;
            angle = strtol (attribute, NULL, 10);
            eek_section_set_angle (data->section, angle);
        }
        goto out;
    }

    if (g_strcmp0 (element_name, "row") == 0) {
        attribute = get_attribute (attribute_names, attribute_values,
                                   "columns");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"columns\" attribute for \"row\"");
            return;
        }
        data->num_columns = strtol (attribute, NULL, 10);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "orientation");
        if (attribute != NULL)
            data->orientation = strtol (attribute, NULL, 10);
        goto out;
    }

    if (g_strcmp0 (element_name, "key") == 0) {
        guint keycode;
        gint column, row;

        attribute = get_attribute (attribute_names, attribute_values,
                                   "keycode");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"keycode\" attribute for \"key\"");
            return;
        }
        keycode = strtoul (attribute, NULL, 10);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "column");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"column\" attribute for \"key\"");
            return;
        }
        column = strtol (attribute, NULL, 10);
        
        attribute = get_attribute (attribute_names, attribute_values,
                                   "row");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"row\" attribute for \"row\"");
            return;
        }
        row = strtol (attribute, NULL, 10);

        data->key = eek_section_create_key (data->section, keycode, column, row);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "name");
        if (attribute != NULL)
            eek_element_set_name (EEK_ELEMENT(data->key), attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "oref");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"oref\" attribute for \"key\"");
            return;
        }
        g_hash_table_insert (data->key_oref_hash,
                             data->key,
                             g_strdup (attribute));
        goto out;
    }

    if (g_strcmp0 (element_name, "outline") == 0) {
        attribute = get_attribute (attribute_names, attribute_values, "id");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"id\" attribute for \"outline\"");
            return;
        }
        data->oref = g_strdup (attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "corner-radius");
        if (attribute != NULL)
            data->corner_radius = g_strtod (attribute, NULL);
        
        goto out;
    }

    if (g_strcmp0 (element_name, "point") == 0) {
        EekPoint *point;
        gdouble x, y;

        attribute = get_attribute (attribute_names, attribute_values, "x");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"x\" attribute for \"bounds\"");
            return;
        }
        x = g_strtod (attribute, NULL);

        attribute = get_attribute (attribute_names, attribute_values, "y");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"y\" attribute for \"bounds\"");
            return;
        }
        y = g_strtod (attribute, NULL);

        point = g_slice_new (EekPoint);
        point->x = x;
        point->y = y;

        data->points = g_slist_prepend (data->points, point);
        goto out;
    }

 out:
    data->element_stack = g_slist_prepend (data->element_stack,
                                           g_strdup (element_name));
}

static void
geometry_end_element_callback (GMarkupParseContext *pcontext,
                               const gchar         *element_name,
                               gpointer             user_data,
                               GError             **error)
{
    GeometryParseData *data = user_data;
    GSList *head = data->element_stack;
    gint i;

    g_free (head->data);
    data->element_stack = g_slist_next (data->element_stack);
    g_slist_free1 (head);

    if (g_strcmp0 (element_name, "section") == 0) {
        data->section = NULL;
        return;
    }

    if (g_strcmp0 (element_name, "key") == 0) {
        data->key = NULL;
        return;
    }

    if (g_strcmp0 (element_name, "outline") == 0) {
        EekOutline *outline = g_slice_new (EekOutline);

        outline->corner_radius = data->corner_radius;
        data->corner_radius = 0.0;

        outline->num_points = g_slist_length (data->points);
        outline->points = g_slice_alloc0 (sizeof (EekPoint) *
                                          outline->num_points);
        for (head = data->points = g_slist_reverse (data->points), i = 0;
             head && i < outline->num_points;
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
        return;
    }

    if (g_strcmp0 (element_name, "row") == 0) {
        eek_section_add_row (data->section,
                             data->num_columns,
                             data->orientation);
        data->num_columns = 0;
        data->orientation = EEK_ORIENTATION_HORIZONTAL;
        return;
    }
}

static const GMarkupParser geometry_parser = {
    geometry_start_element_callback,
    geometry_end_element_callback,
    0,
    0,
    0
};

struct _SymbolsParseData {
    GSList *element_stack;
    GString *text;

    EekKeyboard *keyboard;
    EekKey *key;
    GSList *symbols;
    gchar *label;
    gchar *icon;
    EekSymbolCategory category;
    guint keyval;
    gint groups;
};
typedef struct _SymbolsParseData SymbolsParseData;

static SymbolsParseData *
symbols_parse_data_new (EekKeyboard *keyboard)
{
    SymbolsParseData *data = g_slice_new0 (SymbolsParseData);

    data->keyboard = g_object_ref (keyboard);
    data->text = g_string_sized_new (BUFSIZE);
    return data;
}

static void
symbols_parse_data_free (SymbolsParseData *data)
{
    g_object_unref (data->keyboard);
    g_string_free (data->text, TRUE);
    g_slice_free (SymbolsParseData, data);
}

static const gchar *symbols_valid_path_list[] = {
    "symbols",
    "include/symbols",
    "key/symbols",
    "text/key/symbols",
    "keysym/key/symbols",
    "symbol/key/symbols",
    "invalid/key/symbols",
};

static void
symbols_start_element_callback (GMarkupParseContext *pcontext,
                                const gchar         *element_name,
                                const gchar        **attribute_names,
                                const gchar        **attribute_values,
                                gpointer             user_data,
                                GError             **error)
{
    SymbolsParseData *data = user_data;
    const gchar *attribute;

    if (!validate (symbols_valid_path_list,
                   G_N_ELEMENTS (symbols_valid_path_list),
                   element_name,
                   data->element_stack,
                   error))
        return;

    if (g_strcmp0 (element_name, "key") == 0) {
        guint keycode;

        attribute = get_attribute (attribute_names, attribute_values,
                                   "keycode");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"keycode\" attribute for \"key\"");
            return;
        }
        keycode = strtoul (attribute, NULL, 10);

        data->key = eek_keyboard_find_key_by_keycode (data->keyboard,
                                                      keycode);
        if (data->key == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_INVALID_CONTENT,
                         "no such keycode %u", keycode);
            return;
        }

        attribute = get_attribute (attribute_names, attribute_values,
                                   "groups");
        if (attribute != NULL)
            data->groups = strtol (attribute, NULL, 10);
        else
            data->groups = 1;
        data->symbols = NULL;
        goto out;
    }

    if (g_strcmp0 (element_name, "keysym") == 0) {
        attribute = get_attribute (attribute_names, attribute_values,
                                   "keyval");
        if (attribute == NULL) {
            g_set_error (error,
                         G_MARKUP_ERROR,
                         G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                         "no \"keyval\" attribute for \"keysym\"");
            return;
        }
        data->keyval = strtoul (attribute, NULL, 0);
    }

    if (g_strcmp0 (element_name, "symbol") == 0 ||
        g_strcmp0 (element_name, "keysym") == 0 ||
        g_strcmp0 (element_name, "text") == 0) {
        attribute = get_attribute (attribute_names, attribute_values,
                                   "label");
        if (attribute != NULL)
            data->label = g_strdup (attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "icon");
        if (attribute != NULL)
            data->icon = g_strdup (attribute);

        attribute = get_attribute (attribute_names, attribute_values,
                                   "category");
        if (attribute != NULL)
            data->category = strtoul (attribute, NULL, 10);
        else
            data->category = EEK_SYMBOL_CATEGORY_KEYNAME;
    }

 out:
    data->element_stack = g_slist_prepend (data->element_stack,
                                           g_strdup (element_name));
    data->text->len = 0;
}

static void
symbols_end_element_callback (GMarkupParseContext *pcontext,
                              const gchar         *element_name,
                              gpointer             user_data,
                              GError             **error)
{
    SymbolsParseData *data = user_data;
    GSList *head = data->element_stack;
    gchar *text;
    gint i;

    g_free (head->data);
    data->element_stack = g_slist_next (data->element_stack);
    g_slist_free1 (head);

    text = g_strndup (data->text->str, data->text->len);

    if (g_strcmp0 (element_name, "key") == 0) {
        gint num_symbols = g_slist_length (data->symbols);
        gint levels = num_symbols / data->groups;
        EekSymbolMatrix *matrix = eek_symbol_matrix_new (data->groups,
                                                         levels);

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
        data->key = NULL;
        goto out;
    }

    if (g_strcmp0 (element_name, "symbol") == 0 ||
        g_strcmp0 (element_name, "keysym") == 0 ||
        g_strcmp0 (element_name, "text") == 0) {
        EekSymbol *symbol;

        if (g_strcmp0 (element_name, "keysym") == 0) {
            EekKeysym *keysym;
            if (data->keyval != EEK_INVALID_KEYSYM)
                keysym = eek_keysym_new (data->keyval);
            else
                keysym = eek_keysym_new_from_name (text);
            symbol = EEK_SYMBOL(keysym);
        } else if (g_strcmp0 (element_name, "text") == 0) {
            symbol = EEK_SYMBOL(eek_text_new (text));
        } else {
            symbol = eek_symbol_new (text);
            eek_symbol_set_category (symbol, EEK_SYMBOL_CATEGORY_KEYNAME);
        }

        if (data->label) {
            eek_symbol_set_label (symbol, data->label);
            g_free (data->label);
            data->label = NULL;
        }
        if (data->icon) {
            eek_symbol_set_icon_name (symbol, data->icon);
            g_free (data->icon);
            data->icon = NULL;
        }

        data->symbols = g_slist_prepend (data->symbols, symbol);
        goto out;
    }

    if (g_strcmp0 (element_name, "invalid") == 0) {
        data->symbols = g_slist_prepend (data->symbols, NULL);
        goto out;
    }

 out:
    g_free (text);
}

static void
symbols_text_callback (GMarkupParseContext *pcontext,
                       const gchar         *text,
                       gsize                text_len,
                       gpointer             user_data,
                       GError             **error)
{
    SymbolsParseData *data = user_data;
    g_string_append_len (data->text, text, text_len);
}

static const GMarkupParser symbols_parser = {
    symbols_start_element_callback,
    symbols_end_element_callback,
    symbols_text_callback,
    0,
    0
};

struct _PrerequisitesParseData {
    GSList *element_stack;
    GString *text;

    GList *prerequisites;
};
typedef struct _PrerequisitesParseData PrerequisitesParseData;

static PrerequisitesParseData *
prerequisites_parse_data_new (void)
{
    PrerequisitesParseData *data = g_slice_new0 (PrerequisitesParseData);
    data->text = g_string_sized_new (BUFSIZE);
    return data;
}

static void
prerequisites_parse_data_free (PrerequisitesParseData *data)
{
    g_list_free_full (data->prerequisites, g_free);
    g_string_free (data->text, TRUE);
    g_slice_free (PrerequisitesParseData, data);
}

static void
prerequisites_start_element_callback (GMarkupParseContext *pcontext,
                                      const gchar         *element_name,
                                      const gchar        **attribute_names,
                                      const gchar        **attribute_values,
                                      gpointer             user_data,
                                      GError             **error)
{
    PrerequisitesParseData *data = user_data;

    if (!validate (symbols_valid_path_list,
                   G_N_ELEMENTS (symbols_valid_path_list),
                   element_name,
                   data->element_stack,
                   error))
        return;

    data->element_stack = g_slist_prepend (data->element_stack,
                                           g_strdup (element_name));
    data->text->len = 0;
}

static void
prerequisites_end_element_callback (GMarkupParseContext *pcontext,
                                    const gchar         *element_name,
                                    gpointer             user_data,
                                    GError             **error)
{
    PrerequisitesParseData *data = user_data;
    GSList *head = data->element_stack;

    g_free (head->data);
    data->element_stack = g_slist_next (data->element_stack);
    g_slist_free1 (head);

    if (g_strcmp0 (element_name, "include") == 0) {
        data->prerequisites = g_list_append (data->prerequisites,
                                              g_strndup (data->text->str,
                                                         data->text->len));
    }
}

static void
prerequisites_text_callback (GMarkupParseContext *pcontext,
                             const gchar         *text,
                             gsize                text_len,
                             gpointer             user_data,
                             GError             **error)
{
    PrerequisitesParseData *data = user_data;
    g_string_append_len (data->text, text, text_len);
}

static const GMarkupParser prerequisites_parser = {
    prerequisites_start_element_callback,
    prerequisites_end_element_callback,
    prerequisites_text_callback,
    0,
    0
};

static EekKeyboard *
eek_xml_layout_real_create_keyboard (EekLayout *self,
                                     gdouble    initial_width,
                                     gdouble    initial_height)
{
    EekXmlLayout *layout = EEK_XML_LAYOUT (self);
    EekKeyboard *keyboard;
    gchar *filename, *path;
    GList *loaded;
    GError *error;
    gboolean retval;

    /* Create an empty keyboard to which geometry and symbols
       information are applied. */
    keyboard = g_object_new (EEK_TYPE_KEYBOARD, "layout", layout, NULL);

    /* Read geometry information. */
    filename = g_strdup_printf ("%s.xml", layout->priv->desc->geometry);
    path = g_build_filename (KEYBOARDSDIR, "geometry", filename, NULL);
    g_free (filename);

    error = NULL;
    retval = parse_geometry (path, keyboard, &error);
    g_free (path);
    if (!retval) {
        g_object_unref (keyboard);
        g_warning ("can't parse geometry file %s: %s",
                   layout->priv->desc->geometry,
                   error->message);
        g_error_free (error);
        return NULL;
    }

    /* Read symbols information. */
    loaded = NULL;
    retval = parse_symbols_with_prerequisites (layout->priv->desc->symbols,
                                               keyboard,
                                               &loaded,
                                               &error);
    g_list_free_full (loaded, g_free);
    if (!retval) {
        g_object_unref (keyboard);
        g_warning ("can't parse symbols file %s: %s",
                   layout->priv->desc->symbols,
                   error->message);
        g_error_free (error);
        return NULL;
    }

    /* Fit keyboard in the given width and hight. */
    scale_keyboard (keyboard, initial_width, initial_height);

    /* Use pre-defined modifier mask here. */
    eek_keyboard_set_num_lock_mask (keyboard, EEK_MOD2_MASK);
    eek_keyboard_set_alt_gr_mask (keyboard, EEK_MOD5_MASK);

    return keyboard;
}

static void
eek_xml_layout_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    EekXmlLayout *layout = EEK_XML_LAYOUT (object);
    switch (prop_id) {
    case PROP_ID:
        g_free (layout->priv->id);
        layout->priv->id = g_value_dup_string (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
eek_xml_layout_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    EekXmlLayout *layout = EEK_XML_LAYOUT (object);
    switch (prop_id) {
    case PROP_ID:
        g_value_set_string (value, layout->priv->id);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
eek_xml_layout_finalize (GObject *object)
{
    EekXmlLayoutPrivate *priv = EEK_XML_LAYOUT_GET_PRIVATE (object);

    g_free (priv->id);

    if (priv->desc)
        keyboard_desc_free (priv->desc);

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
    gobject_class->finalize = eek_xml_layout_finalize;

    pspec = g_param_spec_string ("id",
				 "ID",
				 "ID",
				 NULL,
				 G_PARAM_CONSTRUCT_ONLY |
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_ID, pspec);
}

static void
eek_xml_layout_init (EekXmlLayout *self)
{
    self->priv = EEK_XML_LAYOUT_GET_PRIVATE (self);
}

EekLayout *
eek_xml_layout_new (const gchar *id, GError **error)
{
    return (EekLayout *) g_initable_new (EEK_TYPE_XML_LAYOUT,
                                         NULL,
                                         error,
                                         "id", id,
                                         NULL);
}

static gboolean
initable_init (GInitable    *initable,
               GCancellable *cancellable,
               GError      **error)
{
    EekXmlLayout *layout = EEK_XML_LAYOUT (initable);
    GList *keyboards, *p;
    gchar *path;
    EekXmlKeyboardDesc *desc;

    path = g_build_filename (KEYBOARDSDIR, "keyboards.xml", NULL);
    keyboards = parse_keyboards (path, error);
    g_free (path);
    if (error && *error)
        return FALSE;

    for (p = keyboards; p; p = p->next) {
        desc = p->data;
        if (g_strcmp0 (desc->id, layout->priv->id) == 0)
            break;
    }
    if (p == NULL) {
        g_set_error (error,
                     EEK_ERROR,
                     EEK_ERROR_LAYOUT_ERROR,
                     "no such keyboard %s",
                     layout->priv->id);
        return FALSE;
    }

    keyboards = g_list_remove_link (keyboards, p);
    layout->priv->desc = p->data;
    g_list_free_1 (p);
    g_list_free_full (keyboards, (GDestroyNotify) keyboard_desc_free);

    return TRUE;
}

static void
initable_iface_init (GInitableIface *initable_iface)
{
    initable_iface->init = initable_init;
}

GList *
eek_xml_list_keyboards (void)
{
    gchar *path;
    GList *keyboards;

    path = g_build_filename (KEYBOARDSDIR, "keyboards.xml", NULL);
    keyboards = parse_keyboards (path, NULL);
    g_free (path);
    return keyboards;
}

EekXmlKeyboardDesc *
eek_xml_keyboard_desc_copy (EekXmlKeyboardDesc *desc)
{
    return g_slice_dup (EekXmlKeyboardDesc, desc);
}

void
eek_xml_keyboard_desc_free (EekXmlKeyboardDesc *desc)
{
    g_free (desc->id);
    g_free (desc->name);
    g_free (desc->geometry);
    g_free (desc->symbols);
    g_free (desc->language);
    g_free (desc->longname);
    g_slice_free (EekXmlKeyboardDesc, desc);
}

static gboolean
parse_geometry (const gchar *path, EekKeyboard *keyboard, GError **error)
{
    GeometryParseData *data;
    GMarkupParseContext *pcontext;
    GHashTable *oref_hash;
    GHashTableIter iter;
    gpointer k, v;
    GFile *file;
    GFileInputStream *input;
    gboolean retval;

    file = g_file_new_for_path (path);

    input = g_file_read (file, NULL, error);
    g_object_unref (file);
    if (input == NULL)
        return FALSE;

    data = geometry_parse_data_new (keyboard);
    pcontext = g_markup_parse_context_new (&geometry_parser,
                                           0,
                                           data,
                                           NULL);

    retval = parse (pcontext, G_INPUT_STREAM (input), error);
    g_markup_parse_context_free (pcontext);
    g_object_unref (input);
    if (!retval) {
        geometry_parse_data_free (data);
        return FALSE;
    }

    /* Resolve outline references. */
    oref_hash = g_hash_table_new (g_str_hash, g_str_equal);
    g_hash_table_iter_init (&iter, data->oref_outline_hash);
    while (g_hash_table_iter_next (&iter, &k, &v)) {
        EekOutline *outline = v;
        gulong oref;

        oref = eek_keyboard_add_outline (data->keyboard, outline);
        g_hash_table_insert (oref_hash, k, GUINT_TO_POINTER(oref));
    }

    g_hash_table_iter_init (&iter, data->key_oref_hash);
    while (g_hash_table_iter_next (&iter, &k, &v)) {
        gpointer oref;
        if (g_hash_table_lookup_extended (oref_hash, v, NULL, &oref))
            eek_key_set_oref (EEK_KEY(k), GPOINTER_TO_UINT(oref));
    }
    g_hash_table_destroy (oref_hash);

    geometry_parse_data_free (data);
    return TRUE;
}

static gboolean
parse_symbols_with_prerequisites (const gchar *name,
                                  EekKeyboard *keyboard,
                                  GList     **loaded,
                                  GError     **error)
{
    gchar *filename, *path;
    GList *prerequisites, *p;
    GError *prerequisites_error;
    gboolean retval;

    for (p = *loaded; p; p = p->next) {
        if (g_strcmp0 (p->data, name) == 0) {
            g_set_error (error,
                         EEK_ERROR,
                         EEK_ERROR_LAYOUT_ERROR,
                         "%s already loaded",
                         name);
            return FALSE;
        }
    }
    *loaded = g_list_prepend (*loaded, g_strdup (name));

    filename = g_strdup_printf ("%s.xml", name);
    path = g_build_filename (KEYBOARDSDIR, "symbols", filename, NULL);
    g_free (filename);

    prerequisites_error = NULL;
    prerequisites = parse_prerequisites (path, &prerequisites_error);
    if (prerequisites_error != NULL) {
        g_propagate_error (error, prerequisites_error);
        return FALSE;
    }

    for (p = prerequisites; p; p = p->next) {
        retval = parse_symbols_with_prerequisites (p->data,
                                                   keyboard,
                                                   loaded,
                                                   error);
        if (!retval)
            return FALSE;
    }
    g_list_free_full (prerequisites, (GDestroyNotify)g_free);

    retval = parse_symbols (path, keyboard, error);
    g_free (path);

    return retval;
}

static gboolean
parse_symbols (const gchar *path, EekKeyboard *keyboard, GError **error)
{
    SymbolsParseData *data;
    GMarkupParseContext *pcontext;
    GFile *file;
    GFileInputStream *input;
    gboolean retval;

    file = g_file_new_for_path (path);

    input = g_file_read (file, NULL, error);
    g_object_unref (file);
    if (input == NULL)
        return FALSE;

    data = symbols_parse_data_new (keyboard);
    pcontext = g_markup_parse_context_new (&symbols_parser,
                                           0,
                                           data,
                                           NULL);
    retval = parse (pcontext, G_INPUT_STREAM (input), error);
    g_markup_parse_context_free (pcontext);
    g_object_unref (input);
    if (!retval) {
        symbols_parse_data_free (data);
        return FALSE;
    }
    symbols_parse_data_free (data);
    return TRUE;
}

static GList *
parse_prerequisites (const gchar *path, GError **error)
{
    PrerequisitesParseData *data;
    GMarkupParseContext *pcontext;
    GFile *file;
    GFileInputStream *input;
    GList *prerequisites;
    gboolean retval;

    file = g_file_new_for_path (path);

    input = g_file_read (file, NULL, error);
    g_object_unref (file);
    if (input == NULL)
        return FALSE;

    data = prerequisites_parse_data_new ();
    pcontext = g_markup_parse_context_new (&prerequisites_parser,
                                           0,
                                           data,
                                           NULL);
    retval = parse (pcontext, G_INPUT_STREAM (input), error);
    g_markup_parse_context_free (pcontext);
    g_object_unref (input);
    if (!retval) {
        prerequisites_parse_data_free (data);
        return NULL;
    }
    prerequisites = data->prerequisites;
    data->prerequisites = NULL;
    prerequisites_parse_data_free (data);
    return prerequisites;
}

static GList *
parse_keyboards (const gchar *path, GError **error)
{
    KeyboardsParseData *data;
    GMarkupParseContext *pcontext;
    GFile *file;
    GFileInputStream *input;
    GList *keyboards;
    gboolean retval;

    file = g_file_new_for_path (path);

    input = g_file_read (file, NULL, error);
    g_object_unref (file);
    if (input == NULL)
        return NULL;

    data = keyboards_parse_data_new ();
    pcontext = g_markup_parse_context_new (&keyboards_parser,
                                           0,
                                           data,
                                           NULL);
    retval = parse (pcontext, G_INPUT_STREAM (input), error);
    g_object_unref (input);
    g_markup_parse_context_free (pcontext);
    if (!retval) {
        keyboards_parse_data_free (data);
        return NULL;
    }

    keyboards = data->keyboards;
    data->keyboards = NULL;
    keyboards_parse_data_free (data);
    return keyboards;
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

static void scale_keyboard (EekKeyboard *keyboard,
                            gdouble      width,
                            gdouble      height)
{
    gdouble scale;
    EekBounds bounds;
    gsize n_outlines;
    guint i;

    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);

    if (width * bounds.height < height * bounds.width)
        scale = width / bounds.width;
    else
        scale = height / bounds.height;

    scale_bounds (EEK_ELEMENT(keyboard), scale);

    n_outlines = eek_keyboard_get_n_outlines (keyboard);
    for (i = 0; i < n_outlines; i++) {
        EekOutline *outline = eek_keyboard_get_outline (keyboard, i);
        gint j;
 
        for (j = 0; j < outline->num_points; j++) {
            outline->points[j].x *= scale;
            outline->points[j].y *= scale;
        }
    }
}

static gboolean
validate (const gchar **valid_path_list,
          gsize         valid_path_list_len,
          const gchar  *element_name,
          GSList       *element_stack,
          GError      **error)
{
    gint i;
    gchar *element_path;
    GSList *head, *p;
    GString *string;

    head = g_slist_prepend (element_stack, (gchar *)element_name);
    string = g_string_sized_new (64);
    for (p = head; p; p = p->next) {
        g_string_append (string, p->data);
        if (g_slist_next (p))
            g_string_append (string, "/");
    }
    element_path = g_string_free (string, FALSE);
    g_slist_free1 (head);

    for (i = 0; i < valid_path_list_len; i++)
        if (g_strcmp0 (element_path, valid_path_list[i]) == 0)
            break;
    g_free (element_path);

    if (i == valid_path_list_len) {
        gchar *reverse_element_path;

        head = g_slist_reverse (g_slist_copy (element_stack));
        string = g_string_sized_new (64);
        for (p = head; p; p = p->next) {
            g_string_append (string, p->data);
            if (g_slist_next (p))
                g_string_append (string, "/");
        }
        reverse_element_path = g_string_free (string, FALSE);

        abort ();
        g_set_error (error,
                     G_MARKUP_ERROR,
                     G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                     "%s cannot appear as %s",
                     element_name,
                     reverse_element_path);
        g_free (reverse_element_path);

        return FALSE;
    }
    return TRUE;
}

static gboolean
parse (GMarkupParseContext *pcontext,
       GInputStream        *input,
       GError             **error)
{
    gchar buffer[BUFSIZE];

    while (1) {
        gssize nread = g_input_stream_read (input,
                                            buffer,
                                            sizeof (buffer),
                                            NULL,
                                            error);
        if (nread < 0)
            return FALSE;

        if (nread == 0)
            break;

        if (!g_markup_parse_context_parse (pcontext, buffer, nread, error))
            return FALSE;
    }

    return g_markup_parse_context_end_parse (pcontext, error);
}

static const gchar *
get_attribute (const gchar **names, const gchar **values, const gchar *name)
{
    for (; *names && *values; names++, values++) {
        if (g_strcmp0 (*names, name) == 0)
            return *values;
    }
    return NULL;
}
