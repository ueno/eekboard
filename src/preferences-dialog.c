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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include <gtk/gtk.h>
#include "preferences-dialog.h"

struct _PreferencesDialog {
    GtkWidget *dialog;
    GtkWidget *repeat_toggle;
    GtkWidget *repeat_delay_scale;
    GtkWidget *repeat_speed_scale;
    GtkWidget *auto_hide_toggle;
    GtkWidget *auto_hide_delay_scale;
    GtkWidget *start_fullscreen_toggle;
    GtkWidget *keyboard_entry;

    GSettings *settings;
};

static gboolean
get_rate (GValue   *value,
          GVariant *variant,
          gpointer  user_data)
{
    int rate;
    gdouble fraction;

    rate = g_variant_get_uint32 (variant);
    fraction = 1.0 / ((gdouble) rate / 1000.0);
    g_value_set_double (value, fraction);
    return TRUE;
}

static GVariant *
set_rate (const GValue       *value,
          const GVariantType *expected_type,
          gpointer            user_data)
{
    gdouble rate;
    int msecs;

    rate = g_value_get_double (value);
    msecs = (1 / rate) * 1000;
    return g_variant_new_uint32 (msecs);
}

PreferencesDialog *
preferences_dialog_new (void)
{
    PreferencesDialog *dialog;
    gchar *ui_path;
    GtkBuilder *builder;
    GObject *object;
    GError *error;

    dialog = g_slice_new0 (PreferencesDialog);
    dialog->settings = g_settings_new ("org.fedorahosted.eekboard");

    builder = gtk_builder_new ();
    gtk_builder_set_translation_domain (builder, "eekboard");
    ui_path = g_strdup_printf ("%s/%s", PKGDATADIR, "preferences-dialog.ui");
    error = NULL;
    gtk_builder_add_from_file (builder, ui_path, &error);
    g_free (ui_path);

    object =
        gtk_builder_get_object (builder, "dialog");
    dialog->dialog = GTK_WIDGET(object);

    object =
        gtk_builder_get_object (builder, "repeat_toggle");
    dialog->repeat_toggle = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "repeat",
                     dialog->repeat_toggle, "active",
                     G_SETTINGS_BIND_DEFAULT);

    object =
        gtk_builder_get_object (builder, "repeat_delay_scale");
    dialog->repeat_delay_scale = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "repeat-delay",
                     gtk_range_get_adjustment (GTK_RANGE (dialog->repeat_delay_scale)), "value",
                     G_SETTINGS_BIND_DEFAULT);

    object =
        gtk_builder_get_object (builder, "repeat_speed_scale");
    dialog->repeat_speed_scale = GTK_WIDGET(object);

    g_settings_bind_with_mapping (dialog->settings, "repeat-interval",
                                  gtk_range_get_adjustment (GTK_RANGE (gtk_builder_get_object (builder, "repeat_speed_scale"))), "value",
                                  G_SETTINGS_BIND_DEFAULT,
                                  get_rate, set_rate, NULL, NULL);

    object =
        gtk_builder_get_object (builder, "auto_hide_toggle");
    dialog->auto_hide_toggle = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "auto-hide",
                     dialog->auto_hide_toggle, "active",
                     G_SETTINGS_BIND_DEFAULT);

    object =
        gtk_builder_get_object (builder, "auto_hide_delay_scale");
    dialog->auto_hide_delay_scale = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "auto-hide-delay",
                     gtk_range_get_adjustment (GTK_RANGE (dialog->auto_hide_delay_scale)), "value",
                     G_SETTINGS_BIND_DEFAULT);

    object =
        gtk_builder_get_object (builder, "start_fullscreen_toggle");
    dialog->start_fullscreen_toggle = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "start-fullscreen",
                     dialog->start_fullscreen_toggle, "active",
                     G_SETTINGS_BIND_DEFAULT);

    object =
        gtk_builder_get_object (builder, "keyboard_entry");
    dialog->keyboard_entry = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "keyboard",
                     GTK_ENTRY(dialog->keyboard_entry), "text",
                     G_SETTINGS_BIND_DEFAULT);

    return dialog;
}

void
preferences_dialog_run (PreferencesDialog *dialog)
{
    gtk_window_present (GTK_WINDOW(dialog->dialog));
    gtk_dialog_run (GTK_DIALOG(dialog->dialog));
    gtk_widget_destroy (dialog->dialog);
}
