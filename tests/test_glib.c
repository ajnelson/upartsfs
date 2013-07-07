#include <glib.h>

/* Example source: https://developer.gnome.org/glib/stable/glib-Doubly-Linked-Lists.html#GList */

int main(int main_argc, char *main_argv[])
{
    /* Notice that these are initialized to the empty list. */
    GList *list = NULL, *number_list = NULL;

    /* This is a list of strings. */
    list = g_list_append (list, "first");
    list = g_list_append (list, "second");

    /* This is a list of integers. */
    number_list = g_list_append (number_list, GINT_TO_POINTER (27));
    number_list = g_list_append (number_list, GINT_TO_POINTER (14));
    return 0;
}
