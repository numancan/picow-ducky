#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

typedef enum {
  WS_ERR_OK = 0,
  WS_ERR_WIFI_INIT,
  WS_ERR_WIFI_CONN
} ws_err_t;

void ws_init();

#endif