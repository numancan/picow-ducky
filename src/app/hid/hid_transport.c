#include "hid_transport.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "hid_ble.h"
#include "hid_usb.h"
#include "middleware/log.h"
#include "middleware/sys_fault.h"
#include "pico/mutex.h"
#include "task.h"

static const char* TAG = "HID";

// BUSY is a transient flow-control state (USB endpoint not yet polled, tx queue
// full). Absorb it here with a bounded retry so the encoder layer only ever sees
// OK or a terminal status, instead of silently dropping the report.
#define HID_BUSY_RETRY_TIMEOUT_MS 100
#define HID_BUSY_RETRY_DELAY_MS 1

auto_init_mutex(hid_transport_mutex);

static const HidTransport* active_transport;

static const HidTransport* transport_for_index(uint8_t index) {
    switch (index) {
#define X(id, name, func) \
    case id: return func();
        HID_TRANSPORT_LIST(X)
#undef X
        default: return NULL;
    }
}

HidStatus hid_transport_send(const HIDReport* r) {
    ABORT_IF(!r);

    if (active_transport == NULL) {
        LOG_ERROR(TAG, "send: no active transport");
        return HID_STATUS_UNINITIALIZED;
    }

    HidStatus status;
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(HID_BUSY_RETRY_TIMEOUT_MS);
    for (;;) {
        mutex_enter_blocking(&hid_transport_mutex);
        status = active_transport->send_report(r);
        mutex_exit(&hid_transport_mutex);

        if (status != HID_STATUS_BUSY) break;        // OK or terminal status -> done
        if (xTaskGetTickCount() >= deadline) break;  // budget exhausted -> return BUSY
        vTaskDelay(pdMS_TO_TICKS(HID_BUSY_RETRY_DELAY_MS));
    }

    // CRITICAL: This debug message causes a 5-10ms delay for each report,
    // because these logs are sent through USB-CDC.
#ifdef DEBUG
    uint32_t now = xTaskGetTickCount();  // ms

    const char* status_msg = status != HID_STATUS_OK ? "ERROR" : "OK";

    switch (r->kind) {
        case REPORT_ID_KEYBOARD:
            LOG_DEBUG(TAG, "[%s] [%lu] [%s] KB m=0x%02X kc=%02X %02X %02X %02X %02X %02X", active_transport->name, now,
                      status_msg, r->keyboard.modifier, r->keyboard.keycodes[0], r->keyboard.keycodes[1],
                      r->keyboard.keycodes[2], r->keyboard.keycodes[3], r->keyboard.keycodes[4],
                      r->keyboard.keycodes[5]);
            break;

        case REPORT_ID_MOUSE:
            LOG_DEBUG(TAG, "[%s] [%lu] [%s] MOUSE btn=0x%02X x=%d y=%d wheel=%d", active_transport->name, now,
                      status_msg, r->mouse.buttons, r->mouse.x, r->mouse.y, r->mouse.wheel);
            break;

        case REPORT_ID_CONSUMER_CONTROL:
            LOG_DEBUG(TAG, "[%s] [%lu] [%s] CONSUMER 0x%04X", active_transport->name, now, status_msg,
                      r->consumer.usage);
            break;

        default: break;
    }
#endif

    return status;
}

HidStatus hid_transport_status(void) {
    if (active_transport == NULL) return HID_STATUS_UNINITIALIZED;
    return active_transport->get_status();
}

const char* hid_transport_name(uint8_t index) {
    const HidTransport* t = transport_for_index(index);
    return t ? t->name : NULL;
}

bool hid_transport_switch(uint8_t new_transport_id) {
    const HidTransport* new_transport = transport_for_index(new_transport_id);
    ABORT_IF(!new_transport);

    mutex_enter_blocking(&hid_transport_mutex);

    if (active_transport == new_transport) {
        mutex_exit(&hid_transport_mutex);
        return true;
    }

    const HidTransport* old_transport = active_transport;

    if (old_transport != NULL) {
        LOG_INFO(TAG, "Stopping: %s", old_transport->name);
        old_transport->stop();
    }

    active_transport = NULL;  // no active transport during the switch

    LOG_INFO(TAG, "Starting: %s", new_transport->name);
    bool result;

    if (new_transport->start()) {
        active_transport = new_transport;
        LOG_INFO(TAG, "Transport switch successful.");
        result = true;
    } else {
        LOG_ERROR(TAG, "Switch failed: %s", new_transport->name);
        if (old_transport != NULL && old_transport->start()) {
            active_transport = old_transport;
            LOG_WARN(TAG, "Recovered previous transport: %s", old_transport->name);
        } else {
            active_transport = NULL;
            LOG_ERROR(TAG, "Rollback also failed, no active transport");
        }
        result = false;
    }

    mutex_exit(&hid_transport_mutex);
    return result;
}

bool hid_transport_is_active_transport(HidTransportId transport_id) {
    const HidTransport* t = transport_for_index(transport_id);
    return active_transport == t;
}

void hid_transport_stop(void) {
    mutex_enter_blocking(&hid_transport_mutex);
    if (active_transport != NULL) {
        LOG_INFO(TAG, "Stopping: %s", active_transport->name);
        active_transport->stop();
        active_transport = NULL;
    }
    mutex_exit(&hid_transport_mutex);
}
