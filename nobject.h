#ifndef N_OBJECT_H
#define N_OBJECT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define N_TYPE_OBJECT (n_object_get_type ())
G_DECLARE_FINAL_TYPE (NObject, n_object, N, OBJECT, GObject)

// format  value type
//
// b       const gchar *body
// m       const gchar *message
// s       const gchar *summary
// a       const gchar *appname
// c       const gchar *category
// d       const gchar *default_action_name,
// i       const gchar *icon_path
// x       gint id
// t       gint64 timestamp
// o       gint64 timeout
// p       gint progress
NObject *n_object_new (const gchar *format, ...);

const gchar *n_object_body (NObject *self);
const gchar *n_object_message (NObject *self);
const gchar *n_object_summary (NObject *self);
const gchar *n_object_appname (NObject *self);
const gchar *n_object_category (NObject *self);
const gchar *n_object_default_action_name (NObject *self);
const gchar *n_object_icon_path (NObject *self);
gint n_object_id (NObject *self);
gint64 n_object_timestamp (NObject *self);
gint64 n_object_timeout (NObject *self);
gint n_object_progress (NObject *self);

gint n_object_compare_by_timestamp (gconstpointer a, gconstpointer b);

G_END_DECLS

#endif
