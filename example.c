#include "njsonmanager.h"
#include "nmanager.h"
#include <stdio.h>
#include <unistd.h>

gboolean
timeout_callback (gpointer user_data)
{
    GMainLoop *loop = (GMainLoop *)user_data;
    g_main_loop_quit (loop);
    return G_SOURCE_REMOVE; // Remove the source after it is triggered
}

gint
main (gint argc, gchar *argv[])
{
    GMainLoop *loop;

    // Create a new GMainLoop object
    loop = g_main_loop_new (NULL, FALSE);

    // Check if loop creation was successful
    if (loop == NULL)
        {
            g_printerr ("Failed to create main loop\n");
            return 1;
        }
    const gchar *history_file = "notification_history";
     NManager *manager = n_manager_new (); 
     n_manager_set_history_file (manager, history_file, DUNST); 

    if (n_manager_start_listener (manager) < 0)
        {
            printf ("failed to start listener\n");
        }

    g_timeout_add_seconds (20, timeout_callback, NULL);
    // Run the main loop
    g_main_loop_run (loop);

    // Clean up
    /* g_main_loop_unref (loop); */

    /* n_manager_save_state (manager); */
    /* n_manager_stop_listener (manager); */

    /* n_manager_load_state (manager); */
    /* n_manager_save_state (manager); */

    /* const char *genericfile = "generic_history"; */
    /* n_manager_save_to_file (manager, genericfile, GENERIC, TRUE); */
    /* const char *dunstfile = "dunst_history"; */
    /* n_manager_save_to_file (manager, dunstfile, DUNST, TRUE); */

    /* const char *dunstfile = "dunst_history"; */
    /* n_manager_load_from_file(manager, dunstfile, DUNST, TRUE); */
    /* const char *genericfile = "generic_history"; */
    /* n_manager_save_to_file (manager, genericfile, GENERIC, TRUE); */

    /* g_object_unref (manager); */
    return EXIT_SUCCESS;
}
