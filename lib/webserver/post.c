#include <stdio.h>
#include <string.h>

#include "lwip/apps/httpd.h"

#if !LWIP_HTTPD_SUPPORT_POST
#error This needs LWIP_HTTPD_SUPPORT_POST
#endif

#define USER_PASS_BUFSIZE 128

static void *current_connection;
static void *valid_connection;

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

  if (!memcmp(uri, "/upload.cgi", 11)) {
    if (current_connection != connection) {
      current_connection = connection;
      valid_connection = NULL;
      // printf("req: %s, content len:%d\n", http_request, content_len);

      /* default page is "login failed" */
      // snprintf(response_uri, response_uri_len, "/loginfail.html");

      // strtok((char *)http_request, "&");
      // char *pFname = strtok(NULL, "&");

      // if (!pFname) return ERR_VAL;
      // // sdm_mount();

      // // File name uzunluğu bak
      // // isim başka path içermemeli kritik
      // char fn[100];
      // snprintf(fn, sizeof(fn), "%s%s", "payloads/", pFname);
      // sdm_open_write(fn);

      /* e.g. for large uploads to slow flash over a fast connection, you should
         manually update the rx window. That way, a sender can only send a full
         tcp window at a time. If this is required, set 'post_aut_wnd' to 0.
         We do not need to throttle upload speed here, so: */
      *post_auto_wnd = 0;
      return ERR_OK;
    }
  }
  else if (!memcmp(uri, "/settings.cgi", 13)) {
    
    if (current_connection != connection) {
      current_connection = connection;
      valid_connection = NULL;

      // char fn[100];
      // snprintf(fn, sizeof(fn), "%s", "config.txt");
      // printf("open write %d\n", sdm_open_write(fn));
      
      *post_auto_wnd = 0;
      return ERR_OK;
    }
  }
  else if (!memcmp(uri, "/trigger.cgi", 12)) {

    if (current_connection != connection) {
      current_connection = connection;
      valid_connection = NULL;
    

      *post_auto_wnd = 0;
      return ERR_OK;
    }
  }

  return ERR_VAL;
}

char buff[1400];

err_t
httpd_post_receive_data(void *connection, struct pbuf *p)
{
  // printf("variable A is at address: %p\n", (void*)&p);
  // printf("The size of pbuf is %zu\n", sizeof(p));

  if (current_connection == connection) {
    
    char *pt = (char*)pbuf_get_contiguous(p, buff, sizeof(buff), p->len, 0);
    printf("http_post_receive_cb!\n\n");
    printf("buffer:\n--------\n %s\n", pt);
    // sdm_printf(pt);

    pbuf_free(p);
    return ERR_OK;
  }
  
  return ERR_VAL;
}

void
httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len)
{
  printf("httpd_post_finished!\n\n");
  // clear pbuf
  if (current_connection == connection) {
    snprintf(response_uri, response_uri_len, "/index.shtml");
    // sdm_close_file();
    // sdm_unmount();
    current_connection = NULL;
    valid_connection = NULL;
  }
}