#ifndef _GOOROOM_FEEDBACK_UTILS_H_
#define _GOOROOM_FEEDBACK_UTILS_H_

#include <stdio.h>
#include <string.h>
#include <glib-object.h>
#include <curl/curl.h>

#define GFB_RESPONSE_SUCCESS 201
#define GFB_RESPONSE_FAILURE   0

#define GOOROOM_OS_INFO "/etc/gooroom/info"

int         gfb_get_os_info           (char **release, char **code_name);
int         gfb_post_request          (char *server_url, const char *title, char *category, char *release, char *code_name, char *description);
const char *gfb_status_code_to_string (int status_code);

#endif
