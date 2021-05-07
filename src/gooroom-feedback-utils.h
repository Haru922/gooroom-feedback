#ifndef _GOOROOM_FEEDBACK_UTILS_H_
#define _GOOROOM_FEEDBACK_UTILS_H_

#include <stdio.h>
#include <string.h>
#include <glib-object.h>
#include <curl/curl.h>

#define GOOROOM_OS_INFO "/etc/gooroom/info"
#define GFB_PROXY_SERVER_URL "http://127.0.0.1:8000/feedback/new/"

int gfb_get_os_info (char **release, char **code_name);
gboolean gfb_post_request (const char *title, char *category, char *release, char *code_name, char *description);

#endif
