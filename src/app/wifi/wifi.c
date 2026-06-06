#include "wifi.h"

#include "lwip/apps/mdns.h"
#include "lwip/ip4_addr.h"
#include "lwipopts.h"
#include "middleware/log.h"
#include "pico/cyw43_arch.h"

#define WIFI_CONNECTION_TIMEOUT 10000
#define WIFI_MAX_RETRIES 10

static const char* TAG = "wifi";
static const char hostname[] = "picow_ducky";

static WifiState wifi_state = WIFI_STATE_UNINITIALIZED;

WifiState wifi_get_state(void) { return wifi_state; }

static void srv_txt(struct mdns_service* service, void* txt_userdata) {
    err_t res;
    LWIP_UNUSED_ARG(txt_userdata);

    res = mdns_resp_add_service_txtitem(service, "path=/", 6);
    LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}

bool wifi_init(void) {
    wifi_state = WIFI_STATE_CYW43_INIT;

    if (cyw43_arch_init() != PICO_OK) {
        wifi_state = WIFI_STATE_ERR_CYW43_INIT;
        return false;
    }

    cyw43_arch_enable_sta_mode();
    netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], hostname);
    return true;
}

bool wifi_connect(void) {
    wifi_state = WIFI_STATE_CONNECTING;

    int retries = 0;
    while (retries < WIFI_MAX_RETRIES) {
        int err = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK,
                                                     WIFI_CONNECTION_TIMEOUT);

        if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP) {
            LOG_WARN(TAG, "connect error %d, attempt %d of %d", err, retries + 1, WIFI_MAX_RETRIES);
            retries++;
            cyw43_delay_ms(1000);
        } else {
            wifi_state = WIFI_STATE_CONNECTED;
            LOG_INFO(TAG, "connected, IP %s", ip4addr_ntoa(&cyw43_state.netif[CYW43_ITF_STA].ip_addr));
            return true;
        }
    }

    wifi_state = WIFI_STATE_ERR_CONNECT;
    LOG_ERROR(TAG, "failed to connect after %d attempts", WIFI_MAX_RETRIES);
    return false;
}

void wifi_mdns_start(void) {
    mdns_resp_init();
    mdns_resp_add_netif(&cyw43_state.netif[CYW43_ITF_STA], hostname);
    mdns_resp_add_service(&cyw43_state.netif[CYW43_ITF_STA], hostname, "_http", DNSSD_PROTO_TCP, 80, srv_txt, NULL);
    LOG_INFO(TAG, "mdns host name %s.local", hostname);
}

void wifi_deinit(void) {
    mdns_resp_remove_netif(&cyw43_state.netif[CYW43_ITF_STA]);
    cyw43_arch_deinit();
    wifi_state = WIFI_STATE_UNINITIALIZED;
}
