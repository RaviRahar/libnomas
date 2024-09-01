#include "nobject.h"

#include <glib-object.h>
#include <glib.h>

struct _NObject
{
    GObject parent_instance;
    gchar *body;
    gchar *message;
    gchar *summary;
    gchar *appname;
    gchar *category;
    gchar *default_action_name;
    gchar *icon_path;
    guint id;
    gint64 timestamp;
    gint timeout;
    gint progress;
};

G_DEFINE_TYPE (NObject, n_object, G_TYPE_OBJECT)

static void
n_object_init (NObject *self)
{
    self->body = g_strdup ("");
    self->message = g_strdup ("");
    self->summary = g_strdup ("");
    self->appname = g_strdup ("");
    self->category = g_strdup ("");
    self->default_action_name = g_strdup ("");
    self->icon_path = g_strdup ("");
    self->id = 0;
    self->timestamp = -1;
    self->timeout = -1;
    self->progress = -1;
}

static void
n_object_class_finalize (GObject *object)
{
    NObject *self = N_OBJECT (object);
    g_free (self->body);
    g_free (self->message);
    g_free (self->summary);
    g_free (self->appname);
    g_free (self->category);
    g_free (self->default_action_name);
    g_free (self->icon_path);
    G_OBJECT_CLASS (n_object_parent_class)->finalize (object);
}

static void
n_object_class_init (NObjectClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = n_object_class_finalize;
}

NObject *
n_object_new (const gchar *format, ...)
{
    NObject *self = g_object_new (N_TYPE_OBJECT, NULL);
    va_list args;
    va_start (args, format);

    while (*format)
        {

            switch (*format++)
                {
                case 'b':
                    g_free (self->body);
                    self->body = g_strdup (va_arg (args, gchar *));
                    break;
                case 'm':
                    g_free (self->message);
                    self->message = g_strdup (va_arg (args, gchar *));
                    break;
                case 's':
                    g_free (self->summary);
                    self->summary = g_strdup (va_arg (args, gchar *));
                    break;
                case 'a':
                    g_free (self->appname);
                    self->appname = g_strdup (va_arg (args, gchar *));
                    break;
                case 'c':
                    g_free (self->category);
                    self->category = g_strdup (va_arg (args, gchar *));
                    break;
                case 'd':
                    g_free (self->default_action_name);
                    self->default_action_name
                        = g_strdup (va_arg (args, gchar *));
                    break;
                case 'i':
                    g_free (self->icon_path);
                    self->icon_path = g_strdup (va_arg (args, gchar *));
                    break;
                case 'x':
                    self->id = va_arg (args, gint);
                    break;
                case 't':
                    self->timestamp = va_arg (args, gint64);
                    break;
                case 'o':
                    self->timeout = va_arg (args, gint64);
                    break;
                case 'p':
                    self->progress = va_arg (args, gint);
                    break;
                default:
                    g_warning (
                        "n_object_new: unknown format specifier: "
                        "%c\n",
                        *(format - 1)
                    );
                    va_end (args);
                    return NULL;
                }
        }

    va_end (args);
    return self;
}

const gchar *
n_object_body (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_body: arg is not NObject");
            return NULL;
        }
    return self->body;
}
const gchar *
n_object_message (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_message: arg is not NObject");
            return NULL;
        }
    return self->message;
}
const gchar *
n_object_summary (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_summary: arg is not NObject");
            return NULL;
        }
    return self->summary;
}
const gchar *
n_object_appname (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_appname: arg is not NObject");
            return NULL;
        }
    return self->appname;
}
const gchar *
n_object_category (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_category: arg is not NObject");
            return NULL;
        }
    return self->category;
}
const gchar *
n_object_default_action_name (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_default_action_name: arg is not NObject");
            return NULL;
        }
    return self->default_action_name;
}
const gchar *
n_object_icon_path (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_icon_path: arg is not NObject");
            return NULL;
        }
    return self->icon_path;
}
gint
n_object_id (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_id: arg is not NObject");
            return -1;
        }
    return self->id;
}
gint64
n_object_timestamp (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_timestamp: arg is not NObject");
            return -1;
        }
    return self->timestamp;
}
gint64
n_object_timeout (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_timeout: arg is not NObject");
            return -1;
        }
    return self->timeout;
}
gint
n_object_progress (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_progress: arg is not NObject");
            return -1;
        }
    return self->progress;
}

gint
n_object_compare_by_timestamp (gconstpointer a, gconstpointer b)
{
    NObject *notif_a = (NObject *)a;
    NObject *notif_b = (NObject *)b;
    if (!N_IS_OBJECT (notif_a) || !N_IS_OBJECT (notif_b))
        {
            g_warning ("n_object_compare_by_timestamp: arg is not NObject");
            return -1;
        }
    gint64 x = n_object_timestamp (notif_a);
    gint64 y = n_object_timestamp (notif_b);

    if (x < y)
        return -1;
    if (x > y)
        return 1;
    return 0;
}

gchar *
n_object_print (NObject *self)
{
    if (!N_IS_OBJECT (self))
        {
            g_warning ("n_object_print: arg is not NObject");
            return NULL;
        }
    GString *n_object_string = g_string_new (NULL);
    g_string_printf (
        n_object_string,
        "\nNotification:\n"
        "\tappname: %s\n"
        "\tsummary: %s\n"
        "\tbody: %s\n"
        "\tmessage: %s\n"
        "\tcategory: %s\n"
        "\tdefault_action_name: %s\n"
        "\ticon_path: %s\n"
        "\tid: %u\n"
        "\ttimestamp: %ld\n"
        "\ttimeout: %d\n"
        "\tprogress: %d\n"
        "\n",
        self->appname, self->summary, self->body, self->message,
        self->category, self->default_action_name, self->icon_path, self->id,
        self->timestamp, self->timeout, self->progress
    );
    gchar *n_object_pretty = g_string_free_and_steal (n_object_string);
    return n_object_pretty;
}
