#ifndef EEK_DRAWING_H
#define EEK_DRAWING_H 1

#include <pango/pangocairo.h>
#include "eek-keyboard.h"
#include "eek-keysym.h"
#include "eek-types.h"
#include "eek-theme-node.h"

G_BEGIN_DECLS

void eek_draw_text_on_layout  (PangoLayout           *layout,
                               const gchar           *text);

void eek_get_fonts            (EekKeyboard           *keyboard,
                               PangoLayout           *layout,
                               PangoFontDescription **fonts);

void eek_draw_outline         (cairo_t               *cr,
                               EekOutline            *outline,
                               EekGradientType        gradient_type,
                               EekColor              *gradient_start,
                               EekColor              *gradient_end);

void eek_draw_key_label       (cairo_t               *cr,
                               EekKey                *key,
                               PangoFontDescription **fonts);

void eek_draw_rounded_polygon (cairo_t               *cr,
                               gboolean               filled,
                               gdouble                radius,
                               EekPoint              *points,
                               gint                   num_points);

G_END_DECLS
#endif  /* EEK_DRAWING_H */
