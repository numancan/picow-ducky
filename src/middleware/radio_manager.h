#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RADIO_IP_STR_SIZE 16 /* "255.255.255.255" + NUL */

typedef struct RadioScanResult RadioScanResult;

typedef enum {
    RADIO_USER_WIFI = 1u << 0,
    RADIO_USER_BLE = 1u << 1,
    RADIO_USER_KEEPALIVE = 1u << 2,
} RadioUser;

/* Runs in the cyw43 driver context: no blocking, keep it short.
 * r == NULL marks the end of a scan round (sentinel). */
typedef void (*RadioScanCallback)(void* env, const RadioScanResult* r);

struct RadioScanResult {
    const uint8_t* ssid;
    uint8_t ssid_len;
    int16_t rssi;
    bool is_locked; /* encrypted network (auth_mode != 0) */
};

void radio_manager_init(void);

int radio_acquire(RadioUser user);

/* Releases user's claim on the radio. If this drops the last user, tears
 * down the radio; if a scan is active at that point, this call blocks for
 * up to ~2s (radio_scan_abort waiting for the hardware scan to end) while
 * holding the module lock, so radio_acquire/radio_release from other
 * tasks may stall for that long. */
void radio_release(RadioUser user);

bool radio_is_up(void);

uint32_t radio_active_users(void);

void radio_ap_start(const char* ssid, const char* password);

void radio_ap_stop(void);

int radio_sta_connect(const char* ssid, const char* password, uint32_t timeout_ms, char* ip_out, size_t ip_out_len);

/* Starts connecting, does not wait. Track status via radio_sta_link_status. */
int radio_sta_connect_async(const char* ssid, const char* password);

void radio_sta_disconnect(void);

void radio_scan_abort(void);

/* Starts periodic scanning: one hardware scan every interval_ms,
 * cb(env, NULL) is called when a round finishes. Single-consumer
 * constraint: returns PICO_ERROR_RESOURCE_IN_USE if already active.
 * Cannot be called from an ISR. */
int radio_scan_start(RadioScanCallback cb, void* env, uint32_t interval_ms);

/* Stops periodic scanning. An in-flight scan round is not interrupted:
 * it completes and the final sentinel is published. No-op if already stopped. */
void radio_scan_stop(void);

/* Is periodic scanning enabled? */
bool radio_scan_enabled(void);

/* Is a hardware scan still in progress? */
bool radio_scan_active(void);

/* lwIP core lock wrappers (cyw43_arch_lwip_begin/end).
 * For non-radio lwIP app calls (e.g. httpd_init). */
void radio_lwip_lock(void);
void radio_lwip_unlock(void);
