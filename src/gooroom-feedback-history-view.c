#include "gooroom-feedback-history-view.h"

void
gooroom_feedback_history_view_init (GtkWidget *gfb_history_window,
                                    char      *gfb_history)
{
  GtkWidget *gfb_history_view;
  GtkWidget *gfb_history_box;

  gfb_history_view = gtk_viewport_new (NULL, NULL);
  gfb_history_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 1);
  gtk_widget_show_all (gfb_history_box);

  gtk_container_add (GTK_CONTAINER (gfb_history_view), gfb_history_box);
  gooroom_feedback_history_view_get_items (GTK_WIDGET (gfb_history_box), gfb_history);
  gtk_container_add (GTK_CONTAINER (gfb_history_window), gfb_history_view);
  gtk_widget_show_all (GTK_WIDGET (gfb_history_view));
}

void
gooroom_feedback_history_view_get_items (GtkWidget *gfb_history_box,
                                         char      *gfb_history)
{
  GtkWidget *gfb_history_button;
  GtkWidget *gfb_history_info;
  GtkWidget *gfb_history_details;
  GtkWidget *gfb_history_label;
  GtkWidget *gfb_history_image;
  GtkWidget *gfb_history_init_box;
  FILE *fp;
  char history[BUFSIZ];
  struct passwd *pw;
  uid_t uid;
  gchar **segments = NULL;

  uid = geteuid ();
  pw = getpwuid (uid);

  if (fp = fopen (gfb_history, "r"))
  {
    while (fgets (history, BUFSIZ, fp))
    {
        segments = g_strsplit (history, "::", 0);
        //segments[GFB_HISTORY_RESULT][strlen(segments[GFB_HISTORY_RESULT])-1] = '\0';
        gfb_history_button = gtk_button_new ();
        gfb_history_info = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
        if (!strcmp (segments[GFB_HISTORY_TYPE], "problem"))
          gfb_history_image = gtk_image_new_from_resource ("/kr/gooroom/gooroom-feedback/gfb-problem.svg");
        else
          gfb_history_image = gtk_image_new_from_resource ("/kr/gooroom/gooroom-feedback/gfb-suggestion.svg");
        gtk_box_pack_start (GTK_BOX (gfb_history_info), gfb_history_image, FALSE, FALSE, 5);
        gfb_history_details = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
        gfb_history_label = gtk_label_new (segments[GFB_HISTORY_TITLE]);
        gtk_label_set_xalign (GTK_LABEL (gfb_history_label), 0);
        gtk_box_pack_start (GTK_BOX (gfb_history_details), gfb_history_label, FALSE, FALSE, 5);
        gfb_history_label = gtk_label_new (segments[GFB_HISTORY_DATE]);
        gtk_label_set_xalign (GTK_LABEL (gfb_history_label), 0);
        gtk_box_pack_start (GTK_BOX (gfb_history_details), gfb_history_label, FALSE, FALSE, 5);
        gtk_box_pack_start (GTK_BOX (gfb_history_info), gfb_history_details, FALSE, FALSE, 5);
        gtk_container_add (GTK_CONTAINER (gfb_history_button), gfb_history_info);
        gtk_box_pack_end (GTK_BOX (gfb_history_box), gfb_history_button, FALSE, FALSE, 0);
        g_strfreev (segments);
        segments = NULL;
    }
    fclose (fp);
  }
  gfb_history_init_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_box_pack_start (GTK_BOX (gfb_history_init_box), gtk_image_new_from_resource ("/kr/gooroom/gooroom-feedback/gfb-init.svg"), FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (gfb_history_init_box), gtk_label_new ("Feedback posted."), FALSE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (gfb_history_box), gfb_history_init_box, FALSE, FALSE, 0);
}
