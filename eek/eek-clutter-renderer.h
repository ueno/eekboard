#ifndef EEK_CLUTTER_RENDERER_H
#define EEK_CLUTTER_RENDERER_H 1

#include "eek-renderer.h"

G_BEGIN_DECLS

#define EEK_TYPE_CLUTTER_RENDERER (eek_clutter_renderer_get_type())
#define EEK_CLUTTER_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEK_TYPE_CLUTTER_RENDERER, EekClutterRenderer))
#define EEK_CLUTTER_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEK_TYPE_CLUTTER_RENDERER, EekClutterRendererClass))
#define EEK_IS_CLUTTER_RENDERER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEK_TYPE_CLUTTER_RENDERER))
#define EEK_IS_CLUTTER_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EEK_TYPE_CLUTTER_RENDERER))
#define EEK_CLUTTER_RENDERER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEK_TYPE_CLUTTER_RENDERER, EekClutterRendererClass))

typedef struct _EekClutterRenderer EekClutterRenderer;
typedef struct _EekClutterRendererClass EekClutterRendererClass;
typedef struct _EekClutterRendererPrivate EekClutterRendererPrivate;

struct _EekClutterRenderer {
    EekRenderer parent;

    EekClutterRendererPrivate *priv;
};

struct _EekClutterRendererClass
{
    EekRendererClass parent_class;

    /*< private >*/
    /* padding */
    gpointer pdummy[24];
};

GType               eek_clutter_renderer_get_type
                                             (void) G_GNUC_CONST;
EekClutterRenderer *eek_clutter_renderer_new (EekKeyboard        *keyboard,
                                              PangoContext       *pcontext);
void                eek_clutter_renderer_render_key
                                             (EekClutterRenderer *renderer,
                                              ClutterActor       *actor,
                                              EekKey             *key);

G_END_DECLS
#endif  /* EEK_CLUTTER_RENDERER_H */
