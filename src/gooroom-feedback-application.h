#ifndef _GOOROOM_FEEDBACK_APPLICATION_H_
#define _GOOROOM_FEEDBACK_APPLICATION_H_

#include <gtk/gtk.h>

#define GOOROOM_FEEDBACK_APP_TYPE (gooroom_feedback_app_get_type ())
G_DECLARE_FINAL_TYPE (GooroomFeedbackApp, gooroom_feedback_app, GOOROOM_FEEDBACK, APP, GtkApplication)

GooroomFeedbackApp *gooroom_feedback_app_new (void);

#endif
