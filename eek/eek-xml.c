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

G_INLINE_FUNC void
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
    gint i, num_groups, num_levels;
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
                 g_array_index (data->outline_array, gpointer, i) == outline;
             i++)
            ;
        if (i == data->outline_array->len)
            g_array_append_val (data->outline_array, outline);
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output,
                                "<outline>outline%d</outline>\n",
                                i);
    }

    g_string_append_indent (data->output, data->indent + 1);
    g_string_markup_printf (data->output, "<keycode>%u</keycode>\n",
                            eek_key_get_keycode (EEK_KEY(element)));

    keysyms = NULL;
    eek_key_get_keysyms (EEK_KEY(element), &keysyms, &num_groups, &num_levels);
    if (keysyms) {
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output,
                                "<keysyms groups=\"%d\" levels=\"%d\">\n",
                                num_groups, num_levels);

        for (i = 0; i < num_groups * num_levels; i++) {
            g_string_append_indent (data->output, data->indent + 2);
            if (keysyms[i] != EEK_INVALID_KEYSYM) {
                gchar *name = eek_keysym_to_string (keysyms[i]);

                g_string_markup_printf (data->output,
                                        "<keysym name=\"%s\">%u</keysym>\n",
                                        name, keysyms[i]);
                g_free (name);
            } else
                g_string_markup_printf (data->output,
                                        "<keysym>%u</keysym>\n",
                                        keysyms[i]);
        }
        g_string_append_indent (data->output, data->indent + 1);
        g_string_markup_printf (data->output, "</keysyms>\n");
    }

    g_string_append_indent (data->output, data->indent);
    g_string_markup_printf (data->output, "</key>\n");
}

static void
output_section_callback (EekElement *element, gpointer user_data)
{
    OutputCallbackData *data = user_data;
    EekBounds bounds;
    gint angle;

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
    g_string_markup_printf (output, "<?xml version=\"1.0\"?>\n<keyboard>\n");

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
    g_array_free (data.outline_array, FALSE);

    g_string_append_indent (output, indent);
    g_string_markup_printf (output, "</keyboard>\n");
}
