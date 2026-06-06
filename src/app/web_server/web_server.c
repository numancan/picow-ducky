#include "web_server.h"

#include "FreeRTOS.h"
#include "app/task_manager/task_manager.h"
#include "app/wifi/wifi.h"
#include "cgi.h"
#include "lwip/apps/httpd.h"
#include "middleware/log.h"
#include "ssi.h"
#include "task.h"

static const char* TAG = "web_server";

static WebServerState web_server_state = WEB_SERVER_STATE_STOPPED;

WebServerState web_server_get_state(void) { return web_server_state; }

void web_server_start(void) {
    httpd_init();
    ssi_init();
    cgi_init();
    web_server_state = WEB_SERVER_STATE_RUNNING;
    LOG_INFO(TAG, "httpd & ssi initialized");
}

void web_server_task(void* pvParameters) {
    (void)pvParameters;

    if (!wifi_init()) goto cleanup_none;
    if (!wifi_connect()) goto cleanup_wifi;
    wifi_mdns_start();

    web_server_start();

    // HTTP requests are serviced by lwIP's background TCPIP thread, so this task
    // only waits (zero CPU) for a stop signal to tear down wifi gracefully.
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    web_server_state = WEB_SERVER_STATE_STOPPED;
cleanup_wifi:
    wifi_deinit();
cleanup_none:
    task_manager_report_stopped(TASK_MANAGER_TASK_WEB_SERVER);
    vTaskDelete(NULL);
}
