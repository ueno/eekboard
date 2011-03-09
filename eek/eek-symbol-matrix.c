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
#include "eek-symbol-matrix.h"

EekSymbolMatrix *
eek_symbol_matrix_new (gint num_groups,
                       gint num_levels)
{
    EekSymbolMatrix *matrix = g_slice_new (EekSymbolMatrix);

    matrix->num_groups = num_groups;
    matrix->num_levels = num_levels;
    matrix->data = g_slice_alloc0 (sizeof (EekSymbol *) *
                                   num_groups * num_levels);
    return matrix;
}

EekSymbolMatrix *
eek_symbol_matrix_copy (const EekSymbolMatrix *matrix)
{
    EekSymbolMatrix *retval;
    gint i, num_symbols = matrix->num_groups * matrix->num_levels;

    retval = g_slice_dup (EekSymbolMatrix, matrix);
    retval->data = g_slice_copy (sizeof (EekSymbol *) * num_symbols,
                                 matrix->data);
    for (i = 0; i < num_symbols; i++)
        if (retval->data[i])
            g_object_ref (retval->data[i]);
    return retval;
}

void
eek_symbol_matrix_free (EekSymbolMatrix *matrix)
{
    gint i, num_symbols = matrix->num_groups * matrix->num_levels;
    for (i = 0; i < num_symbols; i++)
        if (matrix->data[i])
            g_object_unref (matrix->data[i]);
    g_slice_free1 (sizeof (EekSymbol *) * num_symbols, matrix->data);
    g_slice_free (EekSymbolMatrix, matrix);
}

GType
eek_symbol_matrix_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0)
        our_type =
            g_boxed_type_register_static ("EekSymbolMatrix",
                                          (GBoxedCopyFunc)eek_symbol_matrix_copy,
                                          (GBoxedFreeFunc)eek_symbol_matrix_free);
    return our_type;
}

void
eek_symbol_matrix_set_symbol (EekSymbolMatrix *matrix,
                              gint             group,
                              gint             level,
                              EekSymbol       *symbol)
{
    g_return_if_fail (group >= 0 && group < matrix->num_groups);
    g_return_if_fail (level >= 0 && level < matrix->num_levels);
    g_return_if_fail (EEK_IS_SYMBOL(symbol));
    matrix->data[group * matrix->num_levels + level] = g_object_ref (symbol);
}

/**
 * eek_symbol_matrix_get_symbol:
 * @matrix: an #EekSymbolMatrix
 * @group: group index of @matrix
 * @level: level index of @matrix
 *
 * Get an #EekSymbol stored in the cell selected by (@group, @level)
 * in @matrix.
 *
 * Return value: (transfer none): an #EekSymbol.
 */
EekSymbol *
eek_symbol_matrix_get_symbol (EekSymbolMatrix *matrix,
                              gint             group,
                              gint             level)
{
    g_return_val_if_fail (group >= 0 && group < matrix->num_groups, NULL);
    g_return_val_if_fail (level >= 0 && level < matrix->num_levels, NULL);
    return matrix->data[group * matrix->num_levels + level];
}
