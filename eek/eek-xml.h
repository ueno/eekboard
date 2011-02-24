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

#ifndef EEK_XML_H
#define EEK_XML_H 1

#include <glib-object.h>
#include "eek-keyboard.h"
#include "eek-xml-layout.h"

G_BEGIN_DECLS

#define EEK_XML_SCHEMA_VERSION "0.90"

void eek_keyboard_output (EekKeyboard *keyboard,
                          GString     *output,
                          gint         indent);

G_END_DECLS
#endif  /* EEK_XML_H */
