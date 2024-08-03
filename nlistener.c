#include "nlistener.h"
#include "glib.h"
#include "nobject.h"
#include <time.h>

struct _NListener
{
    GObject parent_instance;
    GDBusConnection *connection;
    void (*callback) (NObject *, gpointer);
    gpointer callback_args;
    guint subscription_id;
};

G_DEFINE_TYPE (NListener, n_listener, G_TYPE_OBJECT)

static void
n_listener_init (NListener *self)
{
    self->connection = NULL;
    self->callback = NULL;
    self->callback_args = NULL;
    self->subscription_id = 0;
}

static void
n_listener_class_finalize (GObject *object)
{
    NListener *self = N_LISTENER (object);
    if (self->connection)
        {
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

typedef enum
{
    N_SIGNAL_RETURN_STRING = 1,
    N_SIGNAL_RETURN_STRING_ARRAY,
    N_SIGNAL_RETURN_NOTIFICATION,
    N_SIGNAL_RETURN_ANY
} N_LISTENER_SIGNAL_RETURN_TYPE;

static gint
n_listener_find_signal_return_type (GVariant *signal_return_val)
{
    gchar *return_val_type = g_variant_get_type_string (signal_return_val);

    if (!g_strcmp0 (return_val_type, "(s)"))
        {
            return N_SIGNAL_RETURN_STRING;
        }
    else if (!g_strcmp0 (return_val_type, "(as)"))
        {
            return N_SIGNAL_RETURN_STRING_ARRAY;
        }
    else if (!g_strcmp0 (return_val_type, "(susssasa{sv}i)"))
        {
            return N_SIGNAL_RETURN_NOTIFICATION;
        }
    else
        {
            return N_SIGNAL_RETURN_ANY;
        }
}

static void
n_listener_on_notification_received (
    GDBusConnection *connection, const gchar *sender_name,
    const gchar *object_path, const gchar *interface_name,
    const gchar *signal_name, GVariant *parameters, gpointer n_listener_object
)
{
    // FIX: implement code to parse notifications
    NListener *self = N_LISTENER (n_listener_object);
    if (!self->callback)
        {
            g_warning (
                "n_listener_on_notification_received: setting callback failed"
            );
            return;
        }
    /* const gchar *body, *message, *summary, *appname, *category, */
    /*     *default_action_name, *icon_path; */
    /* gint id, progress; */
    /* gint64 timestamp, timeout; */
    /*(&s&s&s&s&s&s&s&ixxi)*/

    /* GVariant *variant_actions = g_variant_new ("as", NULL, 0); */
    /* GVariant *variant_hints = g_variant_new ("a{sv}", NULL, 0); */
    gchar *return_val_type = g_strdup (g_variant_get_type_string (parameters));
    /* switch (n_listener_find_signal_return_type (parameters)) */
    /*     { */
    /*     case N_SIGNAL_RETURN_STRING: */
    /*         break; */
    /*     case N_SIGNAL_RETURN_STRING_ARRAY: */
    /*         break; */
    /*     case N_SIGNAL_RETURN_NOTIFICATION: */
    /*         break; */
    /*     case N_SIGNAL_RETURN_ANY: */
    /*         break; */
    /*     } */

    g_warning ("");
    g_warning ("sender_name: %s", sender_name);
    g_warning ("object_path: %s", object_path);
    g_warning ("interface_name: %s", interface_name);
    g_warning ("signal_name: %s", signal_name);
    g_warning ("signal return type any: %s", return_val_type);
    g_warning ("GVariant: %s", g_variant_print (parameters, TRUE));

    /* NObject *n_object = n_object_new ( */
    /*     "bmsacdixtop", body, message, summary, appname, category, */
    /*     default_action_name, icon_path, id, timestamp, timeout, progress */
    /* ); */
    NObject *n_object = n_object_new ("");
    self->callback (n_object, self->callback_args);
    g_object_unref (n_object);
}

static gint
n_listener_dbus_subscribe_to_notifications (NListener *self)
{
    self->subscription_id = g_dbus_connection_signal_subscribe (
        self->connection, NULL, NULL, NULL, NULL, NULL,
        G_DBUS_SIGNAL_FLAGS_NO_MATCH_RULE, n_listener_on_notification_received,
        (gpointer)self, NULL
    );

    if (!self->subscription_id)
        {

            g_warning ("n_listener_dbus_subscribe_to_notifications: failed to "
                       "subscribe");
            return -1;
        }
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
        "type='signal',interface='org.freedesktop.Notifications',member='"
        "Notify',path='/org/freedesktop/Notifications'"
    );
    // FIX: listen to method_calls and not signal
    /* g_variant_builder_add ( */
    /*     match_rule, "s", */
    /*     "type='method_call',interface='org.freedesktop.Notifications',member='Notify',path='/org/freedesktop/Notifications',eavesdrop='true'"
     */
    /* ); */
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
    return n_listener_dbus_subscribe_to_notifications (self);
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
            if (self->subscription_id > 0)
                {
                    g_dbus_connection_signal_unsubscribe (
                        self->connection, self->subscription_id
                    );
                    self->subscription_id = 0;
                }
            g_object_unref (self->connection);
            self->connection = NULL;
        }
    return 0;
}
