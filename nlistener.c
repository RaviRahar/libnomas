#include "nlistener.h"
#include "nobject.h"

#include <gio/gio.h>
#include <glib.h>

struct _NListener
{
    GObject parent_instance;
    GDBusConnection *connection;
    void (*callback) (NObject *, gpointer);
    gpointer callback_args;
    gboolean listener_running;
    guint notif_filter;
    guint drop_outgoing_filter;
};

G_DEFINE_TYPE (NListener, n_listener, G_TYPE_OBJECT)

static void
n_listener_init (NListener *self)
{
    self->connection = NULL;
    self->callback = NULL;
    self->callback_args = NULL;
    self->listener_running = FALSE;
    self->notif_filter = 0;
    self->drop_outgoing_filter = 0;
}

static void
n_listener_class_finalize (GObject *object)
{
    NListener *self = N_LISTENER (object);
    if (self->connection)
        {
            if (self->listener_running)
                {
                    n_listener_stop (self);
                }
            g_object_unref (self->connection);
        }
    G_OBJECT_CLASS (n_listener_parent_class)->finalize (object);
}

static void
n_listener_class_init (NListenerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = n_listener_class_finalize;
}

NListener *
n_listener_new (void)
{
    return g_object_new (N_TYPE_LISTENER, NULL);
}

gint
n_listener_set_callback (
    NListener *self, void (*callback) (NObject *, gpointer),
    gpointer callback_args
)
{
    if (!N_IS_LISTENER (self))
        {
            g_warning ("n_listener_set_callback: argument is not a NListener");
            return -1;
        }
    self->callback = callback;
    if (!self->callback)
        {
            g_warning (
                "n_listener_on_notification_received: setting callback failed"
            );
            return -1;
        }
    self->callback_args = callback_args;
    return 0;
}

static GDBusMessage *
n_listener_dbus_notif_filter (
    GDBusConnection *connection, GDBusMessage *received_message,
    gboolean incoming, gpointer n_listener_object
)
{
    NListener *self = N_LISTENER (n_listener_object);
    if (!self->callback)
        {
            g_warning (
                "n_listener_on_notification_received: setting callback failed"
            );
            /*return;*/
        }
    const gchar *object_path = g_dbus_message_get_path (received_message);
    gboolean obj_path_notif
        = !g_strcmp0 (object_path, "/org/freedesktop/Notifications");
    GDBusMessageType received_message_type
        = g_dbus_message_get_message_type (received_message);
    gboolean message_method_call
        = (received_message_type == G_DBUS_MESSAGE_TYPE_METHOD_CALL);
    if (obj_path_notif && message_method_call)
        {

            GVariant *received_message_body
                = g_dbus_message_get_body (received_message);
            gchar *received_message_body_type
                = g_strdup (g_variant_get_type_string (received_message_body));
            gboolean received_known_notif_type
                = !g_strcmp0 (received_message_body_type, "(susssasa{sv}i)");
            if (received_known_notif_type)
                {
                    gchar *body = NULL, *message = NULL, *summary = NULL,
                          *appname = NULL, *category = NULL,
                          *default_action_name = NULL, *icon_path = NULL;
                    gint id = 0, progress = 0;
                    gint64 timestamp = 0, timeout = 0;
                    /* "(susssasa{sv}i)" */

                    // Help taken from dunst
                    GVariant *hints;
                    gchar **actions;

                    GVariantIter i;
                    g_variant_iter_init (&i, received_message_body);

                    g_variant_iter_next (&i, "s", appname);
                    g_variant_iter_next (&i, "u", id);
                    g_variant_iter_next (&i, "s", icon_path);
                    g_variant_iter_next (&i, "s", summary);
                    g_variant_iter_next (&i, "s", body);
                    g_variant_iter_next (&i, "^a&s", &actions);
                    g_variant_iter_next (&i, "@a{?*}", &hints);
                    g_variant_iter_next (&i, "i", &timeout);

                    GVariant *dict_value;
                    if ((dict_value = g_variant_lookup_value (
                             hints, "category", G_VARIANT_TYPE_STRING
                         )))
                        {
                            category = g_variant_dup_string (dict_value, NULL);
                            g_variant_unref (dict_value);
                        }

                    NObject *n_object = n_object_new (
                        "bmsacdixtop", body, message, summary, appname,
                        category, default_action_name, icon_path, id,
                        timestamp, timeout, progress
                    );
                    self->callback (n_object, self->callback_args);
                    g_object_unref (n_object);
                }
        }
    return received_message;
}

static GDBusMessage *
n_listener_dbus_filter_drop_outgoing (
    GDBusConnection *connection, GDBusMessage *message, gboolean incoming,
    gpointer user_data
)
{
    if (!incoming)
        {
            g_object_unref (message);
            message = NULL;
        }
    return message;
}

gint
n_listener_dbus_filter_notif (NListener *self)
{
    // Drop all outgoing messages, not allowed for a monitor object
    self->notif_filter = g_dbus_connection_add_filter (
        self->connection, n_listener_dbus_notif_filter, self, NULL
    );
    return 0;
}

static gint
n_listener_dbus_become_monitor (NListener *self)
{
    GError *error = NULL;

    GVariantBuilder *match_rule
        = g_variant_builder_new (G_VARIANT_TYPE ("as"));
    g_variant_builder_add (match_rule, "s", "type='signal'");
    g_variant_builder_add (
        match_rule, "s",
        "type='method_call',interface='org.freedesktop.Notifications',member='"
        "Notify',path='/org/freedesktop/Notifications'"

    );

    g_dbus_connection_call_sync (
        self->connection, "org.freedesktop.DBus", "/org/freedesktop/DBus",
        "org.freedesktop.DBus.Monitoring", "BecomeMonitor",
        g_variant_new ("(asu)", match_rule, 0), NULL, G_DBUS_CALL_FLAGS_NONE,
        -1, NULL, &error
    );
    g_variant_builder_unref (match_rule);

    if (error)
        {
            g_warning (
                "n_listener_dbus_become_monitor: failed to become DBus "
                "Monitor: %s",
                error->message
            );
            g_error_free (error);
            return -1;
        }

    // Drop all outgoing messages, not allowed for a monitor object
    g_dbus_connection_add_filter (
        self->connection, n_listener_dbus_filter_drop_outgoing, NULL, NULL
    );

    return 0;
}

static gint
n_listener_dbus_init_connection (NListener *self)
{
    GError *error = NULL;

    self->connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error);

    if (error)
        {
            g_warning (
                "n_listener_dbus_init_connection: failed to connect to DBus: "
                "%s",
                error->message
            );
            g_error_free (error);
            return -1;
        }
    return 0;
}

gint
n_listener_start (NListener *self)
{
    if (!N_IS_LISTENER (self))
        {
            g_warning ("n_listener_start: argument is not NListener");
            return -1;
        }

    if (!self->callback)
        {
            g_warning ("n_listener_start: callback not set");
        }
    if (n_listener_dbus_init_connection (self) < 0)
        {
            g_warning (
                "n_listener_start: dbus connection initialization failed"
            );
        }
    if (n_listener_dbus_become_monitor (self) < 0)
        {
            g_warning ("n_listener_start: dbus BecomeMonitor failed");
        }
    if (n_listener_dbus_filter_notif (self) < 0)
        {
            g_warning (
                "n_listener_start: could not add filter for notifcations"
            );
        }
    self->listener_running = TRUE;
    return 0;
}

gint
n_listener_stop (NListener *self)
{
    if (!N_IS_LISTENER (self))
        {
            g_warning ("n_listener_stop: argument is not NListener");
            return -1;
        }
    if (self->connection)
        {
            if (self->notif_filter > 0)
                {
                    g_dbus_connection_remove_filter (
                        self->connection, self->notif_filter
                    );
                    self->notif_filter = 0;
                }
            if (self->drop_outgoing_filter > 0)
                {
                    g_dbus_connection_remove_filter (
                        self->connection, self->drop_outgoing_filter
                    );
                    self->drop_outgoing_filter = 0;
                }
            g_object_unref (self->connection);
            self->connection = NULL;
        }
    self->listener_running = FALSE;
    return 0;
}
