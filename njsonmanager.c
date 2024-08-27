#include "njsonmanager.h"
#include "nobject.h"

#include <glib-object.h>
#include <glib.h>
#include <json-glib/json-glib.h>

struct _NJsonManager
{
    GObject parent_instance;
    GList *n_list;
    gchar *filename;
    N_JSON_MANAGER_PARSER_TYPE j_parser_type;
};

G_DEFINE_TYPE (NJsonManager, n_json_manager, G_TYPE_OBJECT)

static void
n_json_manager_init (NJsonManager *self)
{
    self->n_list = NULL;
    self->filename = NULL;
    self->j_parser_type = GENERIC;
}

static void
n_json_manager_class_finalize (GObject *object)
{
    NJsonManager *self = N_JSON_MANAGER (object);
    if (self->n_list)
        {
            g_list_free_full (g_steal_pointer (&self->n_list), g_object_unref);
        }
    G_OBJECT_CLASS (n_json_manager_parent_class)->finalize (object);
}

static void
n_json_manager_class_init (NJsonManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = n_json_manager_class_finalize;
}

NJsonManager *
n_json_manager_new (void)
{
    return g_object_new (N_TYPE_JSON_MANAGER, NULL);
};

gint
n_json_manager_set_n_list (NJsonManager *self, GList *n_list)
{
    if (!N_IS_JSON_MANAGER (self))
        {
            g_warning (
                "n_json_manager_set_n_list: argument is not NJsonManager"
            );
            return -1;
        }
    self->n_list = n_list;
    if (!self->n_list)
        {
            g_warning ("n_json_manager_set_n_list: setting n_list failed");
            return -1;
        }
    return 0;
}

gint
n_json_manager_set_file (
    NJsonManager *self, const gchar *filename,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type
)
{
    if (!N_IS_JSON_MANAGER (self))
        {
            g_warning ("n_json_manager_set_file: argument is not NJsonManager"
            );
            return -1;
        }
    self->filename = (gchar *)filename;
    if (!self->filename)
        {
            g_warning ("n_json_manager_set_file: setting filename failed");
            return -1;
        }
    self->j_parser_type = j_parser_type;
    return 0;
};

static void
n_json_manager_n_object_to_json_node (NObject *n_object, JsonNode *j_node)
{
    JsonBuilder *builder = json_builder_new ();
    json_builder_begin_object (builder);
    json_builder_set_member_name (builder, "body");
    json_builder_add_string_value (builder, n_object_body (n_object));
    json_builder_set_member_name (builder, "message");
    json_builder_add_string_value (builder, n_object_message (n_object));
    json_builder_set_member_name (builder, "summary");
    json_builder_add_string_value (builder, n_object_summary (n_object));
    json_builder_set_member_name (builder, "appname");
    json_builder_add_string_value (builder, n_object_appname (n_object));
    json_builder_set_member_name (builder, "category");
    json_builder_add_string_value (builder, n_object_category (n_object));
    json_builder_set_member_name (builder, "default_action_name");
    json_builder_add_string_value (
        builder, n_object_default_action_name (n_object)
    );
    json_builder_set_member_name (builder, "icon_path");
    json_builder_add_string_value (builder, n_object_icon_path (n_object));
    json_builder_set_member_name (builder, "id");
    json_builder_add_int_value (builder, n_object_id (n_object));
    json_builder_set_member_name (builder, "timestamp");
    json_builder_add_int_value (builder, n_object_timestamp (n_object));
    json_builder_set_member_name (builder, "timeout");
    json_builder_add_int_value (builder, n_object_timeout (n_object));
    json_builder_set_member_name (builder, "progress");
    json_builder_add_int_value (builder, n_object_progress (n_object));
    json_builder_end_object (builder);
    j_node = json_builder_get_root (builder);
    g_object_unref (builder);
}

static const gchar *
json_object_get_string_member_or_default (
    JsonObject *object, const gchar *member_name, const gchar *default_value
)
{
    if (json_object_has_member (object, member_name))
        {
            return json_object_get_string_member (object, member_name);
        }
    return default_value;
}

static gint
json_object_get_int_member_or_default (
    JsonObject *object, const gchar *member_name, gint default_value
)
{
    if (json_object_has_member (object, member_name))
        {
            return json_object_get_int_member (object, member_name);
        }
    return default_value;
}

static void
n_json_manager_json_node_to_n_object (JsonNode *j_node, NObject *n_object)
{
    if (!j_node || json_node_get_node_type (j_node) != JSON_NODE_OBJECT)
        {
            g_warning (
                "n_json_manager_json_node_to_n_object: invalid JSON node"
            );
        }

    JsonObject *j_object = json_node_get_object (j_node);

    n_object = n_object_new (
        json_object_get_string_member_or_default (j_object, "body", ""),
        json_object_get_string_member_or_default (j_object, "message", ""),
        json_object_get_string_member_or_default (j_object, "summary", ""),
        json_object_get_string_member_or_default (j_object, "appname", ""),
        json_object_get_string_member_or_default (j_object, "category", ""),
        json_object_get_string_member_or_default (
            j_object, "default_action_name", ""
        ),
        json_object_get_string_member_or_default (j_object, "icon_path", ""),
        json_object_get_int_member_or_default (j_object, "id", -1),
        json_object_get_int_member_or_default (j_object, "timestamp", -1),
        json_object_get_int_member_or_default (j_object, "timeout", -1),
        json_object_get_int_member_or_default (j_object, "progress", -1)
    );
}

static void
n_json_manager_parser_array_from_root (
    JsonNode *root, JsonArray *array, N_JSON_MANAGER_PARSER_TYPE j_parser_type
)
{
    if (json_node_get_node_type (root) == JSON_NODE_OBJECT)
        {
            JsonObject *obj = json_node_get_object (root);
            JsonNode *j_node = json_object_get_member (obj, "data");
            if (j_node && json_node_get_node_type (j_node) == JSON_NODE_ARRAY)
                {
                    JsonArray *extra_array;
                    switch (j_parser_type)
                        {
                        case DUNST:
                            extra_array = json_node_get_array (j_node);
                            if (json_array_get_length (extra_array) == 1)
                                {
                                    JsonNode *j_node_inner
                                        = json_array_get_element (
                                            extra_array, 0
                                        );
                                    if (json_node_get_node_type (j_node_inner)
                                        == JSON_NODE_ARRAY)
                                        {
                                            array = json_node_get_array (
                                                j_node_inner
                                            );
                                        }
                                    g_object_unref (j_node_inner);
                                }
                            g_object_unref (extra_array);
                            break;
                        case GENERIC:
                            array = json_node_get_array (j_node);
                            break;
                        default:
                            g_warning ("n_json_manager_parser_get_array_from_"
                                       "root: parser type not implemented");
                            break;
                        }
                }
            g_object_unref (j_node);
            g_object_unref (obj);
        }
}

static void
n_json_manager_parser_array_from_file (
    const gchar *filename, JsonArray *array,
    N_JSON_MANAGER_PARSER_TYPE j_parser_type
)
{
    gchar *content = NULL;
    if (g_file_get_contents (filename, &content, NULL, NULL))
        {
            JsonParser *parser = json_parser_new ();
            if (json_parser_load_from_data (parser, content, -1, NULL))
                {
                    JsonNode *root = json_parser_get_root (parser);
                    n_json_manager_parser_array_from_root (
                        root, array, j_parser_type
                    );
                    g_object_unref (root);
                }
            g_object_unref (parser);
        }
    else
        {
            g_warning ("n_json_manager_parser_array_from_file: failed to "
                       "read existing content from file");
        }
}

gint
n_json_manager_load_from_file (NJsonManager *self, gboolean append)
{
    if (!N_IS_JSON_MANAGER (self))
        {
            g_warning (
                "n_json_manager_load_from_file: argument is not NJsonManager"
            );
            return -1;
        }

    if (!self->filename)
        {
            g_warning ("n_json_manager_load_from_file: filename not set");
            return -1;
        }

    JsonArray *array = NULL;
    n_json_manager_parser_array_from_file (
        self->filename, array, self->j_parser_type
    );

    if (!array
        || json_node_get_node_type (json_node_new (JSON_NODE_ARRAY))
               != JSON_NODE_ARRAY)
        {
            g_warning (
                "n_json_manager_load_from_file: missing or invalid 'data' "
                "array in JSON file: %s",
                self->filename
            );
            return -1;
        }

    if (!append)
        {
            g_list_free_full (self->n_list, g_object_unref);
        }

    for (guint i = 0; i < json_array_get_length (array); i++)
        {
            JsonNode *j_node = json_array_get_element (array, i);
            NObject *n_object = NULL;
            n_json_manager_json_node_to_n_object (j_node, n_object);
            if (n_object)
                {
                    self->n_list = g_list_insert_sorted (
                        self->n_list, n_object, n_object_compare_by_timestamp
                    );
                }
        }
    return 0;
}

// FIX: not working
gint
n_json_manager_save_to_file (NJsonManager *self, gboolean append)
{
    if (!N_IS_JSON_MANAGER (self))
        {
            g_warning (
                "n_json_manager_save_to_file: argument is not NJsonManager"
            );
            return -1;
        }
    if (!self->filename)
        {
            g_warning ("n_json_manager_load_from_file: filename not set");
            return -1;
        }
    if (!self->n_list)
        {
            g_warning ("n_json_manager_load_from_file: n_list not set");
            return -1;
        }
    JsonBuilder *builder = json_builder_new ();
    JsonGenerator *gen = json_generator_new ();
    JsonArray *array = json_array_new ();

    if (append)
        {
            n_json_manager_parser_array_from_file (
                self->filename, array, self->j_parser_type
            );
        }

    for (GList *l = self->n_list; l != NULL; l = l->next)
        {
            NObject *n_object = (NObject *)l->data;
            JsonNode *j_node = NULL;
            n_json_manager_n_object_to_json_node (n_object, j_node);
            json_array_add_element (array, j_node);
            json_node_unref (j_node);
        }

    JsonObject *j_object = json_object_new ();
    json_object_set_string_member (j_object, "type", "aa{sv}");
    json_object_set_array_member (j_object, "data", array);

    JsonNode *root = json_node_new (JSON_NODE_OBJECT);
    json_node_set_object (root, j_object);
    json_generator_set_root (gen, root);

    if (!json_generator_to_file (gen, self->filename, NULL))
        {
            g_warning ("n_json_manager_save_to_file: failed to write JSON "
                       "data to file");
            return -1;
        }

    g_object_unref (gen);
    g_object_unref (builder);
    json_node_unref (root);
    return 0;
}
