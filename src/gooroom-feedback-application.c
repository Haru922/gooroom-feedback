#include <gtk/gtk.h>

#include "gooroom-feedback-application.h"
#include "gooroom-feedback-window.h"

struct _GooroomFeedbackApp
{
  GtkApplication parent;
};

G_DEFINE_TYPE (GooroomFeedbackApp, gooroom_feedback_app, GTK_TYPE_APPLICATION);

static void
gooroom_feedback_app_init (GooroomFeedbackApp *app)
{
}

static void
gooroom_feedback_app_startup (GApplication *app)
{
  G_APPLICATION_CLASS (gooroom_feedback_app_parent_class)->startup (app);
}

static void
gooroom_feedback_app_activate (GApplication *app)
{
  GooroomFeedbackAppWindow *win;

  win = gooroom_feedback_app_window_new (GOOROOM_FEEDBACK_APP (app));
  gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
  gtk_window_present (GTK_WINDOW (win));
}

static void
gooroom_feedback_app_class_init (GooroomFeedbackAppClass *class)
{
  G_APPLICATION_CLASS (class)->startup = gooroom_feedback_app_startup;
  G_APPLICATION_CLASS (class)->activate = gooroom_feedback_app_activate;
}

GooroomFeedbackApp *
gooroom_feedback_app_new (void)
{
  return g_object_new (GOOROOM_FEEDBACK_APP_TYPE,
                       "application-id", "kr.gooroom.gooroom-feedback",
                       "flags", G_APPLICATION_HANDLES_OPEN,
                       NULL);
}
