#pragma once

typedef enum {
    WEB_SERVER_STATE_STOPPED = 0,
    WEB_SERVER_STATE_RUNNING,
} WebServerState;

void web_server_start(void);  // httpd_init + ssi_init + cgi_init
WebServerState web_server_get_state(void);
void web_server_task(void* pvParameters);  // orchestrator: wifi + http
