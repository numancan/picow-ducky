#include <stdio.h>
#include <string.h>

#include "lwip/apps/httpd.h"
#include "sd_memory.h"

#include "config.h"
#include "post.h"

static void *current_connection;
static void *valid_connection;

enum {
  INVALID_POST = -1,
  POST_UPLOAD = 0,
  POST_SETTINGS,
  POST_TRIGGER
};

// DONT CHANGE ORDER
const char const* post_uris[] = {
  "/upload",
  "/settings",
  "/trigger"
};

// -1 mean no geçerli post req
static int8_t current_post_req_idx = INVALID_POST;

int8_t get_post_idx(const char *uri)
{
  uint count = count_of(post_uris);
  for (uint8_t i = 0; i < count; i++)  
    if (memcmp(uri, post_uris[i], strlen(post_uris[i]) -1) == 0)  return i;
  
  return -1;
}

// now just sigle char param
char *get_uri_param(char *uri, const char *param)
{
  strtok(uri, "?");
  char *pTemp = strtok(NULL, "=");

  if (strcmp(pTemp, param) == 0) {
    pTemp = strtok(NULL, "");
    return pTemp;
  }

  return NULL;
}

err_t
httpd_post_begin(void *connection, const char *uri, const char *http_request,
                 u16_t http_request_len, int content_len, char *response_uri,
                 u16_t response_uri_len, u8_t *post_auto_wnd)
{
  LWIP_UNUSED_ARG(connection);
  LWIP_UNUSED_ARG(http_request);
  LWIP_UNUSED_ARG(http_request_len);
  LWIP_UNUSED_ARG(content_len);

  printf("uri: %s, httpd_post_begin!\n\n", uri);

  current_post_req_idx = get_post_idx(uri);

  switch (current_post_req_idx)
  {
    case POST_UPLOAD:
      if (current_connection != connection) {
        current_connection = connection;
        snprintf(response_uri, response_uri_len, "/index.shtml");
        // printf("req: %s, content len:%d\n", http_request, content_len);

        /** TODO: (char *) baya tatsız */
        char *pTemp = get_uri_param((char *)uri, "p");

        if (!pTemp || 
            !sdm_check_file_ext(pTemp, "txt") ||
            strlen(pTemp) > FILE_MAX_NAME_LEN ) 
            return ERR_VAL;

        char fn[FILE_MAX_NAME_LEN + 9];
        snprintf(fn, sizeof(fn), "%s%s", "payloads/", pTemp);

        FRESULT f_res = sdm_open_write(fn);

        /* e.g. for large uploads to slow flash over a fast connection, you should
          manually update the rx window. That way, a sender can only send a full
          tcp window at a time. If this is required, set 'post_aut_wnd' to 0.
          We do not need to throttle upload speed here, so: */
        *post_auto_wnd = 0;
        return f_res == FR_OK ? ERR_OK : ERR_VAL;
      }
      break;

    case POST_SETTINGS:
      if (current_connection != connection) {
        current_connection = connection;
        snprintf(response_uri, response_uri_len, "/index.shtml");

        // char fn[100];
        // snprintf(fn, sizeof(fn), "%s", "config.txt");
        /** TODO: check settings len? with settings*/
        FRESULT f_res = sdm_open_write("settings.txt");

        /** TODO: Bura kritik dosya yazılıp kapanıyor tekrar açılıp okunuyor eyvah eyvah*/
        // post_settings_cb();
        
        *post_auto_wnd = 0;
        return f_res == FR_OK ? ERR_OK : ERR_VAL;
      }
      break;

    case POST_TRIGGER:
      if (current_connection != connection) {
        current_connection = connection;
        snprintf(response_uri, response_uri_len, "/index.shtml");
        
        /** TODO: (char *) baya tatsız */
        char *pTemp = get_uri_param((char *)uri, "p");

        if (!pTemp || 
            !sdm_check_file_ext(pTemp, "txt") ||
            strlen(pTemp) > FILE_MAX_NAME_LEN ) 
            return ERR_VAL;

        post_trigger_cb(pTemp);
        *post_auto_wnd = 0;
        return ERR_OK;
      }
      break;

    default:
      break;
  }

  return ERR_VAL;
}

char buff[1400];

err_t
httpd_post_receive_data(void *connection, struct pbuf *p)
{
  if (current_connection == connection && current_post_req_idx == POST_UPLOAD) {
    
    char *pt = (char*)pbuf_get_contiguous(p, buff, sizeof(buff), p->len, 0);

    printf("buffer:\n--------\n %s\n", pt);
    sdm_printf(pt);

    pbuf_free(p);
    return ERR_OK;
  }
  
  return ERR_VAL;
}

void
httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
  if (current_connection == connection) {
    snprintf(response_uri, response_uri_len, "/index.shtml");

    sdm_close_file();
    if (current_post_req_idx == POST_SETTINGS) post_settings_cb();

    current_connection = NULL;
    valid_connection = NULL;
    current_post_req_idx = INVALID_POST;
  }
}