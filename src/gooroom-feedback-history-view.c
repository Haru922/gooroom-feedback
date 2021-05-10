#include "gooroom-feedback-history-view.h"

void
gooroom_feedback_history_view_init (GtkWidget *gfb_history_window,
                                    char      *gfb_history)
{
  GtkWidget *gfb_history_view;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkListStore *gfb_history_store;
  int column_id;

  gfb_history_view = gtk_tree_view_new ();
  for (column_id = 0; column_id < GFB_HISTORY_COLUMNS; column_id++)
  {
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (column_names[column_id],
                                                       renderer,
                                                       "text",
                                                       column_id,
                                                       NULL);
    if (column_id == GFB_HISTORY_TITLE)
      gtk_tree_view_column_set_fixed_width (column, 200);
    gtk_tree_view_append_column (GTK_TREE_VIEW (gfb_history_view), column);
  }
  gfb_history_store = gtk_list_store_new (GFB_HISTORY_COLUMNS,
                                          G_TYPE_STRING,
                                          G_TYPE_STRING,
                                          G_TYPE_STRING);
  gtk_tree_view_set_model (GTK_TREE_VIEW (gfb_history_view), GTK_TREE_MODEL (gfb_history_store));
  gtk_container_add (GTK_CONTAINER (gfb_history_window), GTK_WIDGET (gfb_history_view));
  g_object_unref (gfb_history_store);

  gooroom_feedback_history_view_get_items (GTK_WIDGET (gfb_history_view), gfb_history);
  gtk_widget_show_all (GTK_WIDGET (gfb_history_view));
}

void
gooroom_feedback_history_view_get_items (GtkWidget *gfb_history_view,
                                         char      *gfb_history)
{
  GtkListStore *gfb_history_store;
  GtkTreeIter iter;
  FILE *fp;
  char history[BUFSIZ];
  struct passwd *pw;
  uid_t uid;
  gchar **segments = NULL;

  uid = geteuid ();
  pw = getpwuid (uid);

  gfb_history_store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (gfb_history_view)));
  if (fp = fopen (gfb_history, "r"))
  {
    while (fgets (history, BUFSIZ, fp))
    {
        segments = g_strsplit (history, "::", 0);
        segments[GFB_HISTORY_RESULT][strlen(segments[GFB_HISTORY_RESULT])-1] = '\0';
        gtk_list_store_append (gfb_history_store, &iter);
        gtk_list_store_set (gfb_history_store,
                            &iter,
                            GFB_HISTORY_DATE,
                            segments[GFB_HISTORY_DATE],
                            GFB_HISTORY_TITLE,
                            segments[GFB_HISTORY_TITLE],
                            GFB_HISTORY_RESULT,
                            segments[GFB_HISTORY_RESULT],
                            -1);
        g_strfreev (segments);
        segments = NULL;
    }
    fclose (fp);
  }
}
