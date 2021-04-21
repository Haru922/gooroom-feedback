#ifndef _GOOROOM_FEEDBACK_WINDOW_H_
#define _GOOROOM_FEEDBACK_WINDOW_H_

#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include "gooroom-feedback-application.h"
#include "gooroom-feedback-utils.h"

#define GOOROOM_FEEDBACK_APP_WINDOW_TYPE (gooroom_feedback_app_window_get_type ())
#define GOOROOM_FEEDBACK_WINDOW_UI "/kr/gooroom/gooroom-feedback/gooroom-feedback.ui"
G_DECLARE_FINAL_TYPE (GooroomFeedbackAppWindow, gooroom_feedback_app_window, GOOROOM_FEEDBACK, APP_WINDOW, GtkApplicationWindow)

GooroomFeedbackAppWindow *gooroom_feedback_app_window_new (GooroomFeedbackApp *app);

#endif
