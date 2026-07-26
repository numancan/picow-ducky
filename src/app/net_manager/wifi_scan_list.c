#include "wifi_scan_list.h"

#include <string.h>

#include "FreeRTOS.h"
#include "middleware/sys_fault.h"
#include "pico/mutex.h"
#include "radio_manager.h"
#include "semphr.h"

#define SCAN_PERIOD_MS 5000

typedef struct {
    WifiScanEntry entries[WIFI_SCAN_MAX_ENTRIES];
    size_t count;
} ScanList;

static ScanList published;
static bool published_valid = false;
auto_init_mutex(wifi_scan_lock);

/* Working results of the scan in progress; accessed only from the cyw43
 * driver context, no mutex needed. */
static ScanList working_scan;

static bool ssid_equals(const WifiScanEntry* e, const char* ssid, size_t ssid_len) {
    return strncmp(e->ssid, ssid, ssid_len) == 0 && e->ssid[ssid_len] == '\0';
}

/* Inserts the entry into the list sorted by descending RSSI (max 10). If the
 * SSID already exists, updates it only on a stronger RSSI and resets missed
 * to 0 (dedup). */
static void list_insert(ScanList* list, const WifiScanEntry* in) {
    size_t ssid_len = strlen(in->ssid);
    if (ssid_len == 0) {
        return;  // hidden SSID
    }

    /* Dedup */
    for (size_t i = 0; i < list->count; i++) {
        WifiScanEntry* e = &list->entries[i];
        if (ssid_equals(e, in->ssid, ssid_len)) {
            if (in->rssi > e->rssi) {
                e->rssi = in->rssi;
                e->locked = in->locked;
                /* bubble-up */
                while (i > 0 && list->entries[i].rssi > list->entries[i - 1].rssi) {
                    WifiScanEntry tmp = list->entries[i - 1];
                    list->entries[i - 1] = list->entries[i];
                    list->entries[i] = tmp;
                    i--;
                }
            }
            if (in->missed < list->entries[i].missed) {
                list->entries[i].missed = in->missed;
            }
            return;
        }
    }

    /* Insertion position (descending RSSI) */
    size_t pos = list->count;
    for (size_t i = 0; i < list->count; i++) {
        if (in->rssi > list->entries[i].rssi) {
            pos = i;
            break;
        }
    }
    if (pos >= WIFI_SCAN_MAX_ENTRIES) {
        return;  // didn't make the top 10
    }

    size_t new_count = list->count < WIFI_SCAN_MAX_ENTRIES ? list->count + 1 : WIFI_SCAN_MAX_ENTRIES;
    for (size_t i = new_count - 1; i > pos; i--) {
        list->entries[i] = list->entries[i - 1];
    }
    list->entries[pos] = *in;
    list->count = new_count;
}

static bool list_contains(const ScanList* list, const char* ssid, size_t ssid_len) {
    for (size_t i = 0; i < list->count; i++) {
        if (ssid_equals(&list->entries[i], ssid, ssid_len)) {
            return true;
        }
    }
    return false;
}

/*
 * Aging + publishing. Takes the mutex itself.
 *
 * It's normal for an AP to miss a probe response in a single scan, so
 * networks absent from a fresh scan aren't dropped immediately:
 *  - Fresh results are taken as-is (missed = 0)
 *  - Networks present in the old list but absent from the fresh scan have
 *    their missed counter incremented; if it hasn't exceeded
 *    WIFI_SCAN_MAX_MISSED it's merged into the list with its old RSSI,
 *    otherwise it's dropped.
 * This prevents networks from flickering in and out; a network only really
 * disappears from the list after being missing for WIFI_SCAN_MAX_MISSED+1
 * consecutive scans.
 */
static void publish_merged(const ScanList* fresh) {
    ScanList merged = *fresh;

    mutex_enter_blocking(&wifi_scan_lock);

    for (size_t i = 0; i < published.count; i++) {
        WifiScanEntry old = published.entries[i];

        if (list_contains(fresh, old.ssid, strlen(old.ssid))) {
            continue;  // present in the fresh scan, already in merged
        }
        if (old.missed >= WIFI_SCAN_MAX_MISSED) {
            continue;  // missing too long, drop
        }
        old.missed++;
        list_insert(&merged, &old);
    }

    published = merged;
    published_valid = true;

    mutex_exit(&wifi_scan_lock);
}

/* Scan callback: runs in the cyw43 driver context. */
static void scan_result_cb(void* env, const RadioScanResult* result) {
    ScanList* working = (ScanList*)env;

    if (result == NULL) {
        /* End-of-round sentinel: merge, publish, and reset for the next round. */
        publish_merged(working);
        working->count = 0;
        return;
    }

    WifiScanEntry e;
    size_t len = result->ssid_len;
    if (len > WIFI_SCAN_SSID_MAX_LEN) {
        len = WIFI_SCAN_SSID_MAX_LEN;
    }
    memcpy(e.ssid, result->ssid, len);
    e.ssid[len] = '\0';
    e.rssi = result->rssi;
    e.locked = result->is_locked;
    e.missed = 0;

    list_insert(working, &e);
}

void wifi_scan_start(void) {
    /* If already active, the radio layer returns RESOURCE_IN_USE -> no-op by contract. */
    radio_scan_start(scan_result_cb, &working_scan, SCAN_PERIOD_MS);
}

void wifi_scan_stop(void) { radio_scan_stop(); }

void wifi_scan_abort(void) { radio_scan_abort(); }

bool wifi_scan_is_running(void) { return radio_scan_enabled(); }

size_t wifi_scan_list_get(WifiScanEntry* out, size_t max_out) {
    ABORT_IF(out == NULL || max_out == 0);
    size_t n = 0;

    mutex_enter_blocking(&wifi_scan_lock);
    n = published.count < max_out ? published.count : max_out;
    memcpy(out, published.entries, n * sizeof(WifiScanEntry));
    mutex_exit(&wifi_scan_lock);

    return n;
}