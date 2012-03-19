/* 
 * Copyright (C) 2010-2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010-2011 Red Hat, Inc.
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
 * SECTION:eek-container
 * @short_description: Base class of a keyboard container
 *
 * The #EekContainerClass class represents a keyboard container, which
 * shall be used to implement #EekKeyboard and #EekSection.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-container.h"

enum {
    CHILD_ADDED,
    CHILD_REMOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_ABSTRACT_TYPE (EekContainer, eek_container, EEK_TYPE_ELEMENT);

#define EEK_CONTAINER_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_CONTAINER, EekContainerPrivate))


struct _EekContainerPrivate
{
    GList *head;
    GList *last;
};

static void
eek_container_real_add_child (EekContainer *self,
                              EekElement   *child)
{
    EekContainerPrivate *priv = EEK_CONTAINER_GET_PRIVATE(self);

    g_return_if_fail (EEK_IS_ELEMENT(child));
    g_object_ref (child);

    if (!priv->head) {
        priv->head = priv->last = g_list_prepend (priv->head, child);
    } else {
        priv->last->next = g_list_prepend (priv->last->next, child);
        priv->last = priv->last->next;
    }
    eek_element_set_parent (child, EEK_ELEMENT(self));
    g_signal_emit (self, signals[CHILD_ADDED], 0, child);
}

static void
eek_container_real_remove_child (EekContainer *self,
                                 EekElement   *child)
{
    EekContainerPrivate *priv = EEK_CONTAINER_GET_PRIVATE(self);
    GList *head;

    g_return_if_fail (EEK_IS_ELEMENT(child));
    head = g_list_find (priv->head, child);
    g_return_if_fail (head);
    g_object_unref (child);
    if (head == priv->last)
        priv->last = g_list_previous (priv->last);
    priv->head = g_list_remove_link (priv->head, head);
    eek_element_set_parent (child, NULL);
    g_signal_emit (self, signals[CHILD_REMOVED], 0, child);
}

static void
eek_container_real_foreach_child (EekContainer *self,
                                  EekCallback   callback,
                                  gpointer      user_data)
{
    EekContainerPrivate *priv = EEK_CONTAINER_GET_PRIVATE(self);
    GList *head;

    for (head = priv->head; head; head = g_list_next (head))
        (*callback) (EEK_ELEMENT(head->data), user_data);
}

static EekElement *
eek_container_real_find (EekContainer *self,
                         EekCompareFunc func,
                         gpointer user_data)
{
    EekContainerPrivate *priv = EEK_CONTAINER_GET_PRIVATE(self);
    GList *head;

    head = g_list_find_custom (priv->head, user_data, (GCompareFunc)func);
    if (head)
        return head->data;
    return NULL;
}

static void
eek_container_dispose (GObject *object)
{
    EekContainerPrivate *priv = EEK_CONTAINER_GET_PRIVATE(object);
    GList *head;

    for (head = priv->head; head; head = priv->head) {
        g_object_unref (head->data);
        priv->head = g_list_next (head);
        g_list_free1 (head);
    }
    G_OBJECT_CLASS(eek_container_parent_class)->dispose (object);
}

static void
eek_container_class_init (EekContainerClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (gobject_class,
                              sizeof (EekContainerPrivate));

    klass->add_child = eek_container_real_add_child;
    klass->remove_child = eek_container_real_remove_child;
    klass->foreach_child = eek_container_real_foreach_child;
    klass->find = eek_container_real_find;

    /* signals */
    klass->child_added = NULL;
    klass->child_removed = NULL;

    gobject_class->dispose = eek_container_dispose;

    /**
     * EekContainer::child-added:
     * @container: an #EekContainer
     * @element: an #EekElement
     *
     * The ::child-added signal is emitted each time an element is
     * added to @container.
     */
    signals[CHILD_ADDED] =
        g_signal_new (I_("child-added"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekContainerClass, child_added),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1,
                      EEK_TYPE_ELEMENT);

    /**
     * EekContainer::child-removed:
     * @container: an #EekContainer
     * @element: an #EekElement
     *
     * The ::child-removed signal is emitted each time an element is
     * removed from @container.
     */
    signals[CHILD_REMOVED] =
        g_signal_new (I_("child-removed"),
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET(EekContainerClass, child_removed),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE, 1,
                      EEK_TYPE_ELEMENT);
}

static void
eek_container_init (EekContainer *self)
{
    self->priv = EEK_CONTAINER_GET_PRIVATE(self);
}

/**
 * eek_container_foreach_child:
 * @container: an #EekContainer
 * @callback: (scope call): an #EekCallback
 * @user_data: additional data passed to @callback
 *
 * Enumerate children of @container and run @callback with each child.
 */
void
eek_container_foreach_child (EekContainer *container,
                             EekCallback   callback,
                             gpointer      user_data)
{
    g_return_if_fail (EEK_IS_CONTAINER(container));
    EEK_CONTAINER_GET_CLASS(container)->foreach_child (container,
                                                       callback,
                                                       user_data);
}

/**
 * eek_container_find:
 * @container: an #EekContainer
 * @func: function to be used to compare two children
 * @user_data: additional data passed to @func
 *
 * Find a child which matches the criteria supplied as @func, in @container.
 * Returns: an #EekElement or NULL on failure
 */
EekElement *
eek_container_find (EekContainer  *container,
                    EekCompareFunc func,
                    gpointer       user_data)
{
    g_return_val_if_fail (EEK_IS_CONTAINER(container), NULL);
    return EEK_CONTAINER_GET_CLASS(container)->find (container,
                                                     func,
                                                     user_data);
}

void
eek_container_add_child (EekContainer *container, EekElement *element)
{
    g_return_if_fail (EEK_IS_CONTAINER(container));
    g_return_if_fail (EEK_IS_ELEMENT(element));
    return EEK_CONTAINER_GET_CLASS(container)->add_child (container, element);
}
