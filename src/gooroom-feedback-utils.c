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

int
gfb_post_request (char *server_url,
                  const char *title,
                  char *category,
                  char *release,
                  char *code_name,
                  char *description)
{
  CURL *curl;
  CURLcode res;
  struct curl_slist *list = NULL;
  char feedback[BUFSIZ] = { 0, };
  char *feedback_fmt = "title=%s&"
                       "category=%s&"
                       "release=%s&"
                       "codename=%s&"
                       "description=%s";
  int ret = GFB_RESPONSE_SUCCESS;

  curl = curl_easy_init ();
  curl_slist_append (list, "Content-Type: application/json");
  if (curl) {
    curl_easy_setopt (curl, CURLOPT_URL, server_url);
    curl_easy_setopt (curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt (curl, CURLOPT_POST, 1L);
    snprintf (feedback, BUFSIZ, feedback_fmt,
              title, category, release, code_name, description);
    printf ("%s\n", feedback);
    curl_easy_setopt (curl, CURLOPT_POSTFIELDS, feedback);
    res = curl_easy_perform (curl);
    curl_easy_cleanup (curl);
    curl_slist_free_all (list);
    if (res != CURLE_OK)
      ret = GFB_RESPONSE_FAILURE;
  }
  return ret;
}
