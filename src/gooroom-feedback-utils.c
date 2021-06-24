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
  long ret = GFB_RESPONSE_FAILURE;
  char *content = NULL;

  curl = curl_easy_init ();
  curl_slist_append (list, "Content-Type: application/json");

  if (curl) {
    curl_easy_setopt (curl, CURLOPT_URL, server_url);
    curl_easy_setopt (curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt (curl, CURLOPT_POST, 1L);
    snprintf (feedback, BUFSIZ, feedback_fmt,
              title, category, release, code_name, description);
    //printf ("%s\n", feedback);
    curl_easy_setopt (curl, CURLOPT_POSTFIELDS, feedback);
    res = curl_easy_perform (curl);
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &ret);
    curl_easy_getinfo (curl, CURLINFO_CONTENT_TYPE, &content); 
    curl_easy_cleanup (curl);
    curl_slist_free_all (list);
  }

  return ret;
}

char *
gfb_status_code_to_string (int status_code)
{
  switch (status_code) {
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 102: return "Processing";
    case 103: return "Early Hints";
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non Authoritative Info";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 207: return "Multi Status";
    case 208: return "Already Reported";
    case 226: return "IMUsed";
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Auth Required";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Request Entity Too Large";
    case 414: return "Request URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Requested Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 418: return "Teapot";
    case 421: return "Misdirected Request";
    case 422: return "Unprocessable Entity";
    case 423: return "Locked";
    case 424: return "Failed Dependency";
    case 425: return "Too Early";
    case 426: return "Upgrade Required";
    case 428: return "Precondition Required";
    case 429: return "Too Many Requests";
    case 431: return "Request Header Fields Too Large";
    case 451: return "Unavailable For Legal Reasons";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
    case 506: return "Variant Also Negotiages";
    case 507: return "Insufficient Storage";
    case 508: return "Loop Detected";
    case 510: return "Not Extended";
    case 511: return "Network Authentication Required";
    default: return "Unknown Status";
  }
}
