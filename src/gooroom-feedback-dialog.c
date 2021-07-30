#include "gooroom-feedback-dialog.h"

struct _GooroomFeedbackDialog {
  GtkDialog parent;
};

typedef struct _GooroomFeedbackDialogPrivate
{
  GtkWidget *gfb_title_entry;
  GtkWidget *gfb_category_button_problem;
  GtkWidget *gfb_category_button_suggestion;
  GtkWidget *gfb_description_buffer;
  GtkWidget *gfb_button_submit;
  GtkWidget *gfb_button_cancel;
  gchar *server_url;
  gchar *gfb_history;
} GooroomFeedbackDialogPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GooroomFeedbackDialog, gooroom_feedback_dialog, GTK_TYPE_DIALOG);

static void
gooroom_feedback_dialog_dispose (GObject *object)
{
  G_OBJECT_CLASS (gooroom_feedback_dialog_parent_class)->dispose (object);
}

static void
gfb_submit_button_clicked (GtkButton *widget,
                           gpointer   user_data)
{
  GooroomFeedbackDialogPrivate *priv = NULL;
  long response = GFB_RESPONSE_FAILURE;
  const gchar *title = NULL;
  gchar *description = NULL;
  gchar *category = NULL;
  gchar *release = NULL;
  gchar *code_name = NULL;
  const gchar *response_msg = NULL;
  gchar *server_response = NULL;
  GtkTextIter start_iter;
  GtkTextIter end_iter;
  GtkWidget *dialog = NULL;
  FILE *history = NULL;
  time_t temp;
  struct tm *time_ptr = NULL;
  char time_str[100] = { 0, };

  priv = gooroom_feedback_dialog_get_instance_private (GOOROOM_FEEDBACK_DIALOG (user_data));

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

  if (strlen (title) && strlen (description))
  {
    temp = time (NULL);
    time_ptr = localtime (&temp);
    strftime (time_str, sizeof (time_str), "%F %T", time_ptr);

    response = gfb_post_request (priv->server_url,
                                 title,
                                 category,
                                 release,
                                 code_name,
                                 description);
    if (response == GFB_RESPONSE_SUCCESS)
    {
      server_response = _("SUCCESS");
      response_msg = _("\nThanks for taking the time to give us feedback.\n");
      history = fopen (priv->gfb_history, "a");
      fprintf (history, "%s::%s::%s::%s::%d::%s\n",
               time_str,
               title,
               category,
               description,
               counter,
               server_response);
      fclose (history);
    }
    else
    {
      server_response = _("FAILURE");
      response_msg = _("Internal Server Error.\n");
      // DELETE START
      history = fopen (priv->gfb_history, "a");
      fprintf (history, "%s::%s::%s::%s::%s\n",
               time_str, title, category, description, server_response);
      fclose (history);
      // END
    }
    g_free (description);
    if (release)
      free (release);
    if (code_name)
      free (code_name);
  }
  else
    response_msg = _("\nPlease provide us more detailed information of your feedback.\n");

  dialog = gtk_message_dialog_new (GTK_WINDOW (user_data),
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_INFO,
                                   GTK_BUTTONS_CLOSE,
                                   response_msg,
                                   NULL);
  gtk_widget_set_name (dialog,
                       "gfb-need-info-dialog");
  gtk_window_set_title (GTK_WINDOW (dialog),
                        _("Gooroom Feedback"));

  if (response == GFB_RESPONSE_SUCCESS)
    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy),
                              GOOROOM_FEEDBACK_DIALOG (user_data));
  else
    g_signal_connect_swapped (dialog, "response",
                              G_CALLBACK (gtk_widget_destroy),
                              dialog);
  gtk_dialog_run (GTK_DIALOG (dialog));
}

static void
gooroom_feedback_dialog_init (GooroomFeedbackDialog *self)
{
  GooroomFeedbackDialogPrivate *priv = NULL;
  GtkEntryBuffer *buffer = NULL;
  GKeyFile *key_file = g_key_file_new ();
  GtkCssProvider *css_provider = gtk_css_provider_new ();
  GtkWidget *gfb_button_submit_label = gtk_label_new ("Submit");

  priv = gooroom_feedback_dialog_get_instance_private (self);
  gtk_widget_init_template (GTK_WIDGET (self));

  if (g_key_file_load_from_file (key_file,
                                 GOOROOM_FEEDBACK_CONF,
                                 G_KEY_FILE_NONE,
                                 NULL))
  {
    priv->server_url = g_key_file_get_string (key_file, "SERVER", "address", NULL);
    priv->gfb_history = g_key_file_get_string (key_file, "SERVER", "history", NULL);
    g_key_file_free (key_file);
  }

  gtk_css_provider_load_from_resource (css_provider, GOOROOM_FEEDBACK_CSS);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default(),
                                             GTK_STYLE_PROVIDER (css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  buffer = gtk_entry_get_buffer (GTK_ENTRY (priv->gfb_title_entry));
  gtk_entry_buffer_set_max_length (buffer, GFB_TITLE_LEN);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->gfb_category_button_problem), TRUE);

  gtk_widget_set_name (gfb_button_submit_label,
                       "gfb-button-submit-label");
  gtk_container_add (GTK_CONTAINER (priv->gfb_button_submit),
                     gfb_button_submit_label);
  gtk_widget_show (gfb_button_submit_label);

  g_signal_connect (priv->gfb_button_submit,
                    "clicked",
                    G_CALLBACK (gfb_submit_button_clicked),
                    self);
  g_signal_connect_swapped (priv->gfb_button_cancel, "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            self);
  gtk_widget_show_all (GTK_WIDGET (self));
}

static void
gooroom_feedback_dialog_class_init (GooroomFeedbackDialogClass *class)
{
  G_OBJECT_CLASS (class)->dispose = gooroom_feedback_dialog_dispose;
  gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class),
                                               GOOROOM_FEEDBACK_DIALOG_UI);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackDialog,
                                                gfb_title_entry);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackDialog,
                                                gfb_category_button_problem);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackDialog,
                                                gfb_category_button_suggestion);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackDialog,
                                                gfb_description_buffer);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackDialog,
                                                gfb_button_submit);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackDialog,
                                                gfb_button_cancel);
}

GooroomFeedbackDialog *
gooroom_feedback_dialog_new (void)
{
  return g_object_new (GOOROOM_FEEDBACK_DIALOG_TYPE, NULL);
}
