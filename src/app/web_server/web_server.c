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

#define WIFI_CONNECTION_TIMEOUT 10000

static void srv_txt(struct mdns_service* service, void* txt_userdata) {
    err_t res;
    LWIP_UNUSED_ARG(txt_userdata);

    res = mdns_resp_add_service_txtitem(service, "path=/", 6);
    LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}

void ws_init() {
    // if (cyw43_arch_init()) return WS_ERR_WIFI_INIT;
    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    char hostname[] = "picow_ducky";
    // memcpy(&hostname[0], CYW43_HOST_NAME, sizeof(CYW43_HOST_NAME) - 1);
    // get_mac_ascii(CYW43_HAL_MAC_WLAN0, 8, 4, &hostname[sizeof(CYW43_HOST_NAME) - 1]);
    // hostname[sizeof(hostname) - 1] = '\0';
    netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], hostname);

    // if (cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM)) return WS_ERR_WIFI_INIT;

    while (1) {
        int err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK,
                                                     WIFI_CONNECTION_TIMEOUT);

        if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP) {
            printf("WIFI CONN ERR:%d\n", err);
        } else {
            break;
        }

        cyw43_delay_ms(1000);
    }

    printf("Wifi connected!\n");

    mdns_resp_init();
    printf("mdns host name %s.local\n", hostname);
    mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname);
    mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], "picow_freertos_httpd", "_http", DNSSD_PROTO_TCP, 80,
                          srv_txt, NULL);

    ip_addr_t* ip_addr = &cyw43_state.netif[CYW43_ITF_STA].ip_addr;
    printf("IP Address: %s\n", ip4addr_ntoa(ip_addr));

    httpd_init();
    ssi_init();
    cgi_init();
    printf("HTTPD&SSI initialized!\n");

    while (1) {
        vTaskDelay(100);
    }

    mdns_resp_remove_netif(&cyw43_state.netif[CYW43_ITF_STA]);

    cyw43_arch_deinit();
}