#include "gooroom-feedback-window.h"

struct _GooroomFeedbackAppWindow {
  GtkApplicationWindow parent;
};

typedef struct _GooroomFeedbackAppWindowPrivate GooroomFeedbackAppWindowPrivate;

struct _GooroomFeedbackAppWindowPrivate
{
  GtkWidget *gfb_title_entry;
  GtkWidget *gfb_category_button_problem;
  GtkWidget *gfb_category_button_suggestion;
  GtkWidget *gfb_description_buffer;
  GtkWidget *gfb_button_submit;
  GtkWidget *gfb_button_cancel;
  GtkWidget *gfb_history_window;
  gchar *server_url;
  gchar *gfb_history;
};

G_DEFINE_TYPE_WITH_PRIVATE (GooroomFeedbackAppWindow, gooroom_feedback_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
gfb_submit_button_clicked (GtkButton *widget,
                           gpointer   user_data)
{
  GooroomFeedbackAppWindowPrivate *priv = NULL;
  gboolean response = TRUE;
  const gchar *title = NULL;
  gchar *description = NULL;
  gchar *category = NULL;
  gchar *release = NULL;
  gchar *code_name = NULL;
  gchar *response_msg = NULL;
  gchar *server_response = NULL;
  GtkTextIter start_iter;
  GtkTextIter end_iter;
  GtkWidget *dialog = NULL;
  FILE *history = NULL;
  time_t temp;
  struct tm *time_ptr = NULL;
  char time_str[100] = { 0, };

  priv = gooroom_feedback_app_window_get_instance_private (GOOROOM_FEEDBACK_APP_WINDOW (user_data));

  title = gtk_entry_get_text (GTK_ENTRY (priv->gfb_title_entry));

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->gfb_category_button_problem)))
    category = "problem";
  else
    category = "suggestion";

  gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (priv->gfb_description_buffer),
                                  &start_iter);
  gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (priv->gfb_description_buffer),
                                &end_iter);
  description = gtk_text_buffer_get_text (GTK_TEXT_BUFFER (priv->gfb_description_buffer),
                                          &start_iter,
                                          &end_iter,
                                          FALSE);

  gfb_get_os_info (&release, &code_name);

  g_print ("title: %s\ncategory: %s\ndescription: %s\nrelease: %s\ncode name: %s\n",
           title, category, description, release, code_name);

  // TODO: Submit

  if (strlen (title))
  {
    temp = time (NULL);
    time_ptr = localtime (&temp);
    strftime (time_str, sizeof (time_str), "%F %T", time_ptr);
    history = fopen (priv->gfb_history, "a");
    response = gfb_post_request (priv->server_url, title, category, release, code_name, description);
    if (response)
    {
      server_response = "SUCCESS";
      response_msg = "\nThanks for taking the time to give us feedback.\n";
    }
    else
    {
      server_response = "FAILURE";
      response_msg = "\nFAILURE.\n";
    }
    fprintf (history, "%s::%s::%s\n",
            time_str, title, server_response);
    fclose (history);
    g_free (description);
    if (release)
        free (release);
    if (code_name)
        free (code_name);
    dialog = gtk_message_dialog_new (GTK_WINDOW (user_data),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_INFO,
                                     GTK_BUTTONS_CLOSE,
                                     response_msg,
                                     NULL);
    gtk_window_set_title (GTK_WINDOW (dialog),
                          "Gooroom Feedback");
    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy),
                              GOOROOM_FEEDBACK_APP_WINDOW (user_data));
  }
  else
  {
    response_msg = "\nPlease provide us more detailed information of your feedback.\n";
    dialog = gtk_message_dialog_new (GTK_WINDOW (user_data),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_INFO,
                                     GTK_BUTTONS_CLOSE,
                                     response_msg,
                                     NULL);
    gtk_window_set_title (GTK_WINDOW (dialog),
                          "Gooroom Feedback");
    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy),
                              dialog);
  }

  gtk_dialog_run (GTK_DIALOG (dialog));
}

static void
gooroom_feedback_app_window_init (GooroomFeedbackAppWindow *win)
{
  GooroomFeedbackAppWindowPrivate *priv = NULL;
  GtkEntryBuffer *buffer = NULL;
  GKeyFile *key_file = g_key_file_new ();

  priv = gooroom_feedback_app_window_get_instance_private (win);
  gtk_widget_init_template (GTK_WIDGET (win));

  if (g_key_file_load_from_file (key_file, GFB_CONF, G_KEY_FILE_NONE, NULL))
  {
    priv->server_url = g_key_file_get_string (key_file, "SERVER", "address", NULL);
    priv->gfb_history = g_key_file_get_string (key_file, "SERVER", "history", NULL);
    g_key_file_free (key_file);
  }

  buffer = gtk_entry_get_buffer (GTK_ENTRY (priv->gfb_title_entry));
  gtk_entry_buffer_set_max_length (buffer,
                                   GFB_TITLE_LEN);

  gooroom_feedback_history_view_init (priv->gfb_history_window, priv->gfb_history);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->gfb_category_button_problem),
                                TRUE);
  g_signal_connect (priv->gfb_button_submit,
                    "clicked",
                    G_CALLBACK (gfb_submit_button_clicked),
                    win);
  g_signal_connect_swapped (priv->gfb_button_cancel,
                            "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            win);
}

static void
gooroom_feedback_app_window_dispose (GObject *object)
{
  G_OBJECT_CLASS (gooroom_feedback_app_window_parent_class)->dispose (object);
}

static void
gooroom_feedback_app_window_class_init (GooroomFeedbackAppWindowClass *class)
{
  G_OBJECT_CLASS (class)->dispose = gooroom_feedback_app_window_dispose;

  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               GOOROOM_FEEDBACK_WINDOW_UI);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_title_entry);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_category_button_problem);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_category_button_suggestion);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_description_buffer);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_button_submit);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_history_window);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_button_cancel);
}

GooroomFeedbackAppWindow *
gooroom_feedback_app_window_new (GooroomFeedbackApp *app)
{
  return g_object_new (GOOROOM_FEEDBACK_APP_WINDOW_TYPE,
                       "application",
                       app,
                       NULL);
}
