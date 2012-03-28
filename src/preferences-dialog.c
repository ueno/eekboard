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
#include <eek/eek.h>

struct _PreferencesDialog {
    GtkWidget *dialog;
    GtkWidget *repeat_toggle;
    GtkWidget *repeat_delay_scale;
    GtkWidget *repeat_speed_scale;
    GtkWidget *auto_hide_toggle;
    GtkWidget *auto_hide_delay_scale;
    GtkWidget *selected_keyboards_treeview;
    GtkWidget *up_button;
    GtkWidget *down_button;
    GtkWidget *add_button;
    GtkWidget *remove_button;

    GtkWidget *new_keyboard_dialog;
    GtkWidget *available_keyboards_treeview;

    GList *available_keyboards;
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

static void
add_keyboard_to_treeview (GtkTreeView *treeview,
                          const gchar *id,
                          const gchar *longname)
{
    GtkTreeModel *model = gtk_tree_view_get_model (treeview);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter_first (model, &iter)) {
        do {
            gchar *_id;
            gtk_tree_model_get (model, &iter, 0, &_id, -1);
            if (g_strcmp0 (id, _id) == 0) {
                g_free (_id);
                return;
            }
            g_free (_id);
        } while (gtk_tree_model_iter_next (model, &iter));
    }

    gtk_list_store_append (GTK_LIST_STORE(model),
                           &iter);
    gtk_list_store_set (GTK_LIST_STORE(model),
                        &iter,
                        0, id,
                        1, longname,
                        -1);
}

static void
add_keyboard (GtkWidget *button, PreferencesDialog *dialog)
{
    gint retval = gtk_dialog_run (GTK_DIALOG(dialog->new_keyboard_dialog));
    if (retval == GTK_RESPONSE_OK) {
        GtkTreeSelection *selection;
        GtkTreeModel *model;
        GList *rows, *p;

        selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dialog->available_keyboards_treeview));
        rows = gtk_tree_selection_get_selected_rows (selection, &model);
        for (p = rows; p; p = p->next) {
            GtkTreeIter iter;
            if (gtk_tree_model_get_iter (model, &iter, p->data)) {
                gchar *id, *longname;
                gtk_tree_model_get (model, &iter, 0, &id, 1, &longname, -1);
                add_keyboard_to_treeview (GTK_TREE_VIEW(dialog->selected_keyboards_treeview),
                                          id,
                                          longname);
                g_free (id);
                g_free (longname);
            }
        }
    }
    gtk_widget_hide (dialog->new_keyboard_dialog);
}

static void
remove_keyboard (GtkWidget *button, PreferencesDialog *dialog)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rows, *p;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dialog->selected_keyboards_treeview));
    rows = gtk_tree_selection_get_selected_rows (selection, &model);
    for (p = rows; p; p = p->next) {
        GtkTreeIter iter;
        if (gtk_tree_model_get_iter (model, &iter, p->data))
            gtk_list_store_remove (GTK_LIST_STORE(model), &iter);
    }
}

static void
up_keyboard (GtkWidget *button, PreferencesDialog *dialog)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dialog->selected_keyboards_treeview));
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        GtkTreeIter prev = iter;
        if (gtk_tree_model_iter_previous (model, &prev))
            gtk_list_store_swap (GTK_LIST_STORE(model), &iter, &prev);
    }
}

static void
down_keyboard (GtkWidget *button, PreferencesDialog *dialog)
{
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(dialog->selected_keyboards_treeview));
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        GtkTreeIter next = iter;
        if (gtk_tree_model_iter_next (model, &next))
            gtk_list_store_swap (GTK_LIST_STORE(model), &iter, &next);
    }
}

static void
selection_changed_cb (GtkTreeSelection *selection, PreferencesDialog *dialog)
{
    gint count = gtk_tree_selection_count_selected_rows (selection);
    if (count > 0) {
        gtk_widget_set_sensitive (dialog->remove_button, TRUE);
        gtk_widget_set_sensitive (dialog->up_button, TRUE);
        gtk_widget_set_sensitive (dialog->down_button, TRUE);
    } else {
        gtk_widget_set_sensitive (dialog->remove_button, FALSE);
        gtk_widget_set_sensitive (dialog->up_button, FALSE);
        gtk_widget_set_sensitive (dialog->down_button, FALSE);
    }
}

static gint
compare_keyboard_id (const EekXmlKeyboardDesc *desc, const char *id)
{
    return g_strcmp0 (desc->id, id);
}

static void
populate_selected_keyboards (PreferencesDialog *dialog)
{
    gchar **strv, **p;

    strv = g_settings_get_strv (dialog->settings, "keyboards");
    for (p = strv; *p != NULL; p++) {
        GList *head = g_list_find_custom (dialog->available_keyboards,
                                            *p,
                                            (GCompareFunc) compare_keyboard_id);
        if (head == NULL) {
            g_warning ("unknown keyboard %s", *p);
        } else {
            EekXmlKeyboardDesc *desc = head->data;
            add_keyboard_to_treeview (GTK_TREE_VIEW(dialog->selected_keyboards_treeview),
                                      desc->id,
                                      desc->longname);
        }
    }
    g_strfreev (strv);
}

static void
populate_available_keyboards (PreferencesDialog *dialog)
{
    GList *p;

    for (p = dialog->available_keyboards; p; p = p->next) {
        EekXmlKeyboardDesc *desc = p->data;
        add_keyboard_to_treeview (GTK_TREE_VIEW(dialog->available_keyboards_treeview),
                                  desc->id,
                                  desc->longname);
    }
}

static void
save_keyboards (PreferencesDialog *dialog)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GList *list = NULL, *head;
    gchar **strv, **p;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW(dialog->selected_keyboards_treeview));
    if (gtk_tree_model_get_iter_first (model, &iter)) {
        do {
            gchar *id;
            gtk_tree_model_get (model, &iter, 0, &id, -1);
            list = g_list_prepend (list, id);
        } while (gtk_tree_model_iter_next (model, &iter));
    }
    list = g_list_reverse (list);

    strv = g_new0 (gchar *, g_list_length (list) + 1);
    for (head = list, p = strv; head; head = head->next, p++) {
        *p = head->data;
    }
    g_settings_set_strv (dialog->settings,
                         "keyboards",
                         (const gchar * const *)strv);
    g_strfreev (strv);
    g_list_free (list);
}

PreferencesDialog *
preferences_dialog_new (void)
{
    PreferencesDialog *dialog;
    gchar *ui_path;
    GtkBuilder *builder;
    GObject *object;
    GError *error;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeSelection *selection;

    dialog = g_slice_new0 (PreferencesDialog);
    dialog->settings = g_settings_new ("org.fedorahosted.eekboard");

    builder = gtk_builder_new ();
    gtk_builder_set_translation_domain (builder, "eekboard");
    ui_path = g_build_filename (PKGDATADIR,
                                "preferences-dialog.ui",
                                NULL);
    error = NULL;
    if (gtk_builder_add_from_file (builder, ui_path, &error) == 0) {
        g_warning ("can't load %s: %s", ui_path, error->message);
        g_error_free (error);
    }
    g_free (ui_path);

    object = gtk_builder_get_object (builder, "dialog");
    dialog->dialog = GTK_WIDGET(object);

    object = gtk_builder_get_object (builder, "repeat_toggle");
    dialog->repeat_toggle = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "repeat",
                     dialog->repeat_toggle, "active",
                     G_SETTINGS_BIND_DEFAULT);

    object = gtk_builder_get_object (builder, "repeat_delay_scale");
    dialog->repeat_delay_scale = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "repeat-delay",
                     gtk_range_get_adjustment (GTK_RANGE (dialog->repeat_delay_scale)), "value",
                     G_SETTINGS_BIND_DEFAULT);

    object = gtk_builder_get_object (builder, "repeat_speed_scale");
    dialog->repeat_speed_scale = GTK_WIDGET(object);

    g_settings_bind_with_mapping (dialog->settings, "repeat-interval",
                                  gtk_range_get_adjustment (GTK_RANGE (gtk_builder_get_object (builder, "repeat_speed_scale"))), "value",
                                  G_SETTINGS_BIND_DEFAULT,
                                  get_rate, set_rate, NULL, NULL);

    object = gtk_builder_get_object (builder, "auto_hide_toggle");
    dialog->auto_hide_toggle = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "auto-hide",
                     dialog->auto_hide_toggle, "active",
                     G_SETTINGS_BIND_DEFAULT);

    object = gtk_builder_get_object (builder, "auto_hide_delay_scale");
    dialog->auto_hide_delay_scale = GTK_WIDGET(object);

    g_settings_bind (dialog->settings, "auto-hide-delay",
                     gtk_range_get_adjustment (GTK_RANGE (dialog->auto_hide_delay_scale)), "value",
                     G_SETTINGS_BIND_DEFAULT);

    object = gtk_builder_get_object (builder,
                                     "selected_keyboards_treeview");
    dialog->selected_keyboards_treeview = GTK_WIDGET(object);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (object));
    g_signal_connect (G_OBJECT(selection), "changed",
                      G_CALLBACK(selection_changed_cb), dialog);

    renderer = GTK_CELL_RENDERER(gtk_cell_renderer_text_new ());
    column = gtk_tree_view_column_new_with_attributes ("Keyboard",
                                                       renderer,
                                                       "text",
                                                       1,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (object), column);

    object = gtk_builder_get_object (builder, "up_button");
    dialog->up_button = GTK_WIDGET(object);
    g_signal_connect (object, "clicked",
                      G_CALLBACK(up_keyboard), dialog);

    object = gtk_builder_get_object (builder, "down_button");
    dialog->down_button = GTK_WIDGET(object);
    g_signal_connect (object, "clicked",
                      G_CALLBACK(down_keyboard), dialog);

    object = gtk_builder_get_object (builder, "add_button");
    dialog->add_button = GTK_WIDGET(object);
    g_signal_connect (object, "clicked",
                      G_CALLBACK(add_keyboard), dialog);

    object = gtk_builder_get_object (builder, "remove_button");
    dialog->remove_button = GTK_WIDGET(object);
    g_signal_connect (object, "clicked",
                      G_CALLBACK(remove_keyboard), dialog);

    object = gtk_builder_get_object (builder, "new_keyboard_dialog");
    dialog->new_keyboard_dialog = GTK_WIDGET(object);

    object = gtk_builder_get_object (builder,
                                     "available_keyboards_treeview");
    dialog->available_keyboards_treeview = GTK_WIDGET(object);

    renderer = GTK_CELL_RENDERER(gtk_cell_renderer_text_new ());
    column = gtk_tree_view_column_new_with_attributes ("Keyboard",
                                                       renderer,
                                                       "text",
                                                       1,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (object), column);

    object = gtk_builder_get_object (builder,
                                     "available_keyboards_liststore");
    store = GTK_LIST_STORE(object);

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(store),
                                          1,
                                          GTK_SORT_ASCENDING);

    dialog->available_keyboards = eek_xml_list_keyboards ();

    populate_selected_keyboards (dialog);
    populate_available_keyboards (dialog);

    g_object_unref (builder);

    return dialog;
}

void
preferences_dialog_run (PreferencesDialog *dialog)
{
    gtk_window_present (GTK_WINDOW(dialog->dialog));
    gtk_dialog_run (GTK_DIALOG(dialog->dialog));
    save_keyboards (dialog);
}

void
preferences_dialog_free (PreferencesDialog *dialog)
{
    gtk_widget_destroy (dialog->dialog);
    // gtk_widget_destroy (dialog->new_keyboard_dialog);

    g_object_unref (dialog->settings);
    g_list_free_full (dialog->available_keyboards,
                      (GDestroyNotify) eek_xml_keyboard_desc_free);

    g_slice_free (PreferencesDialog, dialog);
}
