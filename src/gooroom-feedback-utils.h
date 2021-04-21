#ifndef _GOOROOM_FEEDBACK_UTILS_H_
#define _GOOROOM_FEEDBACK_UTILS_H_

#include <stdio.h>
#include <string.h>

#define GOOROOM_OS_INFO "/etc/gooroom/info"

int gfb_get_os_info (char **release, char **code_name);

#endif
