#ifndef N_JSON_MANAGER_H
#define N_JSON_MANAGER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define N_TYPE_JSON_MANAGER (n_json_manager_get_type ())
G_DECLARE_FINAL_TYPE (NJsonManager, n_json_manager, N, JSON_MANAGER, GObject)

typedef enum
{
    DUNST = 1,
    GENERIC
} N_JSON_MANAGER_PARSER_TYPE;

NJsonManager *n_json_manager_new (void);
gint n_json_manager_set_n_list (NJsonManager *self, GList **n_list);
gint n_json_manager_set_file (
    NJsonManager *self, const gchar *file,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type
);
gint n_json_manager_load_from_file (NJsonManager *self, gboolean append);
gint n_json_manager_save_to_file (NJsonManager *self, gboolean append);

G_END_DECLS

#endif
