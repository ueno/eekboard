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

#include <stdio.h>
#include <stdarg.h>
#include <glib/gprintf.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-section.h"
#include "eek-key.h"
#include "eek-xml.h"

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
    GArray *outline_array;
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
    EekOutline *outline;
    gint i, num_groups, num_levels, num_keysyms;
    guint *keysyms;
    gint column, row;

    eek_key_get_index (EEK_KEY(element), &column, &row);
    g_string_append_indent (data->output, data->indent);
    if (eek_element_get_name (element))
        g_string_markup_printf (data->output,
                                "<key column=\"%d\" row=\"%d\" name=\"%s\">\n",
                                column, row, eek_element_get_name (element));
    else
        g_string_markup_printf (data->output,
                                "<key column=\"%d\" row=\"%d\">\n",
                                column, row);

    eek_element_get_bounds (element, &bounds);
    g_string_append_indent (data->output, data->indent + 1);
    output_bounds (data->output, &bounds);

    outline = eek_key_get_outline (EEK_KEY(element));
    if (outline) {
        for (i = 0;
             i < data->outline_array->len &&
                 g_array_index (data->outline_array, gpointer, i) != outline;
             i++)
            ;
        if (i == data->outline_array->len)
            g_array_append_val (data->outline_array, outline);
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output,
                                "<outline-ref>outline%d</outline-ref>\n",
                                i);
    }

    eek_key_get_keysyms (EEK_KEY(element), NULL, &num_groups, &num_levels);
    num_keysyms = num_groups * num_levels;
    if (num_keysyms > 0) {
        keysyms = g_slice_alloc (num_keysyms * sizeof(guint));
        eek_key_get_keysyms (EEK_KEY(element), &keysyms, NULL, NULL);
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output,
                                "<symbols groups=\"%d\" levels=\"%d\">\n",
                                num_groups, num_levels);

        for (i = 0; i < num_groups * num_levels; i++) {
            g_string_append_indent (data->output, data->indent + 2);
            if (keysyms[i] != EEK_INVALID_KEYSYM) {
                gchar *name = eek_xkeysym_to_string (keysyms[i]);

                if (name) {
                    g_string_markup_printf (data->output,
                                            "<xkeysym>%s</xkeysym>\n",
                                            name);
                    g_free (name);
                } else
                    g_string_markup_printf (data->output,
                                            "<invalid/>\n");
            } else
                g_string_markup_printf (data->output,
                                        "<invalid/>\n");
        }
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output, "</symbols>\n");
        g_slice_free1 (num_keysyms * sizeof(guint), keysyms);
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

void
eek_keyboard_output (EekKeyboard *keyboard, GString *output, gint indent)
{
    OutputCallbackData data;
    EekBounds bounds;
    gint i;

    g_assert (EEK_IS_KEYBOARD(keyboard));
 
    g_string_append_indent (output, indent);
    g_string_markup_printf (output, "<?xml version=\"1.0\"?>\n"
                            "<keyboard version=\""
                            EEK_XML_SCHEMA_VERSION
                            "\">\n");

    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);
    g_string_append_indent (output, indent + 1);
    output_bounds (output, &bounds);

    data.output = output;
    data.indent = indent;
    data.outline_array = g_array_new (FALSE, FALSE, sizeof (gpointer));

    data.indent++;
    eek_container_foreach_child (EEK_CONTAINER(keyboard),
                                 output_section_callback,
                                 &data);
    data.indent--;

    for (i = 0; i < data.outline_array->len; i++) {
        EekOutline *outline;
        gint j;

        g_string_append_indent (output, indent + 1);
        g_string_markup_printf (output, "<outline id=\"outline%d\">\n", i);

        outline = g_array_index (data.outline_array, gpointer, i);
        for (j = 0; j < outline->num_points; j++) {
            g_string_append_indent (output, indent + 2);
            g_string_markup_printf (output, "<point>%lf,%lf</point>\n",
                                    outline->points[j].x,
                                    outline->points[j].y);
        }

        g_string_append_indent (output, indent + 1);
        g_string_markup_printf (output, "</outline>\n");
    }
    g_array_free (data.outline_array, TRUE);

    g_string_append_indent (output, indent);
    g_string_markup_printf (output, "</keyboard>\n");
}
