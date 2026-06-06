#pragma once

#include <stdbool.h>

typedef enum {
    WIFI_STATE_UNINITIALIZED = 0,
    WIFI_STATE_CYW43_INIT,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_ERR_CYW43_INIT,
    WIFI_STATE_ERR_CONNECT,
} WifiState;

bool wifi_init(void);        // cyw43_arch_init + enable STA mode + hostname
bool wifi_connect(void);     // retry connect loop
void wifi_mdns_start(void);  // mdns_resp_init + _http service advertisement
void wifi_deinit(void);      // mdns remove + cyw43_arch_deinit
WifiState wifi_get_state(void);
