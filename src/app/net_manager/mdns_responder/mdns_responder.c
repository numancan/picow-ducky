#include "mdns_responder.h"

#include "lwip/apps/mdns.h"
#include "middleware/log.h"

static const char* TAG = "mdns";

#define MDNS_HTTP_PORT 80

static void srv_txt(struct mdns_service* service, void* txt_userdata) {
    (void)txt_userdata;
    if (mdns_resp_add_service_txtitem(service, "path=/", 6) != ERR_OK) {
        LOG_WARN(TAG, "txt item add failed");
    }
}

static bool mdns_initialized = false;

void mdns_responder_init(void) {
    if (!mdns_initialized) {
        mdns_resp_init();
        mdns_initialized = true;
    }
}

void mdns_responder_start(struct netif* netif, const char* hostname) {
    err_t err = mdns_resp_add_netif(netif, hostname);
    s8_t slot = -1;
    if (err == ERR_OK) {
        slot = mdns_resp_add_service(netif, hostname, "_http", DNSSD_PROTO_TCP, MDNS_HTTP_PORT, srv_txt, NULL);
    }

    if (err != ERR_OK) {
        LOG_WARN(TAG, "add_netif failed: %d", err);
    } else if (slot < 0) {
        LOG_WARN(TAG, "add_service failed: %d", slot);
    } else {
        LOG_INFO(TAG, "advertising %s.local", hostname);
        mdns_resp_announce(netif);
    }
}

void mdns_responder_stop(struct netif* nif) { mdns_resp_remove_netif(nif); }
