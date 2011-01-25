/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

/**
 * SECTION:eek-keyboard
 * @short_description: Base class of a keyboard
 * @see_also: #EekSection
 *
 * The #EekKeyboardClass class represents a keyboard, which consists
 * of one or more sections of the #EekSectionClass class.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-keyboard.h"
#include "eek-section.h"
#include "eek-key.h"
#include "eek-marshallers.h"

enum {
    PROP_0,
    PROP_GROUP,
    PROP_LEVEL,
    PROP_LAYOUT,
    PROP_LAST
};

enum {
    KEY_PRESSED,
    KEY_RELEASED,
    KEYSYM_INDEX_CHANGED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (EekKeyboard, eek_keyboard, EEK_TYPE_CONTAINER);

#define EEK_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_KEYBOARD, EekKeyboardPrivate))


struct _EekKeyboardPrivate
{
    gint group;
    gint level;
    EekLayout *layout;
};

struct _SetKeysymIndexCallbackData {
    gint group;
    gint level;
};
typedef struct _SetKeysymIndexCallbackData SetKeysymIndexCallbackData;

static void
set_keysym_index_for_key (EekElement *element,
                          gpointer    user_data)
{
    SetKeysymIndexCallbackData *data;

    g_return_if_fail (EEK_IS_KEY(element));

    data = user_data;
    eek_key_set_keysym_index (EEK_KEY(element), data->group, data->level);
}

static void
set_keysym_index_for_section (EekElement *element,
                              gpointer user_data)
{
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 set_keysym_index_for_key,
                                 user_data);
}

static void
eek_keyboard_real_set_keysym_index (EekKeyboard *self,
                                    gint         group,
                                    gint         level)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(self);
    SetKeysymIndexCallbackData data;

    if (priv->group != group || priv->level != level) {
        data.group = priv->group = group;
        data.level = priv->level = level;

        eek_container_foreach_child (EEK_CONTAINER(self),
                                     set_keysym_index_for_section,
                                     &data);

        g_signal_emit_by_name (self, "keysym-index-changed", group, level);
    }
}

void
eek_keyboard_real_get_keysym_index (EekKeyboard *self,
                                    gint        *group,
                                    gint        *level)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (group || level);
    if (group)
        *group = priv->group;
    if (level)
        *level = priv->level;
}

static void
key_pressed_event (EekSection  *section,
                   EekKey      *key,
                   EekKeyboard *keyboard)
{
    g_signal_emit_by_name (keyboard, "key-pressed", key);
}

static void
key_released_event (EekSection  *section,
                    EekKey      *key,
                    EekKeyboard *keyboard)
{
    g_signal_emit_by_name (keyboard, "key-released", key);
}

static EekSection *
eek_keyboard_real_create_section (EekKeyboard *self)
{
    EekSection *section;

    section = g_object_new (EEK_TYPE_SECTION, NULL);
    g_return_val_if_fail (section, NULL);

    g_signal_connect (section, "key-pressed",
                      G_CALLBACK(key_pressed_event), self);
    g_signal_connect (section, "key-released",
                      G_CALLBACK(key_released_event), self);

    EEK_CONTAINER_GET_CLASS(self)->add_child (EEK_CONTAINER(self),
                                              EEK_ELEMENT(section));
    return section;
}

struct _FindKeyByKeycodeCallbackData {
    EekKey *key;
    guint keycode;
};
typedef struct _FindKeyByKeycodeCallbackData FindKeyByKeycodeCallbackData;

static gint
find_key_by_keycode_section_callback (EekElement *element, gpointer user_data)
{
    FindKeyByKeycodeCallbackData *data = user_data;

    data->key = eek_section_find_key_by_keycode (EEK_SECTION(element),
                                                 data->keycode);
    if (data->key)
        return 0;
    return -1;
}

static EekKey *
eek_keyboard_real_find_key_by_keycode (EekKeyboard *self,
                                       guint        keycode)
{
    FindKeyByKeycodeCallbackData data;

    data.keycode = keycode;
    if (eek_container_find (EEK_CONTAINER(self),
                            find_key_by_keycode_section_callback,
                            &data))
        return data.key;
    return NULL;
}

static void
eek_keyboard_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_GROUP:
        eek_keyboard_set_group (EEK_KEYBOARD(object), g_value_get_int (value));
        break;
    case PROP_LEVEL:
        eek_keyboard_set_level (EEK_KEYBOARD(object), g_value_get_int (value));
        break;
    case PROP_LAYOUT:
        priv->layout = g_value_get_object (value);
        g_object_ref (priv->layout);
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_keyboard_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(object);

    switch (prop_id) {
    case PROP_GROUP:
        g_value_set_int (value,
                         eek_keyboard_get_group (EEK_KEYBOARD(object)));
        break;
    case PROP_LEVEL:
        g_value_set_int (value,
                         eek_keyboard_get_level (EEK_KEYBOARD(object)));
        break;
    case PROP_LAYOUT:
        g_value_set_object (value, priv->layout);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_keyboard_real_keysym_index_changed (EekKeyboard *self,
                                        gint         group,
                                        gint         level)
{
    /* g_debug ("keysym-index-changed"); */
}

static void
eek_keyboard_class_init (EekKeyboardClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekKeyboardPrivate));

    klass->set_keysym_index = eek_keyboard_real_set_keysym_index;
    klass->get_keysym_index = eek_keyboard_real_get_keysym_index;
    klass->create_section = eek_keyboard_real_create_section;
    klass->find_key_by_keycode = eek_keyboard_real_find_key_by_keycode;

    klass->keysym_index_changed = eek_keyboard_real_keysym_index_changed;

    gobject_class->get_property = eek_keyboard_get_property;
    gobject_class->set_property = eek_keyboard_set_property;

    /**
     * EekKeyboard:group:
     *
     * The group (row) index of symbol matrix of #EekKeyboard.
     */
    pspec = g_param_spec_int ("group",
                              "Group",
                              "Group index of symbol matrix of the keyboard",
                              0, G_MAXINT, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_GROUP,
                                     pspec);

    /**
     * EekKeyboard:level:
     *
     * The level (row) index of symbol matrix of #EekKeyboard.
     */
    pspec = g_param_spec_int ("level",
                              "Level",
                              "Level index of symbol matrix of the keyboard",
                              0, G_MAXINT, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_LEVEL,
                                     pspec);

    /**
     * EekKeyboard:layout:
     *
     * The layout used to create this #EekKeyboard.
     */
    pspec = g_param_spec_object ("layout",
                                 "Layout",
                                 "Layout used to create the keyboard",
                                 EEK_TYPE_LAYOUT,
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_LAYOUT,
                                     pspec);

    /**
     * EekKeyboard::key-pressed:
     * @keyboard: an #EekKeyboard
     * @key: an #EekKey
     *
     * The ::key-pressed signal is emitted each time a key in @keyboard
     * is shifted to the pressed state.
     */
    signals[KEY_PRESSED] =
        g_signal_new ("key-pressed",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE,
                      1,
                      EEK_TYPE_KEY);

    /**
     * EekKeyboard::key-released:
     * @keyboard: an #EekKeyboard
     * @key: an #EekKey
     *
     * The ::key-released signal is emitted each time a key in @keyboard
     * is shifted to the released state.
     */
    signals[KEY_RELEASED] =
        g_signal_new ("key-released",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE,
                      1,
                      EEK_TYPE_KEY);

    /**
     * EekKeyboard::keysym-index-changed:
     * @keyboard: an #EekKeyboard
     * @group: row index of the symbol matrix of keys on @keyboard
     * @level: column index of the symbol matrix of keys on @keyboard
     *
     * The ::keysym-index-changed signal is emitted each time the
     * global configuration of group/level index changes.
     */
    signals[KEYSYM_INDEX_CHANGED] =
        g_signal_new ("keysym-index-changed",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET(EekKeyboardClass, keysym_index_changed),
                      NULL,
                      NULL,
                      _eek_marshal_VOID__INT_INT,
                      G_TYPE_NONE,
                      2,
                      G_TYPE_INT,
                      G_TYPE_INT);
}

static void
eek_keyboard_init (EekKeyboard *self)
{
    EekKeyboardPrivate *priv;

    priv = self->priv = EEK_KEYBOARD_GET_PRIVATE(self);
    priv->group = priv->level = 0;
    priv->layout = NULL;
}

/**
 * eek_keyboard_set_keysym_index:
 * @keyboard: an #EekKeyboard
 * @group: row index of the symbol matrix of keys on @keyboard
 * @level: column index of the symbol matrix of keys on @keyboard
 *
 * Select a cell of the symbol matrix of each key on @keyboard.
 */
void
eek_keyboard_set_keysym_index (EekKeyboard *keyboard,
                               gint         group,
                               gint         level)
{
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));
    EEK_KEYBOARD_GET_CLASS(keyboard)->set_keysym_index (keyboard, group, level);
}

/**
 * eek_keyboard_get_keysym_index:
 * @keyboard: an #EekKeyboard
 * @group: a pointer where row index of the symbol matrix of keys on
 * @keyboard will be stored
 * @level: a pointer where column index of the symbol matrix of keys
 * on @keyboard will be stored
 *
 * Get the current cell position of the symbol matrix of each key on @keyboard.
 */
void
eek_keyboard_get_keysym_index (EekKeyboard *keyboard,
                               gint        *group,
                               gint        *level)
{
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));
    EEK_KEYBOARD_GET_CLASS(keyboard)->get_keysym_index (keyboard, group, level);
}

/**
 * eek_keyboard_set_group:
 * @keyboard: an #EekKeyboard
 * @group: group index of @keyboard
 *
 * Set the group index of symbol matrix of @keyboard.
 */
void
eek_keyboard_set_group (EekKeyboard *keyboard,
                        gint         group)
{
    gint level = eek_keyboard_get_level (keyboard);
    eek_keyboard_set_keysym_index (keyboard, group, level);
}

/**
 * eek_keyboard_set_level:
 * @keyboard: an #EekKeyboard
 * @level: level index of @keyboard
 *
 * Set the level index of symbol matrix of @keyboard.
 */
void
eek_keyboard_set_level (EekKeyboard *keyboard,
                        gint         level)
{
    gint group = eek_keyboard_get_group (keyboard);
    eek_keyboard_set_keysym_index (keyboard, group, level);
}

/**
 * eek_keyboard_get_group:
 * @keyboard: an #EekKeyboard
 *
 * Return the group index of @keyboard.
 */
gint
eek_keyboard_get_group (EekKeyboard *keyboard)
{
    gint group;
    eek_keyboard_get_keysym_index (keyboard, &group, NULL);
    return group;
}

/**
 * eek_keyboard_get_level:
 * @keyboard: an #EekKeyboard
 *
 * Return the level index of @keyboard.
 */
gint
eek_keyboard_get_level (EekKeyboard *keyboard)
{
    gint level;
    eek_keyboard_get_keysym_index (keyboard, NULL, &level);
    return level;
}

/**
 * eek_keyboard_create_section:
 * @keyboard: an #EekKeyboard
 *
 * Create an #EekSection instance and append it to @keyboard.  This
 * function is rarely called by application but called by #EekLayout
 * implementation.
 */
EekSection *
eek_keyboard_create_section (EekKeyboard *keyboard)
{
    EekSection *section;
    g_return_val_if_fail (EEK_IS_KEYBOARD(keyboard), NULL);
    section = EEK_KEYBOARD_GET_CLASS(keyboard)->create_section (keyboard);
    return section;
}

/**
 * eek_keyboard_find_key_by_keycode:
 * @keyboard: an #EekKeyboard
 * @keycode: a keycode
 *
 * Find an #EekKey whose keycode is @keycode.
 * Returns: an #EekKey or NULL (if not found)
 */
EekKey *
eek_keyboard_find_key_by_keycode (EekKeyboard *keyboard,
                                  guint        keycode)
{
    g_assert (EEK_IS_KEYBOARD(keyboard));
    return EEK_KEYBOARD_GET_CLASS(keyboard)->
        find_key_by_keycode (keyboard, keycode);
}

EekLayout *
eek_keyboard_get_layout (EekKeyboard *keyboard)
{
    EekKeyboardPrivate *priv;

    g_assert (EEK_IS_KEYBOARD(keyboard));
    priv = EEK_KEYBOARD_GET_PRIVATE(keyboard);
    return priv->layout;
}

void
eek_keyboard_get_size (EekKeyboard *keyboard,
                       gdouble     *width,
                       gdouble     *height)
{
    EekBounds bounds;

    g_assert (EEK_IS_KEYBOARD(keyboard));
    eek_element_get_bounds (EEK_ELEMENT(keyboard), &bounds);
    *width = bounds.width;
    *height = bounds.height;
}
