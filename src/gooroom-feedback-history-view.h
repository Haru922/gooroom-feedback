#ifndef _GOOROOM_FEEDBACK_HISTORY_VIEW_H_
#define _GOOROOM_FEEDBACK_HISTORY_VIEW_H_

#include <gtk/gtk.h>

#define GFB_HISTORY "/home/haru/feedback.log"

enum _gfb_history_column {
  GFB_HISTORY_DATE,
  GFB_HISTORY_TITLE,
  GFB_HISTORY_RESULT,
  //GFB_HISTORY_TYPE,
  //GFB_HISTORY_OS,
  GFB_HISTORY_COLUMNS
};

static char *column_names[] = {
  "DATE",
  "TITLE",
  "RESULT"
  //"TYPE",
  //"OS"
};

void gooroom_feedback_history_view_init (GtkWidget *gfb_history_window);
void gooroom_feedback_history_view_get_items (GtkWidget *gfb_history_view);

#endif