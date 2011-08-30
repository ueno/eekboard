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
 * SECTION: eek-xml
 * @title: XML Conversion Utilities
 * @short_description: #EekKeyboard to XML conversion utilities
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdarg.h>
#include <glib/gprintf.h>

#include "eek-section.h"
#include "eek-key.h"
#include "eek-xml.h"
#include "eek-keysym.h"
#include "eek-text.h"

#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

void
g_string_markup_printf (GString *output, const gchar *format, ...)
{
    gchar *escaped_text;
    va_list ap;

    va_start (ap, format);
    escaped_text = g_markup_vprintf_escaped (format, ap);
    va_end (ap);

    g_string_append (output, escaped_text);
    g_free (escaped_text);
}

struct _OutputCallbackData {
    GString *output;
    gint indent;
    GHashTable *oref_hash;
    gint key_serial;
};
typedef struct _OutputCallbackData OutputCallbackData;

static void
output_bounds (GString *output, EekBounds *bounds)
{
    g_string_markup_printf (output,
                            "<bounds>%lf,%lf,%lf,%lf</bounds>\n",
                            bounds->x,
                            bounds->y,
                            bounds->width,
                            bounds->height);
}

static void
output_key_callback (EekElement *element, gpointer user_data)
{
    OutputCallbackData *data = user_data;
    EekBounds bounds;
    gint i, num_symbols;
    EekSymbolMatrix *matrix;
    gint column, row;
    guint keycode;
    gchar *id;
    gulong oref;

    keycode = eek_key_get_keycode (EEK_KEY(element));
    if (keycode == EEK_INVALID_KEYCODE)
        id = g_strdup_printf ("key%d", data->key_serial);
    else
        id = g_strdup_printf ("keycode%d", keycode);
    data->key_serial++;

    eek_key_get_index (EEK_KEY(element), &column, &row);
    g_string_append_indent (data->output, data->indent);
    if (eek_element_get_name (element))
        g_string_markup_printf (data->output,
                                "<key id=\"%s\" name=\"%s\" "
                                "column=\"%d\" row=\"%d\">\n",
                                id,
                                eek_element_get_name (element),
                                column,
                                row);
    else
        g_string_markup_printf (data->output,
                                "<key id=\"%s\" "
                                "column=\"%d\" row=\"%d\">\n",
                                id,
                                column,
                                row);
    g_free (id);

    eek_element_get_bounds (element, &bounds);
    g_string_append_indent (data->output, data->indent + 1);
    output_bounds (data->output, &bounds);

    oref = eek_key_get_oref (EEK_KEY(element));
    if (oref != 0) {
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output,
                                "<oref>outline%u</oref>\n",
                                oref);
        if (!g_hash_table_lookup (data->oref_hash, (gpointer)oref))
            g_hash_table_insert (data->oref_hash,
                                 (gpointer)oref,
                                 (gpointer)TRUE);
    }

    matrix = eek_key_get_symbol_matrix (EEK_KEY(element));
    num_symbols = matrix->num_groups * matrix->num_levels;
    if (num_symbols > 0) {
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output,
                                "<symbols groups=\"%d\" levels=\"%d\">\n",
                                matrix->num_groups, matrix->num_levels);

        for (i = 0; i < num_symbols; i++) {
            EekSymbol *symbol = matrix->data[i];

            g_string_append_indent (data->output, data->indent + 2);
            if (EEK_IS_KEYSYM(symbol)) {
                guint xkeysym = eek_keysym_get_xkeysym (EEK_KEYSYM(symbol));

                if (xkeysym != EEK_INVALID_KEYSYM)
                    g_string_markup_printf
                        (data->output,
                         "<keysym keyval=\"%u\">%s</keysym>\n",
                         xkeysym,
                         eek_symbol_get_name (symbol));
                else
                    g_string_markup_printf (data->output,
                                            "<keysym>%s</keysym>\n",
                                            eek_symbol_get_name (symbol));
            }
            else if (EEK_IS_TEXT(symbol)) {
                g_string_markup_printf (data->output,
                                        "<text>%s</text>\n",
                                        eek_text_get_text (EEK_TEXT(symbol)));
            }
            else
                g_string_markup_printf (data->output,
                                        "<symbol>%s</symbol>\n",
                                        eek_symbol_get_name (symbol));
        }
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output, "</symbols>\n");
    }

    g_string_append_indent (data->output, data->indent);
    g_string_markup_printf (data->output, "</key>\n");
}

static void
output_section_callback (EekElement *element, gpointer user_data)
{
    OutputCallbackData *data = user_data;
    EekBounds bounds;
    gint angle, n_rows, i;

    g_string_append_indent (data->output, data->indent);
    if (eek_element_get_name (element))
        g_string_markup_printf (data->output, "<section name=\"%s\">\n",
                                eek_element_get_name (element));
    else
        g_string_markup_printf (data->output, "<section>\n");

    eek_element_get_bounds (element, &bounds);
    g_string_append_indent (data->output, data->indent + 1);
    output_bounds (data->output, &bounds);

    angle = eek_section_get_angle (EEK_SECTION(element));
    g_string_append_indent (data->output, data->indent + 1);
    g_string_markup_printf (data->output, "<angle>%d</angle>\n", angle);

    n_rows = eek_section_get_n_rows (EEK_SECTION(element));
    for (i = 0; i < n_rows; i++) {
        gint num_columns;
        EekOrientation orientation;

        eek_section_get_row (EEK_SECTION(element),
                             i,
                             &num_columns,
                             &orientation);
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output, "<row>\n");
        g_string_append_indent (data->output, data->indent + 2);
        g_string_markup_printf (data->output, "<columns>%d</columns>\n",
                                num_columns);
        g_string_append_indent (data->output, data->indent + 2);
        g_string_markup_printf (data->output, "<orientation>%d</orientation>\n",
                                orientation);
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output, "</row>\n");
    }
    
    data->indent++;
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 output_key_callback,
                                 data);
    data->indent--;

    g_string_append_indent (data->output, data->indent);
    g_string_markup_printf (data->output, "</section>\n");
}

/**
 * eek_keyboard_output:
 * @keyboard: an #EekKeyboard
 * @output: a GString
 * @indent: an integer
 *
 * Convert @keyboard into the XML format and store it into @output.
 */
void
eek_keyboard_output (EekKeyboard *keyboard, GString *output, gint indent)
{
    OutputCallbackData data;
    EekBounds bounds;
    gulong oref;
    GHashTableIter iter;

    g_assert (EEK_IS_KEYBOARD(keyboard));
 
    g_string_append_indent (output, indent);
    if (eek_element_get_name (EEK_ELEMENT(keyboard)))
        g_string_markup_printf (output, "<?xml version=\"1.0\"?>\n"
                                "<keyboard version=\"%s\" id=\"%s\">\n",
                                EEK_XML_SCHEMA_VERSION,
                                eek_element_get_name (EEK_ELEMENT(keyboard)));
    else
        g_string_markup_printf (output, "<?xml version=\"1.0\"?>\n"
                                "<keyboard version=\"%s\">\n",
                                EEK_XML_SCHEMA_VERSION);

    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);
    g_string_append_indent (output, indent + 1);
    output_bounds (output, &bounds);

    data.output = output;
    data.indent = indent;
    data.oref_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
    data.key_serial = 0;

    data.indent++;
    eek_container_foreach_child (EEK_CONTAINER(keyboard),
                                 output_section_callback,
                                 &data);
    data.indent--;

    g_hash_table_iter_init (&iter, data.oref_hash);
    while (g_hash_table_iter_next (&iter, (gpointer *)&oref, NULL)) {
        EekOutline *outline;
        gint j;

        outline = eek_keyboard_get_outline (keyboard, oref);
        g_string_append_indent (output, indent + 1);
        g_string_markup_printf (output, "<outline id=\"outline%u\">\n", oref);
        g_string_append_indent (output, indent + 2);
        g_string_markup_printf (output, "<corner-radius>%lf</corner-radius>\n",
                                outline->corner_radius);
        for (j = 0; j < outline->num_points; j++) {
            g_string_append_indent (output, indent + 2);
            g_string_markup_printf (output, "<point>%lf,%lf</point>\n",
                                    outline->points[j].x,
                                    outline->points[j].y);
        }

        g_string_append_indent (output, indent + 1);
        g_string_markup_printf (output, "</outline>\n");
    }
    g_hash_table_destroy (data.oref_hash);

    g_string_append_indent (output, indent);
    g_string_markup_printf (output, "</keyboard>\n");
}
