#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

typedef enum {
    WS_STATE_UNINITIALIZED = 0,
    WS_STATE_CYW43_INIT,
    WS_STATE_WIFI_CONNECTING,
    WS_STATE_WIFI_CONNECTED,
    WS_STATE_MDNS_INIT,
    WS_STATE_HTTPD_INIT,
    WS_STATE_SSI_CGI_INIT,
    WS_STATE_RUNNING,
    WS_STATE_ERR_CYW43_INIT,
    WS_STATE_ERR_WIFI_CONN
} web_server_state_t;

web_server_state_t web_server_get_state(void);
void web_server_task(void* pvParameters);

#endif