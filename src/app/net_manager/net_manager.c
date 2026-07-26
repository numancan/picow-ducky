#include "net_manager.h"

#include <stdatomic.h>
#include <string.h>

#include "FreeRTOS.h"
#include "config/task_config.h"
#include "dhcpserver/dhcpserver.h"
#include "dnsserver/dnsserver.h"
#include "gui/gui.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "mdns_responder/mdns_responder.h"
#include "middleware/log.h"
#include "middleware/sleep_manager.h"
#include "middleware/sys_fault.h"
#include "net_config.h" /* net_config_load_wifi / net_config_save_wifi / net_config_erase_wifi */
#include "net_manager_view.h"
#include "pico/cyw43_arch.h" /* CYW43_LINK_* status codes */
#include "queue.h"
#include "radio_manager.h"
#include "task.h"
#include "timers.h"
#include "web_server/web_server.h"
#include "wifi_scan_list.h"

#define NET_QUEUE_LEN 8

#define CONNECT_TIMEOUT_MS 20000
#define CONNECT_MAX_ATTEMPTS 5u
#define POLL_PERIOD_MS 500

#define NET_AP_SSID DEVICE_NAME
#define NET_AP_PASS NULL /* open network */

static const char* TAG = "net";

static QueueHandle_t net_event_queue;
static TimerHandle_t poll_timer; /* link-loss detection while CONNECTED */
static _Atomic NetState state = NET_STATE_IDLE;

/* Abort latch for an in-flight do_connect(): STOP can't be dequeued while
   the task blocks in the connect loop, so net_manager_stop() also raises
   this flag. It stays raised until the next explicit start_networking(), so
   any connect attempt (including a POLL-driven reconnect) between the stop
   and a fresh start aborts too. Standalone flag, no other data published
   -> relaxed. */
static _Atomic bool stop_requested = false;

static dhcp_server_t dhcp;
static dns_server_t dns;

static NetManagerView net_manager_view;

static void stop_networking(void); /* used by the connect flows on abort */

/* Applies all side effects of a transition; invariant: poll timer and mDNS
   responder <=> CONNECTED. mDNS only changes on the CONNECTED entry/exit
   edge: since the netif is persistent, add is required on every connect and
   remove on every disconnect (otherwise reconnecting triggers a "double
   add"). */
static void enter_state(NetState st) {
    NetState prev = state;
    LOG_INFO(TAG, "enter_state: %s -> %s", net_manager_get_status_str(prev), net_manager_get_status_str(st));

    if (st == NET_STATE_CONNECTED && prev != NET_STATE_CONNECTED) {
        radio_lwip_lock();
        mdns_responder_start(&cyw43_state.netif[CYW43_ITF_STA], DEVICE_NAME);
        radio_lwip_unlock();
        xTimerStart(poll_timer, 0);
    } else if (st != NET_STATE_CONNECTED && prev == NET_STATE_CONNECTED) {
        radio_lwip_lock();
        mdns_responder_stop(&cyw43_state.netif[CYW43_ITF_STA]);
        radio_lwip_unlock();
        xTimerStop(poll_timer, 0);
    }

    state = st;
}

/* Timer task context: no radio calls, just post. If the queue is full the
   event is dropped; the next tick makes up for it. */
static void poll_timer_cb(TimerHandle_t t) {
    (void)t;
    NetEvent e = {.type = NET_EVT_POLL};
    net_manager_post(&e);
}

/* Bring the provisioning infrastructure up/down as a unit */
static void start_provisioning(void) {
    radio_ap_start(NET_AP_SSID, NET_AP_PASS);

    ip4_addr_t mask;
    ip4_addr_t gw;
    gw.addr = PP_HTONL(DEVICE_IP_AP_ADDRESS);
    mask.addr = PP_HTONL(DEVICE_IP_AP_MASK);

    radio_lwip_lock();
    netif_set_default(&cyw43_state.netif[CYW43_ITF_AP]);
    dhcp_server_init(&dhcp, &cyw43_state.netif[CYW43_ITF_AP], &gw, &mask);
    dns_server_init(&dns, &cyw43_state.netif[CYW43_ITF_AP], &gw);
    radio_lwip_unlock();

    wifi_scan_start();
    enter_state(NET_STATE_PROVISIONING);
}

static void stop_provisioning(void) {
    wifi_scan_stop();

    radio_lwip_lock();

    dns_server_deinit(&dns);
    dhcp_server_deinit(&dhcp);
    if (netif_default == &cyw43_state.netif[CYW43_ITF_AP]) netif_set_default(NULL);
    radio_lwip_unlock();

    radio_ap_stop();
}

/* Blocking connect: attempts + per-attempt timeout, straight-line. The net
   task sits in here for the duration (worst case ~ATTEMPTS*TIMEOUT); events
   posted meanwhile queue up (or are dropped once the queue is full) and are
   handled afterwards in whatever state we land in -- acceptable by design,
   nothing mid-connect needs a reaction. */
static bool do_connect(const char* ssid, const char* pass) {
    wifi_scan_abort(); /* single radio: a physical scan disrupts the join */

    for (unsigned attempt = 1; attempt <= CONNECT_MAX_ATTEMPTS; attempt++) {
        LOG_INFO(TAG, "connecting to \"%s\" (attempt %u/%u)", ssid, attempt, CONNECT_MAX_ATTEMPTS);

        if (radio_sta_connect_async(ssid, pass) != 0) {
            LOG_WARN(TAG, "radio_sta_connect_async failed");
            continue; /* join never started; a DOWN status would never change */
        }

        TickType_t start = xTaskGetTickCount();
        while ((TickType_t)(xTaskGetTickCount() - start) < pdMS_TO_TICKS(CONNECT_TIMEOUT_MS)) {
            vTaskDelay(pdMS_TO_TICKS(POLL_PERIOD_MS));

            if (atomic_load_explicit(&stop_requested, memory_order_relaxed)) {
                LOG_INFO(TAG, "connect aborted: stop requested");
                return false;
            }

            int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);

            if (status == CYW43_LINK_UP) {
                LOG_INFO(TAG, "connected to \"%s\"", ssid);
                return true;
            }
            if (status == CYW43_LINK_BADAUTH) {
                LOG_WARN(TAG, "connect to \"%s\" failed: bad auth", ssid);
                return false; /* permanent, no retry */
            }
            if (status == CYW43_LINK_NONET) {
                LOG_INFO(TAG, "Reconnecting to \"%s\" (NOIP detected)", ssid);
                /* Mirrors cyw43_arch_wifi_connect_bssid_until(): re-issue
                   the join; the per-attempt timeout keeps running. */
                radio_sta_connect_async(ssid, pass);
            } else if (status < 0) {
                /* FAIL etc.: final for this attempt, don't wait out the
                   window. */
                LOG_WARN(TAG, "connect attempt %u failed, status=%d", attempt, status);
                break;
            }
            /* DOWN/JOIN/NOIP: still in progress, keep waiting. */
        }
    }

    LOG_WARN(TAG, "connect to \"%s\" failed after %u attempts", ssid, CONNECT_MAX_ATTEMPTS);
    return false;
}

/* Portal flow: try the submitted credentials while keeping the AP infra up;
   persist and tear the portal down only on success. */
static void connect_from_portal(const char* ssid, const char* pass) {
    enter_state(NET_STATE_PROV_CONNECTING); /* status reporting only */
    if (do_connect(ssid, pass)) {
        net_config_save_wifi(ssid, pass); /* success: persist */
        stop_provisioning();
        enter_state(NET_STATE_CONNECTED);
    } else if (atomic_load_explicit(&stop_requested, memory_order_relaxed)) {
        stop_networking(); /* aborted mid-connect: tear down and release now */
    } else {
        radio_sta_disconnect();
        /* Portal (AP) infra is still up; restore it as the default netif
           since radio_sta_disconnect() no longer does that itself. */
        radio_lwip_lock();
        netif_set_default(&cyw43_state.netif[CYW43_ITF_AP]);
        radio_lwip_unlock();
        wifi_scan_start(); /* keep the portal list live */
        enter_state(NET_STATE_PROVISIONING);
    }
}

/* Boot/reconnect flow: connect with the identity from config; if that fails
   (or there's no config), fall back to provisioning. */
static void connect_from_config_or_provision(void) {
    char ssid[NET_SSID_MAX + 1], pass[NET_PASS_MAX + 1];
    if (!net_config_load_wifi(ssid, sizeof ssid, pass, sizeof pass)) {
        start_provisioning();
        return;
    }

    enter_state(NET_STATE_STA_CONNECTING); /* status reporting only */
    if (do_connect(ssid, pass)) {
        enter_state(NET_STATE_CONNECTED);
    } else if (atomic_load_explicit(&stop_requested, memory_order_relaxed)) {
        stop_networking(); /* aborted mid-connect: tear down and release now */
    } else {
        radio_sta_disconnect();
        start_provisioning();
    }
}

/* Bring networking up on external request: claim the radio, one-time init
   of the raw-API pcbs, then run the boot flow. Acquire failure is
   recoverable -- stay idle (radio_acquire leaks nothing on failure) so a
   later START can retry. */
static void start_networking(void) {
    // because of pico sdk issue#3096, we need to keep radio alive
    if (radio_acquire(RADIO_USER_KEEPALIVE) != 0) {
        LOG_WARN(TAG, "radio acquire failed; staying idle");
        return;
    }
    atomic_store_explicit(&stop_requested, false, memory_order_relaxed); /* clear any stale stop latch */

    radio_lwip_lock();
    mdns_responder_init(); /* both are one-shot guarded */
    web_server_init();     /* -> safe on every START    */
    radio_lwip_unlock();

    connect_from_config_or_provision();
}

/* Tear everything down and hand the radio back. enter_state(IDLE) runs
   while the radio is still up so the CONNECTED->IDLE edge can stop mdns +
   poll timer under the lwIP lock; release only afterwards. */
static void stop_networking(void) {
    switch (state) {
        case NET_STATE_PROV_CONNECTING: /* portal join in flight, AP infra up */
            radio_sta_disconnect();
            stop_provisioning();
            break;
        case NET_STATE_PROVISIONING: stop_provisioning(); break;
        case NET_STATE_STA_CONNECTING: /* boot/reconnect join in flight */
        case NET_STATE_CONNECTED: radio_sta_disconnect(); break;
        default: break;
    }
    enter_state(NET_STATE_IDLE);

    // because of pico sdk issue#3096, radio is not released here
    // radio_release(RADIO_USER_WIFI);
}

static void on_idle(const NetEvent* event) {
    if (event->type == NET_EVT_START) start_networking();
}

static void on_connected(const NetEvent* event) {
    switch (event->type) {
        case NET_EVT_POLL:
            /* Wifi-level status: a healthy connection is JOIN (only
               tcpip-level status returns UP). DOWN and negative error codes
               count as a disconnect. */
            if (cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) <= CYW43_LINK_DOWN) {
                LOG_WARN(TAG, "link lost");
                connect_from_config_or_provision();
            }
            break;

        case NET_EVT_FORGET:
            net_config_erase_wifi();
            radio_sta_disconnect();
            start_provisioning();
            break;

        case NET_EVT_STOP: stop_networking(); break;

        default: break;
    }
}

static void on_provisioning(const NetEvent* event) {
    switch (event->type) {
        case NET_EVT_SCAN_REQUEST:
            /* Results are collected by wifi_scan_list and published; the
               web_server SSI side reads them via wifi_scan_list_get(). */
            wifi_scan_start();
            break;

        case NET_EVT_CREDENTIALS: connect_from_portal(event->creds.ssid, event->creds.pass); break;

        case NET_EVT_STOP: stop_networking(); break;

        default: break;
    }
}

static void handle_event(const NetEvent* event) {
    switch (state) {
        case NET_STATE_IDLE: on_idle(event); break;
        case NET_STATE_CONNECTED: on_connected(event); break;
        case NET_STATE_PROVISIONING: on_provisioning(event); break;
        default: break; /* *_CONNECTING never handles events: do_connect blocks the task */
    }
}

/* Idle until an external NET_EVT_START arrives: no radio_* call happens
   before start_networking() runs radio_acquire (radio_manager contract).
   The radio setup, one-time pcb init and boot flow all live on the START
   path now. */
void net_manager_task(void* arg) {
    (void)arg;

    NetEvent event;
    for (;;) {
        if (xQueueReceive(net_event_queue, &event, portMAX_DELAY) != pdTRUE) continue;

        /* Handled globally, not via the state dispatch: the *_CONNECTING states
           drop events, but a sleep must always tear down and exit. */
        if (event.type == NET_EVT_SLEEP) break;

        handle_event(&event);
    }

    if (state != NET_STATE_IDLE) stop_networking();

    vQueueDelete(net_event_queue);
    xTimerDelete(poll_timer, 0);

    LOG_INFO(TAG, "shutdown");
    sleep_manager_ack_shutdown();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

/* sleep_manager shutdown hook: ask the task to tear down and exit. Raise the
   abort latch too so an in-flight do_connect() bails out fast and the radio is
   released before the device sleeps (mirrors net_manager_stop). */
static void net_manager_shutdown_cb(void* context) {
    (void)context;
    atomic_store_explicit(&stop_requested, true, memory_order_relaxed);
    NetEvent e = {.type = NET_EVT_SLEEP};
    net_manager_post(&e);
}

void net_manager_init(void) {
    net_event_queue = xQueueCreate(NET_QUEUE_LEN, sizeof(NetEvent));
    poll_timer = xTimerCreate("net_poll", pdMS_TO_TICKS(POLL_PERIOD_MS), pdTRUE, NULL, poll_timer_cb);
    PANIC_IF(net_event_queue == NULL || poll_timer == NULL, "net: init alloc failed");

    net_manager_view_init(&net_manager_view, gui_get_view_manager());

    PANIC_IF(task_create(&NET_MANAGER_TASK_CONFIG, net_manager_task, NULL) == NULL, "netmgr task create failed");

    sleep_manager_register(net_manager_shutdown_cb, NULL);
}

bool net_manager_post(const NetEvent* evt) {
    ABORT_IF(evt == NULL); /* debug builds trap; release falls through */
    if (evt == NULL) return false;
    return xQueueSend(net_event_queue, evt, 0) == pdTRUE;
}

bool net_manager_start(void) {
    NetEvent e = {.type = NET_EVT_START};
    return net_manager_post(&e);
}

bool net_manager_stop(void) {
    /* Break an in-flight do_connect() out of its loop; if the task isn't in
       a connect, the posted event drives the teardown instead. */
    atomic_store_explicit(&stop_requested, true, memory_order_relaxed);
    NetEvent e = {.type = NET_EVT_STOP};
    return net_manager_post(&e);
}

bool net_manager_submit_credentials(const char* ssid, const char* pass) {
    if (ssid == NULL || strlen(ssid) > NET_SSID_MAX) return false;

    NetEvent e = {.type = NET_EVT_CREDENTIALS}; /* zero-init guarantees NUL termination */
    strncpy(e.creds.ssid, ssid, NET_SSID_MAX);
    strncpy(e.creds.pass, pass ? pass : "", NET_PASS_MAX);
    return net_manager_post(&e);
}

bool net_manager_forget(void) {
    NetEvent e = {.type = NET_EVT_FORGET};
    return net_manager_post(&e);
}

NetState net_manager_get_state(void) { return state; }

bool net_manager_get_ip_str(char* out, size_t out_len) {
    if (out == NULL || out_len == 0) return false;
    if (state != NET_STATE_CONNECTED) return false;

    radio_lwip_lock();
    ip4_addr_t ip = *netif_ip4_addr(&cyw43_state.netif[CYW43_ITF_STA]);
    radio_lwip_unlock();

    /* Reentrant variant: no static buffer, no lock required. */
    ip4addr_ntoa_r(&ip, out, (int)out_len);
    return true;
}

const char* net_manager_get_status_str(NetState status) { ENUM_TO_STR_SWITCH(status, NET_STATUS_LIST) }