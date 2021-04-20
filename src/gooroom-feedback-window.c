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
};

G_DEFINE_TYPE_WITH_PRIVATE (GooroomFeedbackAppWindow, gooroom_feedback_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
gfb_submit_button_clicked (GtkButton *widget,
                           gpointer   user_data)
{
  GooroomFeedbackAppWindowPrivate *priv = NULL;
  gchar *title = NULL;
  gchar *description = NULL;
  gchar *category = NULL;
  GtkTextIter start_iter;
  GtkTextIter end_iter;

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
  g_free (description);
  // TODO: Submit
}

static void
gooroom_feedback_app_window_init (GooroomFeedbackAppWindow *win)
{
  GooroomFeedbackAppWindowPrivate *priv = NULL;

  priv = gooroom_feedback_app_window_get_instance_private (win);
  gtk_widget_init_template (GTK_WIDGET (win));

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
