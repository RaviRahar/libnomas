#include "nmanager.h"
#include "njsonmanager.h"
#include "nlistener.h"
#include "nobject.h"

#include <glib-object.h>
#include <glib.h>

enum object_access
{
    NONE,
    NEXT,
    PREV
};

struct _NManager
{
    GObject parent_instance;
    GList *n_list;
    gint index;
    const char *history_file;
    N_JSON_MANAGER_PARSER_TYPE parser_type;
    NJsonManager *n_jmanager;
    NListener *n_listener;
    void (*callback) (NObject *, gpointer);
    gpointer callback_args;

    // TRUE = next; FALSE = prev
    enum object_access last_access;
};

G_DEFINE_TYPE (NManager, n_manager, G_TYPE_OBJECT)

static void
n_manager_listener_callback (NObject *n_object, gpointer n_manager);

static void
n_manager_init (NManager *self)
{
    self->n_list = NULL;
    self->index = -1;
    self->history_file = NULL;
    self->parser_type = GENERIC;
    self->n_jmanager = n_json_manager_new ();
    self->n_listener = n_listener_new ();
    self->callback = NULL;
    self->callback_args = NULL;

    n_listener_set_callback (
        self->n_listener, n_manager_listener_callback, (gpointer)self
    );
    self->last_access = NONE;
}

static void
n_manager_class_finalize (GObject *object)
{
    NManager *self = N_MANAGER (object);
    if (self->n_jmanager)
        {
            g_object_unref (self->n_jmanager);
        }
    if (self->n_listener)
        {
            g_object_unref (self->n_listener);
        }
    if (self->n_list)
        {
            g_list_free_full (g_steal_pointer (&self->n_list), g_object_unref);
        }
    G_OBJECT_CLASS (n_manager_parent_class)->finalize (object);
}

static void
n_manager_class_init (NManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = n_manager_class_finalize;
}

NManager *
n_manager_new (void)
{
    return g_object_new (N_TYPE_MANAGER, NULL);
}

static void
n_manager_set_n_list (NManager *self, GList *n_list)
{
    self->n_list = n_list;
    n_json_manager_set_n_list (self->n_jmanager, &self->n_list);
}

static GList *
n_manager_n_list_append (NManager *self, NObject *n_object)
{
    self->n_list = g_list_append (self->n_list, n_object);
    if (!self->n_list)
        {
            n_manager_set_n_list (self, self->n_list);
        }

    g_warning ("%s", n_object_print (n_object)); // FIX: REMOVE

    return self->n_list;
}

static void
n_manager_listener_callback (NObject *n_object, gpointer n_manager)
{
    /*NManager *self*/
    NManager *self = N_MANAGER (n_manager);

    if (!n_manager_n_list_append (self, n_object))
        {
            g_warning ("n_manager_listener_callback: could not add new "
                       "notification to n_list");
        }

    if (!self->callback)
        {
            g_info (
                "n_manager_listener_callback: callback not set, nothing to do"
            );
            return;
        }
    self->callback (n_object, self->callback_args);
}

gint
n_manager_set_callback (
    NManager *self, void (*callback) (NObject *, gpointer),
    gpointer callback_args
)
{

    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_set_callback: argument is not NManager");
            return -1;
        }

    self->callback = callback;

    if (!self->callback)
        {
            g_warning ("n_manager_set_callback: setting callback failed");
            return -1;
        }

    self->callback_args = callback_args;

    return 0;
}

gint
n_manager_set_history_file (
    NManager *self, const gchar *history_file,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type
)
{

    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_set_history_file: argument is not NManager");
            return -1;
        }

    self->history_file = g_strdup (history_file);
    self->parser_type = j_parser_type;

    if (!self->history_file)
        {
            g_warning (
                "n_manager_set_history_file: setting history_file failed"
            );
            return -1;
        }

    return n_json_manager_set_file (
        self->n_jmanager, self->history_file, j_parser_type
    );
}

gint
n_manager_start_listener (NManager *self)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_start_listener: argument is not NManager");
            return -1;
        }
    if (!self->callback)
        {
            g_warning ("n_manager_start_listener: callback not set");
        }
    return n_listener_start (self->n_listener);
}

gint
n_manager_stop_listener (NManager *self)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_stop_listener: argument is not NManager");
            return -1;
        }
    return n_listener_stop (self->n_listener);
}

gint
n_manager_clear_state (NManager *self)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_clear_state: argument is not NManager");
            return -1;
        }
    n_manager_set_n_list (self, NULL);
    return 0;
};

gint
n_manager_save_state (NManager *self)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_save_state: argument is not NManager");
            return -1;
        }
    gboolean append = TRUE;
    return n_json_manager_save_to_file (self->n_jmanager, append);
}

gint
n_manager_load_state (NManager *self)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_load_state: argument is not NManager");
            return -1;
        }
    gboolean append = TRUE;
    return n_json_manager_load_from_file (self->n_jmanager, append);
}

gint
n_manager_save_to_file (
    NManager *self, const gchar *filename,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type, gboolean append
)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_save_to_file: argument is not NManager");
            return -1;
        }
    NJsonManager *temp_jmanager = n_json_manager_new ();
    n_json_manager_set_file (temp_jmanager, filename, j_parser_type);
    gint status = n_json_manager_save_to_file (temp_jmanager, append);
    g_object_unref (temp_jmanager);
    return status;
}

gint
n_manager_load_from_file (
    NManager *self, const gchar *filename,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type, gboolean append
)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_load_from_file: argument is not NManager");
            return -1;
        }
    n_json_manager_set_file (self->n_jmanager, filename, j_parser_type);
    gint status = n_json_manager_save_to_file (self->n_jmanager, append);
    n_json_manager_set_file (
        self->n_jmanager, self->history_file, self->parser_type
    );
    // TODO: better error handling
    return status;
}

gint
n_manager_n_object_next (NManager *self, NObject *n_object)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_n_object_next: argument is not NManager");
            return -1;
        };

    if (!N_IS_OBJECT (n_object))
        {
            g_warning ("n_manager_n_object_next: argument is not NObject");
            return -1;
        }

    if (self->last_access == PREV)
        {
            self->index++;
        }

    void *maybe_n_object = g_list_nth_data (self->n_list, self->index++);
    self->last_access = NEXT;

    if (!N_IS_OBJECT (maybe_n_object))
        {
            g_warning ("n_manager_n_object_next: n_list data is not NObject");
            return -1;
        }
    n_object = N_OBJECT (maybe_n_object);
    return 0;
}

gint
n_manager_n_object_prev (NManager *self, NObject *n_object)
{
    if (!N_IS_MANAGER (self))
        {
            g_warning ("n_manager_n_object_prev: argument is not NManager");
            return -1;
        }

    if (!N_IS_OBJECT (n_object))
        {
            g_warning ("n_manager_n_object_prev: argument is not NObject");
            return -1;
        }

    if (self->last_access == NEXT)
        {
            self->index--;
        }

    void *maybe_n_object = g_list_nth (self->n_list, self->index--);
    self->last_access = PREV;

    if (!N_IS_OBJECT (maybe_n_object))
        {
            g_warning ("n_manager_n_object_next: n_list data is not NObject");
            return -1;
        }
    n_object = N_OBJECT (maybe_n_object);
    return 0;
}
