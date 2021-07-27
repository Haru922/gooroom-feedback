#include "gooroom-feedback-window.h"

struct _GooroomFeedbackAppWindow {
  GtkApplicationWindow parent;
};

typedef struct _GooroomFeedbackAppWindowPrivate GooroomFeedbackAppWindowPrivate;

struct _GooroomFeedbackAppWindowPrivate
{
  GtkWidget *gfb_button_new_dialog;
  GtkWidget *gfb_history_scrolled_window;
  GtkWidget *gfb_history_label;
  gchar *gfb_history;
};

G_DEFINE_TYPE_WITH_PRIVATE (GooroomFeedbackAppWindow, gooroom_feedback_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
gfb_dialog_new_button_clicked (GtkButton *widget,
                               gpointer   user_data)
{
  GooroomFeedbackDialog *dialog = gooroom_feedback_dialog_new ();
  //gtk_window_set_modal (GTK_WINDOW (dialog), FALSE);
  gtk_dialog_run (GTK_DIALOG (dialog));
}

static void
gooroom_feedback_app_window_init (GooroomFeedbackAppWindow *win)
{
  GooroomFeedbackAppWindowPrivate *priv = NULL;
  GKeyFile *key_file = g_key_file_new ();
  GtkCssProvider *css_provider = gtk_css_provider_new ();
  GtkWidget *gfb_button_new_dialog_label = gtk_label_new ("+ New");

  priv = gooroom_feedback_app_window_get_instance_private (win);
  gtk_widget_init_template (GTK_WIDGET (win));

  if (g_key_file_load_from_file (key_file, GFB_CONF, G_KEY_FILE_NONE, NULL))
  {
    priv->gfb_history = g_key_file_get_string (key_file, "SERVER", "history", NULL);
    g_key_file_free (key_file);
  }

  gtk_css_provider_load_from_resource (css_provider, GOOROOM_FEEDBACK_CSS);
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default(),
                                             GTK_STYLE_PROVIDER (css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  gtk_widget_set_name (gfb_button_new_dialog_label, "gfb-button-new-dialog-label");
  gtk_container_add (GTK_CONTAINER (priv->gfb_button_new_dialog), gfb_button_new_dialog_label);
  gtk_widget_show (gfb_button_new_dialog_label);
  gooroom_feedback_history_view_init (priv->gfb_history_scrolled_window, priv->gfb_history);

  g_signal_connect (priv->gfb_button_new_dialog,
                    "clicked",
                    G_CALLBACK (gfb_dialog_new_button_clicked),
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
                                                gfb_history_label);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_history_scrolled_window);
  gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class),
                                                GooroomFeedbackAppWindow,
                                                gfb_button_new_dialog);
}

GooroomFeedbackAppWindow *
gooroom_feedback_app_window_new (GooroomFeedbackApp *app)
{
  return g_object_new (GOOROOM_FEEDBACK_APP_WINDOW_TYPE,
                       "application",
                       app,
                       NULL);
}
