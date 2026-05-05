#include "web_server.h"

#include "FreeRTOS.h"
#include "cgi.h"
#include "lwip/apps/httpd.h"
#include "lwip/apps/mdns.h"
#include "lwip/init.h"
#include "lwip/ip4_addr.h"
#include "lwipopts.h"
#include "pico/cyw43_arch.h"
#include "ssi.h"
#include "task.h"
#include "app/task_manager/task_manager.h"

#define WIFI_CONNECTION_TIMEOUT 10000
#define WIFI_MAX_RETRIES 10

static void srv_txt(struct mdns_service* service, void* txt_userdata) {
    err_t res;
    LWIP_UNUSED_ARG(txt_userdata);

    res = mdns_resp_add_service_txtitem(service, "path=/", 6);
    LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}

static web_server_state_t web_server_state = WS_STATE_UNINITIALIZED;

web_server_state_t ws_get_state(void) { return web_server_state; }

static bool web_server_wifi_connect(void) {
    int retries = 0;

    while (retries < WIFI_MAX_RETRIES) {
        int err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK,
                                                     WIFI_CONNECTION_TIMEOUT);

        if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP) {
            printf("WIFI CONN ERR:%d, attempt %d of %d\n", err, retries + 1, WIFI_MAX_RETRIES);
            retries++;
            cyw43_delay_ms(1000);
        } else {
            return true;
        }
    }
    return false;
}

void web_server_task(void* pvParameters) {
    (void)pvParameters;

    web_server_state = WS_STATE_CYW43_INIT;

    if (cyw43_arch_init() != PICO_OK) {
        web_server_state = WS_STATE_ERR_CYW43_INIT;
        goto cleanup_none;
    }

    cyw43_arch_enable_sta_mode();

    char hostname[] = "picow_ducky";
    netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], hostname);

    web_server_state = WS_STATE_WIFI_CONNECTING;
    if (!web_server_wifi_connect()) {
        printf("Failed to connect to WiFi after %d attempts.\n", WIFI_MAX_RETRIES);
        web_server_state = WS_STATE_ERR_WIFI_CONN;
        goto cleanup_cyw43;
    }

    web_server_state = WS_STATE_WIFI_CONNECTED;
    printf("Wifi connected!\n");

    web_server_state = WS_STATE_MDNS_INIT;
    mdns_resp_init();
    printf("mdns host name %s.local\n", hostname);
    mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname);
    mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "picow_ducky", "_http", DNSSD_PROTO_TCP, 80, srv_txt,
                          NULL);

    ip_addr_t* ip_addr = &cyw43_state.netif[CYW43_ITF_STA].ip_addr;
    printf("IP Address: %s\n", ip4addr_ntoa(ip_addr));

    web_server_state = WS_STATE_HTTPD_INIT;
    httpd_init();
    ssi_init();
    cgi_init();

    web_server_state = WS_STATE_RUNNING;
    printf("HTTPD&SSI initialized!\n");

    while (1) {
        // TODO: we can use portMAX_DELAY instead of 100ms delay idk
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100))) {
            break;
        }
    }

    mdns_resp_remove_netif(&cyw43_state.netif[CYW43_ITF_STA]);
cleanup_cyw43:
    cyw43_arch_deinit();
cleanup_none:
    task_manager_report_stopped(TASK_MANAGER_TASK_WEB_SERVER);
    vTaskDelete(NULL);
}