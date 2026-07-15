// hid_usb.c
//
// USB (TinyUSB) HID transport backend. Implements the HidTransport contract by
// pushing reports through tud_hid_* and blocking until TinyUSB signals the
// transfer completed.

#include "hid_usb.h"

#include "FreeRTOS.h"
#include "app/task_manager/task_manager.h"
#include "device/usbd.h"
#include "hid_report.h"
#include "hid_transport.h"
#include "middleware/log.h"
#include "semphr.h"
#include "sys_fault.h"
#include "task.h"
#include "tusb.h"
#include "usb_device.h"

static const char* TAG = "HID_USB";

typedef struct {
    SemaphoreHandle_t complete_semaphore;
    const HidTransport transport;
} UsbState;

static bool hid_usb_start(void);
static void hid_usb_stop(void);
static HidStatus hid_usb_get_status(void);
static HidStatus hid_usb_send(const HIDReport* report);

static UsbState usb_state = {.complete_semaphore = NULL,
                             .transport = {
                                 .name = "USB",
                                 .start = hid_usb_start,
                                 .stop = hid_usb_stop,
                                 .get_status = hid_usb_get_status,
                                 .send_report = hid_usb_send,
                             }};

static bool hid_usb_start(void) {
    if (usb_state.complete_semaphore == NULL) {
        usb_state.complete_semaphore = xSemaphoreCreateBinary();
        PANIC_IF(usb_state.complete_semaphore == NULL, "USB sent semaphore alloc failed");
    }

    return task_manager_start(TASK_MANAGER_TASK_USB_DEVICE);
}

static void hid_usb_stop(void) {
    if (usb_state.complete_semaphore) {
        vSemaphoreDelete(usb_state.complete_semaphore);
        usb_state.complete_semaphore = NULL;
    }

    task_manager_stop(TASK_MANAGER_TASK_USB_DEVICE);
}

const HidTransport* hid_usb_get_transport(void) { return &usb_state.transport; }

static HidStatus hid_usb_get_status(void) {
    if (!tud_inited() || !task_manager_is_created(TASK_MANAGER_TASK_USB_DEVICE)) {
        return HID_STATUS_UNINITIALIZED;
    }
    if (!tud_mounted()) {
        return HID_STATUS_DISCONNECTED;
    }
    if (!tud_hid_ready() || !tud_ready()) {
        return HID_STATUS_BUSY;
    }
    return HID_STATUS_OK;
}

static HidStatus hid_usb_send(const HIDReport* report) {
    HidStatus status = hid_usb_get_status();
    if (status != HID_STATUS_OK) return status;

    xSemaphoreTake(usb_state.complete_semaphore, 0);

    bool tx_success = false;
    switch (report->kind) {
        case REPORT_ID_KEYBOARD:
            tx_success = tud_hid_keyboard_report(REPORT_ID_KEYBOARD, report->keyboard.modifier,
                                                 (uint8_t*)report->keyboard.keycodes);
            break;

        case REPORT_ID_MOUSE:
            tx_success = tud_hid_mouse_report(REPORT_ID_MOUSE, report->mouse.buttons, report->mouse.x, report->mouse.y,
                                              report->mouse.wheel, 0);
            break;

        case REPORT_ID_CONSUMER_CONTROL:
            tx_success =
                tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &report->consumer.usage, sizeof(report->consumer.usage));
            break;

        default: return HID_STATUS_UNKNOWN;
    }

    // Maybe we need to add new status about tx_success status
    if (!tx_success) return HID_STATUS_BUSY;

    if (xSemaphoreTake(usb_state.complete_semaphore, pdMS_TO_TICKS(100)) == pdFAIL) {
        LOG_WARN(TAG, "USB Host timeout");
        return HID_STATUS_TIMEOUT;
    }

    return HID_STATUS_OK;
}
//--------------------------------------------------------------------+
// TinyUSB HID callbacks
//--------------------------------------------------------------------+

// Invoked when sent REPORT successfully to host
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void)instance;
    (void)report;
    (void)len;

    xSemaphoreGive(usb_state.complete_semaphore);
}

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer,
                               uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer,
                           uint16_t bufsize) {
    (void)instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD) {
            // bufsize should be (at least) 1
            if (bufsize < 1) return;

            uint8_t const kbd_leds = buffer[0];
            LOG_INFO(TAG, "Keyboard LEDs: %s %s %s", (kbd_leds & KEYBOARD_LED_CAPSLOCK) ? "CapsLock" : "",
                     (kbd_leds & KEYBOARD_LED_NUMLOCK) ? "NumLock" : "",
                     (kbd_leds & KEYBOARD_LED_SCROLLLOCK) ? "ScrollLock" : "");
        }
    }
}
