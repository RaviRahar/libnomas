#ifndef NOTIFICATION_LISTENER_H
#define NOTIFICATION_LISTENER_H

#include "nobject.h"
#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define N_TYPE_LISTENER (n_listener_get_type ())
G_DECLARE_FINAL_TYPE (NListener, n_listener, N, LISTENER, GObject)

NListener *n_listener_new (void);
gint n_listener_set_callback (
    NListener *self, void (*callback) (NObject *, gpointer),
    gpointer callback_args
);
gint n_listener_start (NListener *self);
gint n_listener_stop (NListener *self);

G_END_DECLS

#endif
