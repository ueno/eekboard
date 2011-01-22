#ifndef EEK_RENDERER_H
#define EEK_RENDERER_H 1

#include <pango/pangocairo.h>
#include "eek-keyboard.h"
#include "eek-keysym.h"
#include "eek-types.h"

G_BEGIN_DECLS

#define EEK_TYPE_RENDERER (eek_renderer_get_type())
#define EEK_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_RENDERER, EekRenderer))
#define EEK_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_RENDERER, EekRendererClass))
#define EEK_IS_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_RENDERER))
#define EEK_IS_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_RENDERER))
#define EEK_RENDERER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_RENDERER, EekRendererClass))

typedef struct _EekRenderer EekRenderer;
typedef struct _EekRendererClass EekRendererClass;
typedef struct _EekRendererPrivate EekRendererPrivate;

struct _EekRenderer {
    GObject parent;

    EekRendererPrivate *priv;
};

struct _EekRendererClass
{
    GObjectClass parent_class;

    void (* render_key)      (EekRenderer *self,
                              cairo_t     *cr,
                              EekKey      *key,
                              gdouble      scale);

    void (* render_keyboard) (EekRenderer *self,
                              cairo_t     *cr);

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType        eek_renderer_get_type             (void) G_GNUC_CONST;
EekRenderer *eek_renderer_new                  (EekKeyboard  *keyboard,
                                                PangoContext *pcontext);
void         eek_renderer_set_preferred_size   (EekRenderer  *renderer,
                                                gdouble       width,
                                                gdouble       height);
void         eek_renderer_get_size             (EekRenderer  *renderer,
                                                gdouble      *width,
                                                gdouble      *height);
void         eek_renderer_get_key_bounds       (EekRenderer  *renderer,
                                                EekKey       *key,
                                                EekBounds    *bounds,
                                                gboolean      rotate);
gdouble      eek_renderer_get_scale            (EekRenderer  *renderer);

void         eek_renderer_render_key           (EekRenderer  *renderer,
                                                cairo_t      *cr,
                                                EekKey       *key,
                                                gdouble       scale);

void         eek_renderer_render_keyboard      (EekRenderer  *renderer,
                                                cairo_t      *cr);

void         eek_renderer_set_foreground       (EekRenderer  *renderer,
                                                EekColor     *foreground);
void         eek_renderer_set_background       (EekRenderer  *renderer,
                                                EekColor     *background);
void         eek_renderer_get_foreground       (EekRenderer  *renderer,
                                                EekColor     *foreground);
void         eek_renderer_get_background       (EekRenderer  *renderer,
                                                EekColor     *background);
void         eek_renderer_set_border_width     (EekRenderer  *renderer,
                                                gdouble       border_width);
EekKey      *eek_renderer_find_key_by_position (EekRenderer  *renderer,
                                                gdouble       x,
                                                gdouble       y);

G_END_DECLS
#endif  /* EEK_RENDERER_H */
