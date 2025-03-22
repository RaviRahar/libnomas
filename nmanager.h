#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include "njsonmanager.h"
#include "nlistener.h"
#include "nobject.h"

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define N_TYPE_MANAGER (n_manager_get_type ())
G_DECLARE_FINAL_TYPE (NManager, n_manager, N, MANAGER, GObject)

NManager *n_manager_new (void);

gint n_manager_set_history_file (
    NManager *self, const gchar *history_file,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type
);
gint n_manager_set_callback (
    NManager *self, void (*callback) (NObject *, gpointer),
    gpointer callback_args
);
gint n_manager_start_listener (NManager *self);
gint n_manager_stop_listener (NManager *self);
gint n_manager_clear_state (NManager *self);
// wrapper aroud n_manager_save_to_file
gint n_manager_save_state (NManager *self);
// wrapper aroud n_manager_load_from_file
gint n_manager_load_state (NManager *self);
gint n_manager_save_to_file (
    NManager *self, const gchar *filename,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type, gboolean append
);
gint n_manager_load_from_file (
    NManager *self, const gchar *filename,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type, gboolean append
);
gint n_manager_notification_next (NManager *self, NObject *n_object);
gint n_manager_notification_prev (NManager *self, NObject *n_object);

// Returned string is still owned by NManager
// It is freed when destructing NManager
gchar *n_manager_print_n_list (NManager *self);

G_END_DECLS

#endif
