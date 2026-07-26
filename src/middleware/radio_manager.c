#include "radio_manager.h"

#include <cyw43.h>
#include <cyw43_configport.h>
#include <pico/error.h>
#include <stdatomic.h>
#include <string.h>

#include "FreeRTOS.h"
#include "config/task_config.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "middleware/log.h"
#include "middleware/sys_fault.h"
#include "pico/async_context_freertos.h"
#include "pico/cyw43_arch.h"
#include "pico/mutex.h"
#include "pico/time.h"
#include "sleep_manager.h"
#include "task.h"

#define TAG "RADIO"

#define CYW43_STA_IS_ACTIVE(self) (((self)->itf_state >> CYW43_ITF_STA) & 1)
#define CYW43_AP_IS_ACTIVE(self) (((self)->itf_state >> CYW43_ITF_AP) & 1)

auto_init_mutex(radio_state_lock);

static uint32_t user_mask = 0; /* guarded by radio_state_lock */

/* _Atomic solely for the lock-free radio_is_up reader; writers hold
 * radio_state_lock. Plain load/store forms only: RMW ops (++/+=) lower to
 * library calls on M0+. */
static _Atomic int refcount = 0;

static const char* radio_user_str(RadioUser user) {
    switch (user) {
        case RADIO_USER_WIFI: return "WIFI";
        case RADIO_USER_BLE: return "BLE";
        case RADIO_USER_KEEPALIVE: return "KEEPALIVE";
        default: return "NONE";
    }
}

// Runs last in the sleep sequence: drop the keepalive claim so the radio can
// be torn down before the device sleeps. No-op if nothing holds it.
static void radio_manager_sleep_finalizer(void* context) {
    (void)context;
    radio_release(RADIO_USER_KEEPALIVE);
}

void radio_manager_init(void) { sleep_manager_register_finalizer(radio_manager_sleep_finalizer, NULL); }

int radio_acquire(RadioUser user) {
    ABORT_IF(user == 0);
    int result = 0;
    mutex_enter_blocking(&radio_state_lock);

    if ((user_mask & (uint32_t)user) == 0) {
        if (refcount == 0) {
            // TODO: remove once on SDK >= 2.3.1 (pico-sdk#3083)
            UBaseType_t old_affinity = vTaskCoreAffinityGet(NULL);
            vTaskCoreAffinitySet(NULL, RADIO_MANAGER_TASK_CONFIG.core_affinity);
            result = cyw43_arch_init();
            vTaskCoreAffinitySet(NULL, old_affinity);
            LOG_INFO(TAG, "radio cyw43_arch_init: %d", result);
        }
        if (result == 0) {
            user_mask |= (uint32_t)user;
            refcount = refcount + 1;
            LOG_INFO(TAG, "radio_acquired: %s", radio_user_str(user));
        }
        // On failure refcount stays 0; caller may retry.
    }

    mutex_exit(&radio_state_lock);
    return result;
}

void radio_release(RadioUser user) {
    ABORT_IF(user == 0);
    mutex_enter_blocking(&radio_state_lock);

    if (user_mask & (uint32_t)user) {
        user_mask &= ~(uint32_t)user;
        refcount = refcount - 1;
        if (refcount == 0) {
            radio_scan_abort();
            cyw43_arch_deinit();
            LOG_INFO(TAG, "radio_deinit!");
        }
    }

    mutex_exit(&radio_state_lock);

    LOG_INFO(TAG, "radio_released: %s", radio_user_str(user));
}

bool radio_is_up(void) { return refcount > 0; }

void radio_ap_start(const char* ssid, const char* password) {
    ABORT_IF(refcount == 0);

    if (CYW43_AP_IS_ACTIVE(&cyw43_state)) {
        return;
    }

    cyw43_arch_enable_ap_mode(ssid, password, CYW43_AUTH_WPA2_AES_PSK);
    LOG_INFO(TAG, "radio_ap_started");
}

void radio_ap_stop(void) {
    ABORT_IF(refcount == 0);
    cyw43_arch_disable_ap_mode();
    LOG_INFO(TAG, "radio_ap_stopped");
}

/* Delay after the STA mode switch; empirically avoids early connect failures. */
#define RADIO_STA_SETTLE_MS 500

// With both interfaces up, join traffic must exit via STA -> make STA the default.
static void radio_sta_mode_enter(void) {
    if (CYW43_STA_IS_ACTIVE(&cyw43_state)) return;

    cyw43_arch_enable_sta_mode();
    cyw43_delay_ms(RADIO_STA_SETTLE_MS);

    radio_lwip_lock();
    netif_set_default(&cyw43_state.netif[CYW43_ITF_STA]);
    radio_lwip_unlock();
}

// TODO: for now we only support WPA2_AES_PSK, but this function is here for future support of other auth modes
static uint32_t radio_get_auth_mode(const char* password) {
    return (password && password[0]) ? CYW43_AUTH_WPA2_AES_PSK : CYW43_AUTH_OPEN;
}

int radio_sta_connect(const char* ssid, const char* password, uint32_t timeout_ms, char* ip_out, size_t ip_out_len) {
    ABORT_IF(refcount == 0);
    radio_sta_mode_enter();
    uint32_t auth = radio_get_auth_mode(password);
    LOG_INFO(TAG, "radio_sta_connecting...");

    int rc = cyw43_arch_wifi_connect_timeout_ms(ssid, password, auth, timeout_ms);

    if (rc == PICO_OK && ip_out != NULL && ip_out_len > 0) {
        radio_lwip_lock();
        ip4_addr_t ip = *netif_ip4_addr(&cyw43_state.netif[CYW43_ITF_STA]);
        radio_lwip_unlock();
        // Reentrant variant: no static buffer, no lock required.
        ip4addr_ntoa_r(&ip, ip_out, (int)ip_out_len);
    }
    return rc;
}

int radio_sta_connect_async(const char* ssid, const char* password) {
    ABORT_IF(refcount == 0);
    radio_sta_mode_enter();
    uint32_t auth = radio_get_auth_mode(password);
    LOG_INFO(TAG, "radio_sta_connect_async");
    return cyw43_arch_wifi_connect_async(ssid, password, auth);
}

void radio_sta_disconnect(void) {
    ABORT_IF(refcount == 0);

    if (!CYW43_STA_IS_ACTIVE(&cyw43_state)) return;

    cyw43_arch_disable_sta_mode();

    radio_lwip_lock();
    netif_set_default(NULL);
    radio_lwip_unlock();

    LOG_INFO(TAG, "radio_sta_disconnect");
}

typedef enum {
    SCAN_IDLE = 0,
    SCAN_RUNNING,
    SCAN_STOPPING,
} ScanState;

/* Short poll interval to catch when an in-flight scan finishes. */
#define RADIO_SCAN_POLL_MS 200
#define RADIO_SCAN_ABORT_TIMEOUT_MS 2000

/* Scan session state machine, driven by single-attempt CAS transitions:
 * IDLE -> RUNNING (start reserves; the only way out of IDLE, so exactly one
 * caller wins), RUNNING -> STOPPING (stop/abort), STOPPING -> IDLE (worker
 * retires, or abort after a successful unlink). No lock is possible here:
 * the worker runs in cyw43's async context while radio_release holds
 * radio_state_lock waiting for that very worker to retire -- a worker
 * blocking on the same lock would deadlock. */
static _Atomic ScanState scan_state = SCAN_IDLE;
static _Atomic RadioScanCallback scan_cb = NULL;

/* static: address must remain valid for the worker's lifetime. */
static struct {
    void* env;
    uint32_t interval_ms;
} scan_ctx;

static async_at_time_worker_t scan_worker;
/* Worker-context only, plus start/abort where the worker provably cannot
 * run (before enqueue; after a successful unlink). */
static _Atomic bool is_round_open = false;

/* Runs in the cyw43 driver context; converts the result to the neutral type and forwards it. */
static int radio_scan_trampoline(void* env, const cyw43_ev_scan_result_t* result) {
    RadioScanCallback cb = scan_cb; /* abort revokes concurrently */
    if (result == NULL || cb == NULL) {
        return 0;
    }
    RadioScanResult r = {
        .ssid = result->ssid,
        .ssid_len = result->ssid_len,
        .rssi = (int16_t)result->rssi,
        .is_locked = (result->auth_mode != 0),
    };
    cb(env, &r);
    return 0;
}

/* Worker retire path: the async context has already unlinked us; publish
 * SCAN_IDLE last so a new radio_scan_start can only win the CAS after all
 * session teardown in this function is complete. */
static void radio_scan_worker_retire(void) {
    is_round_open = false;
    scan_state = SCAN_IDLE;
}

/* Runs in cyw43's async context: cyw43_wifi_* can be called directly. */
static void radio_scan_worker_fn(async_context_t* context, async_at_time_worker_t* worker) {
    /* Round still in flight in hardware: just poll again. */
    if (cyw43_wifi_scan_active(&cyw43_state)) {
        async_context_add_at_time_worker_in_ms(context, worker, RADIO_SCAN_POLL_MS);
        return;
    }

    /* A round just completed: close it and deliver the sentinel. */
    if (is_round_open) {
        is_round_open = false;
        RadioScanCallback cb = scan_cb; /* abort may revoke it concurrently */
        if (cb != NULL) {
            cb(scan_ctx.env, NULL); /* end-of-round sentinel */
        }
        if (scan_state == SCAN_RUNNING) {
            async_context_add_at_time_worker_in_ms(context, worker, scan_ctx.interval_ms);
            return;
        }
    }

    if (scan_state != SCAN_RUNNING) {
        radio_scan_worker_retire();
        return;
    }

    /* Start the next round. */
    cyw43_wifi_scan_options_t opts = {0};
    int err = cyw43_wifi_scan(&cyw43_state, &opts, scan_ctx.env, radio_scan_trampoline);
    if (err == 0) {
        is_round_open = true;
        async_context_add_at_time_worker_in_ms(context, worker, RADIO_SCAN_POLL_MS);
        return;
    }

    /* Skip this round; retry next period unless a stop landed mid-flight. */
    LOG_ERROR(TAG, "scan start failed: %d", err);
    if (scan_state == SCAN_RUNNING) {
        async_context_add_at_time_worker_in_ms(context, worker, scan_ctx.interval_ms);
    } else {
        radio_scan_worker_retire();
    }
}

int radio_scan_start(RadioScanCallback cb, void* env, uint32_t interval_ms) {
    ABORT_IF(cb == NULL);
    ABORT_IF(refcount == 0);

    /* Atomically reserve the single scan session. This is the only
     * transition out of SCAN_IDLE, so exactly one caller can win. */
    ScanState expected = SCAN_IDLE;
    if (!atomic_compare_exchange_strong(&scan_state, &expected, SCAN_RUNNING)) {
        // Single-consumer constraint: a session is active or still draining.
        return PICO_ERROR_RESOURCE_IN_USE;
    }

    /* Session is reserved and no worker is queued: safe to write plain
     * context. The enqueue below publishes it. */
    scan_ctx.env = env;
    scan_ctx.interval_ms = interval_ms;
    is_round_open = false;
    scan_cb = cb;

    scan_worker.do_work = radio_scan_worker_fn;
    async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &scan_worker, 0);

    LOG_INFO(TAG, "radio_scan_started");
    return PICO_OK;
}

void radio_scan_stop(void) {
    /* Graceful: the worker finishes the in-flight round, publishes the final
     * sentinel, then retires itself; no removal needed here. */
    ScanState expected = SCAN_RUNNING;
    if (atomic_compare_exchange_strong(&scan_state, &expected, SCAN_STOPPING)) {
        LOG_INFO(TAG, "radio_scan_stop requested");
    }
}

void radio_scan_abort(void) {
    /* Silence deliveries first so no callback fires after abort returns. */
    scan_cb = NULL;

    ScanState expected = SCAN_RUNNING;
    atomic_compare_exchange_strong(&scan_state, &expected, SCAN_STOPPING);

    if (scan_state == SCAN_STOPPING && async_context_remove_at_time_worker(cyw43_arch_async_context(), &scan_worker)) {
        /* The queued worker was unlinked and will never run again: we own
         * the retire path. */
        is_round_open = false;
        scan_state = SCAN_IDLE;
    }
    /* Else the worker is currently executing (or already idle); it will
     * observe SCAN_STOPPING and retire itself. */

    /* Wait for the hardware scan to drain and the worker to retire so the
     * caller (typically radio_release before deinit) sees a quiet radio. */
    absolute_time_t deadline = make_timeout_time_ms(RADIO_SCAN_ABORT_TIMEOUT_MS);
    while ((cyw43_wifi_scan_active(&cyw43_state) || scan_state != SCAN_IDLE) && !time_reached(deadline)) {
        cyw43_delay_ms(10);
    }
}

bool radio_scan_enabled(void) { return scan_state == SCAN_RUNNING; }
bool radio_scan_active(void) { return cyw43_wifi_scan_active(&cyw43_state); }

void radio_lwip_lock(void) { cyw43_arch_lwip_begin(); }
void radio_lwip_unlock(void) { cyw43_arch_lwip_end(); }