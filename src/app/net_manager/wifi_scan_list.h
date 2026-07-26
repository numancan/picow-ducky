#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define WIFI_SCAN_MAX_ENTRIES 10
#define WIFI_SCAN_SSID_MAX_LEN 32  // 802.11 max SSID length

/* Number of consecutive missed scans before a network is dropped from the
 * list. 2 => a network is kept with its last RSSI for 2 scans, dropped on
 * the 3rd. */
#define WIFI_SCAN_MAX_MISSED 2

typedef struct {
    char ssid[WIFI_SCAN_SSID_MAX_LEN + 1];  // null-terminated
    int16_t rssi;                           // dBm
    bool locked;                            // true = secured (WPA/WEP)
    uint8_t missed;                         // 0 = seen in the last scan,
                                            // N = missing for N consecutive scans
} WifiScanEntry;

/* Initializes the module (creates the mutex). Call before wifi_scan_start. */
bool wifi_scan_list_init(void);

/* Starts periodic scanning (5 s); delegates to radio_manager's async worker,
 * the first scan fires immediately. Result processing runs in the cyw43
 * driver context, no task needed. No-op if already running.
 * Callable from any context except ISR. Radio must already be up. */
void wifi_scan_start(void);

/* Stops periodic scanning. The scan currently in progress is not interrupted;
 * it completes and its result is published. The published list is not
 * cleared, readers keep seeing the last result. No-op if already stopped. */
void wifi_scan_stop(void);

/* Is periodic scanning currently active? */
bool wifi_scan_is_running(void);

/* Copies the last published result. Returns the number of entries. */
size_t wifi_scan_list_get(WifiScanEntry* out, size_t max_out);

/* Has at least one scan completed and been published? */
bool wifi_scan_list_ready(void);

/* Aborts an in-progress scan immediately (e.g. to free the radio for another
 * operation). */
void wifi_scan_abort(void);
