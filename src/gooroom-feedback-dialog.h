#ifndef _GOOROOM_FEEDBACK_DIALOG_H_
#define _GOOROOM_FEEDBACK_DIALOG_H_

#include <time.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include "gooroom-feedback-utils.h"

#define GOOROOM_FEEDBACK_DIALOG_TYPE (gooroom_feedback_dialog_get_type ())

#define GFB_TITLE_LEN 25

#define GOOROOM_FEEDBACK_DIALOG_UI "/kr/gooroom/gooroom-feedback/gooroom-feedback-dialog.ui"
#define GOOROOM_FEEDBACK_CSS       "/kr/gooroom/gooroom-feedback/gooroom-feedback.css"
#define GOOROOM_FEEDBACK_CONF      "/etc/gooroom/gooroom-feedback/gooroom-feedback.conf"

G_DECLARE_FINAL_TYPE (GooroomFeedbackDialog, gooroom_feedback_dialog, GOOROOM_FEEDBACK, DIALOG, GtkDialog)

GooroomFeedbackDialog *gooroom_feedback_dialog_new (void);

#endif
