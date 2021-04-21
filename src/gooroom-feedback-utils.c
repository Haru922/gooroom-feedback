#include "gooroom-feedback-utils.h"

int
gfb_get_os_info (char **release,
                 char **code_name)
{
  FILE *fp = NULL;
  int ret = -1;
  char line[BUFSIZ] = { 0, };
  const char *tok = "=";
  char *token = NULL;
  char *key = NULL;
  char *value = NULL;
  int len = 0;

  *release = NULL;
  *code_name = NULL;

  if (fp = fopen(GOOROOM_OS_INFO, "r"))
  {
    while (fgets (line, BUFSIZ, fp))
    {
      len = strlen (line);
      if (0 < len && line[len-1] == '\n')
        line[len-1] = '\0';
      token = strtok (line, tok);
      while (token) {
        if (!key)
          key = token;
        else
          value = token;
        token = strtok (NULL, tok);
      }
      if (strcmp ("RELEASE", key) == 0)
        *release = strdup (value);
      else if (strcmp ("CODENAME", key) == 0)
        *code_name = strdup (value);
    }
  }
  fclose (fp);

  if (*release && *code_name)
    ret = 0;

  return ret;
}
