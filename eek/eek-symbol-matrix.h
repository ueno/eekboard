/* 
 * Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2011 Red Hat, Inc.
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
#ifndef EEK_SYMBOL_MATRIX_H
#define EEK_SYMBOL_MATRIX_H 1

#include "eek-symbol.h"

G_BEGIN_DECLS

/**
 * EekSymbolMatrix:
 * @data: array of symbols
 * @num_groups: the number of groups (rows)
 * @num_levels: the number of levels (columns)
 *
 * Symbol matrix of a key.
 */
struct _EekSymbolMatrix
{
    /*< public >*/
    gint num_groups;
    gint num_levels;
    EekSymbol **data;
};

#define EEK_TYPE_SYMBOL_MATRIX (eek_symbol_matrix_get_type ())
GType            eek_symbol_matrix_get_type (void) G_GNUC_CONST;
EekSymbolMatrix *eek_symbol_matrix_new      (gint                   num_groups,
                                             gint                   num_levels);
EekSymbolMatrix *eek_symbol_matrix_copy     (const EekSymbolMatrix *matrix);
void             eek_symbol_matrix_free     (EekSymbolMatrix       *matrix);

void             eek_symbol_matrix_set_symbol
                                            (EekSymbolMatrix       *matrix,
                                             gint                   group,
                                             gint                   level,
                                             EekSymbol             *symbol);
EekSymbol       *eek_symbol_matrix_get_symbol
                                            (EekSymbolMatrix       *matrix,
                                             gint                   group,
                                             gint                   level);

G_END_DECLS

#endif  /* EEK_SYMBOL_MATRIX_H */
