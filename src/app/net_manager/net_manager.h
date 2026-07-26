#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "middleware/enum_gen.h"

#define NET_SSID_MAX 32
#define NET_PASS_MAX 64
#define NET_IP_STR_SIZE 16 /* "255.255.255.255" + NUL */

// $EXPORT=ID,STATUS_NAME
#define NET_STATUS_LIST(X)                    \
    X(NET_STATE_IDLE, "Idle")                 \
    X(NET_STATE_STA_CONNECTING, "Connecting") \
    X(NET_STATE_CONNECTED, "Connected")       \
    X(NET_STATE_PROVISIONING, "Provisioning") \
    X(NET_STATE_PROV_CONNECTING, "Provisioning Connecting")

DECLARE_ENUM(NetState, NET_STATE_COUNT, NET_STATUS_LIST)

/* Connect result is not an event: net_task calls the blocking
 * radio_sta_connect() and its return code is the authoritative result.
 * Link-loss detection is done internally via POLL.
 *
 * Note: while a blocking connect is in progress (up to
 * CONNECT_MAX_ATTEMPTS * CONNECT_TIMEOUT_MS), posted events stay queued and
 * are handled in whatever state the connect attempt ends in. Events that
 * are not meaningful in that state are silently dropped. */
typedef enum {
    NET_EVT_START = 0,    /* external: acquire the radio and start work  */
    NET_EVT_STOP,         /* external: tear down and release the radio   */
    NET_EVT_SCAN_REQUEST, /* from web_server: user requested the list    */
    NET_EVT_CREDENTIALS,  /* from web_server: user entered SSID/pass     */
    NET_EVT_FORGET,       /* from web_server: forget the saved network   */
    NET_EVT_POLL,         /* internal: link-monitoring timer tick        */
    NET_EVT_SLEEP,        /* from sleep_manager: tear down and exit task  */
} NetEventType;

typedef struct {
    char ssid[NET_SSID_MAX + 1];
    char pass[NET_PASS_MAX + 1];
} NetCredentials;

typedef struct {
    NetEventType type;
    NetCredentials creds; /* valid only with NET_EVT_CREDENTIALS */
} NetEvent;

/* Creates the event queue, the poll timer and the net_manager task. Called before the scheduler starts. */
void net_manager_init(void);

/* Idles until an external NET_EVT_START arrives; does no radio work on its own. Created by net_manager_init. */
void net_manager_task(void* arg);

/* Acquire the radio and begin the boot/provisioning flow. No-op unless currently idle. */
bool net_manager_start(void);

/* Tear down networking and release the radio; returns to idle. */
bool net_manager_stop(void);

/* Post an event from any task context (never call from ISR). Returns false if the queue is full or evt is NULL. */
bool net_manager_post(const NetEvent* evt);

/* Validates and queues submitted Wi-Fi credentials for the portal flow. */
bool net_manager_submit_credentials(const char* ssid, const char* pass);

/* Requests erasing the saved network and returning to provisioning. */
bool net_manager_forget(void);

/* Returns the current network state; safe to read from any context. */
NetState net_manager_get_state(void);

/* Writes the STA IPv4 address ("a.b.c.d") into out. Returns false if not
 * connected or out is invalid. Safe to call from any task (never from ISR). */
bool net_manager_get_ip_str(char* out, size_t out_len);

/* Returns the human-readable label for a given NetState. */
const char* net_manager_get_status_str(NetState status);