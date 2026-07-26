/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
 * GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@bluekitchen-gmbh.com
 *
 */

#include "radio_manager.h"
#define BTSTACK_FILE__ "hid_ble.c"

// *****************************************************************************
/* BLE HID-over-GATT transport: keyboard, mouse and consumer input reports.
 */
// *****************************************************************************

#include <inttypes.h>
#include <pico/cyw43_arch.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "ble/gatt-service/battery_service_server.h"
#include "ble/gatt-service/device_information_service_server.h"
#include "hid_ble.h"

// clang-format off
#define hid_report_type_t btstack_hid_report_type_t
#define HID_REPORT_TYPE_INPUT BTSTACK_HID_REPORT_TYPE_INPUT
#define HID_REPORT_TYPE_OUTPUT BTSTACK_HID_REPORT_TYPE_OUTPUT
#define HID_REPORT_TYPE_FEATURE BTSTACK_HID_REPORT_TYPE_FEATURE
#include "ble/gatt-service/hids_device.h"
#include "btstack.h"
#undef hid_report_type_t
#undef HID_REPORT_TYPE_INPUT
#undef HID_REPORT_TYPE_OUTPUT
#undef HID_REPORT_TYPE_FEATURE

// clang-format on

#include "btstack_event.h"
#include "hid_report.h"
#include "hog_combo.h"
#include "middleware/log.h"
#include "middleware/radio_manager.h"
#include "middleware/sys_fault.h"
#include "semphr.h"

static const char* TAG = "HID_BLE";

// Send blocks until the can-send-now event delivers the report; bounded so a
// disconnect mid-send cannot hang the caller forever.
#define BLE_SEND_TIMEOUT_MS 200

// Composite report map: keyboard (Report ID 1, boot compatible), relative mouse
// (Report ID 2) and consumer control (Report ID 3). Keyboard section is the
// USB HID Specification 1.1, Appendix B.1 boot keyboard.
const uint8_t hid_ble_descriptor_keyboard_mouse_consumer[] = {

    0x05,
    0x01,  // Usage Page (Generic Desktop)
    0x09,
    0x06,  // Usage (Keyboard)
    0xa1,
    0x01,  // Collection (Application)

    0x85,
    REPORT_ID_KEYBOARD,  // Report ID 1

    // Modifier byte

    0x75,
    0x01,  //   Report Size (1)
    0x95,
    0x08,  //   Report Count (8)
    0x05,
    0x07,  //   Usage Page (Key codes)
    0x19,
    0xe0,  //   Usage Minimum (Keyboard LeftControl)
    0x29,
    0xe7,  //   Usage Maxium (Keyboard Right GUI)
    0x15,
    0x00,  //   Logical Minimum (0)
    0x25,
    0x01,  //   Logical Maximum (1)
    0x81,
    0x02,  //   Input (Data, Variable, Absolute)

    // Reserved byte

    0x75,
    0x01,  //   Report Size (1)
    0x95,
    0x08,  //   Report Count (8)
    0x81,
    0x03,  //   Input (Constant, Variable, Absolute)

    // LED report + padding

    0x95,
    0x05,  //   Report Count (5)
    0x75,
    0x01,  //   Report Size (1)
    0x05,
    0x08,  //   Usage Page (LEDs)
    0x19,
    0x01,  //   Usage Minimum (Num Lock)
    0x29,
    0x05,  //   Usage Maxium (Kana)
    0x91,
    0x02,  //   Output (Data, Variable, Absolute)

    0x95,
    0x01,  //   Report Count (1)
    0x75,
    0x03,  //   Report Size (3)
    0x91,
    0x03,  //   Output (Constant, Variable, Absolute)

    // Keycodes

    0x95,
    0x06,  //   Report Count (6)
    0x75,
    0x08,  //   Report Size (8)
    0x15,
    0x00,  //   Logical Minimum (0)
    0x25,
    0xff,  //   Logical Maximum (255)
    0x05,
    0x07,  //   Usage Page (Key codes)
    0x19,
    0x00,  //   Usage Minimum (Reserved (no event indicated))
    0x29,
    0xff,  //   Usage Maxium (Reserved)
    0x81,
    0x00,  //   Input (Data, Array)

    0xc0,  // End collection

    // Mouse: buttons + relative X/Y/Wheel, payload {buttons, dx, dy, wheel}

    0x05,
    0x01,  // Usage Page (Generic Desktop)
    0x09,
    0x02,  // Usage (Mouse)
    0xa1,
    0x01,  // Collection (Application)

    0x85,
    REPORT_ID_MOUSE,  // Report ID 2
    0x09,
    0x01,  //   Usage (Pointer)
    0xa1,
    0x00,  //   Collection (Physical)

    0x05,
    0x09,  //     Usage Page (Buttons)
    0x19,
    0x01,  //     Usage Minimum (Button 1)
    0x29,
    0x03,  //     Usage Maximum (Button 3)
    0x15,
    0x00,  //     Logical Minimum (0)
    0x25,
    0x01,  //     Logical Maximum (1)
    0x95,
    0x03,  //     Report Count (3)
    0x75,
    0x01,  //     Report Size (1)
    0x81,
    0x02,  //     Input (Data, Variable, Absolute)

    0x95,
    0x01,  //     Report Count (1)
    0x75,
    0x05,  //     Report Size (5)
    0x81,
    0x03,  //     Input (Constant) -- padding

    0x05,
    0x01,  //     Usage Page (Generic Desktop)
    0x09,
    0x30,  //     Usage (X)
    0x09,
    0x31,  //     Usage (Y)
    0x09,
    0x38,  //     Usage (Wheel)
    0x15,
    0x81,  //     Logical Minimum (-127)
    0x25,
    0x7f,  //     Logical Maximum (127)
    0x75,
    0x08,  //     Report Size (8)
    0x95,
    0x03,  //     Report Count (3)
    0x81,
    0x06,  //     Input (Data, Variable, Relative)

    0xc0,  //   End collection (Physical)
    0xc0,  // End collection (Application)

    // Consumer control: single 16-bit usage selector, payload = usage (LE)

    0x05,
    0x0c,  // Usage Page (Consumer)
    0x09,
    0x01,  // Usage (Consumer Control)
    0xa1,
    0x01,  // Collection (Application)

    0x85,
    REPORT_ID_CONSUMER_CONTROL,  // Report ID 3
    0x15,
    0x00,  //   Logical Minimum (0)
    0x26,
    0xff,
    0x03,  //   Logical Maximum (0x3ff)
    0x19,
    0x00,  //   Usage Minimum (0)
    0x2a,
    0xff,
    0x03,  //   Usage Maximum (0x3ff)
    0x75,
    0x10,  //   Report Size (16)
    0x95,
    0x01,  //   Report Count (1)
    0x81,
    0x00,  //   Input (Data, Array)

    0xc0,  // End collection
};

static bool hid_ble_start(void);
static void hid_ble_stop(void);
static HidStatus hid_ble_get_status(void);
static HidStatus hid_ble_send_report(const HIDReport* report);
static void hid_ble_handle_packet(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size);

typedef struct {
    bool is_initialized;
    // Set for the duration of hid_ble_stop() so the disconnect handler knows
    // this is an intentional teardown and must not re-enable advertising.
    volatile bool stopping;
    uint8_t battery_level;
    volatile hci_con_handle_t connection_handle;
    volatile bool input_reports_enabled;
    // One report in flight at a time, flushed in CAN_SEND_NOW.
    uint8_t pending_id;
    uint8_t pending_data[8];
    uint8_t pending_length;
    volatile bool last_tx_success;
    // Blocking send: the caller task waits on sent_semaphore until CAN_SEND_NOW delivers.
    SemaphoreHandle_t sent_semaphore;
    btstack_context_callback_registration_t send_registration;
    // stop() marshals BTstack teardown onto the run loop and waits on teardown_semaphore.
    SemaphoreHandle_t teardown_semaphore;
    btstack_context_callback_registration_t teardown_registration;
    btstack_packet_callback_registration_t hci_callback;
    btstack_packet_callback_registration_t sm_callback;
    HidTransport transport;
} BleState;

static BleState ble_state = {
    .connection_handle = HCI_CON_HANDLE_INVALID,
    .battery_level = 100,
    .transport =
        {
            .name = "BLE",
            .start = hid_ble_start,
            .stop = hid_ble_stop,
            .get_status = hid_ble_get_status,
            .send_report = hid_ble_send_report,
        },
};

const uint8_t advertising_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02,
    BLUETOOTH_DATA_TYPE_FLAGS,
    0x06,
    // Name
    0x08,
    BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
    'p',
    'i',
    'c',
    'o',
    'z',
    'a',
    'p',
    // 16-bit Service UUIDs
    0x03,
    BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
    ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff,
    ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Keyboard (Category 15, Sub-Category 1)
    0x03,
    BLUETOOTH_DATA_TYPE_APPEARANCE,
    0xC1,
    0x03,
};
const uint8_t advertising_data_length = sizeof(advertising_data);

static void hid_ble_setup_le_keyboard(void) {
    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);

    att_server_init(profile_data, NULL, NULL);

    battery_service_server_init(ble_state.battery_level);
    device_information_service_server_init();

    // One slot per REPORT characteristic in the .gatt:
    // keyboard input, keyboard LED output, mouse input, consumer input.
    static hids_device_report_t hid_reports_storage[4];
    hids_device_init_with_storage(0, hid_ble_descriptor_keyboard_mouse_consumer,
                                  sizeof(hid_ble_descriptor_keyboard_mouse_consumer),
                                  (sizeof(hid_reports_storage) / sizeof(hid_reports_storage[0])), hid_reports_storage);

    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(advertising_data_length, (uint8_t*)advertising_data);
    gap_advertisements_enable(1);

    ble_state.hci_callback.callback = &hid_ble_handle_packet;
    hci_add_event_handler(&ble_state.hci_callback);

    ble_state.sm_callback.callback = &hid_ble_handle_packet;
    sm_add_event_handler(&ble_state.sm_callback);

    hids_device_register_packet_handler(hid_ble_handle_packet);
}

// Runs in BTstack run-loop context (marshaled). If the link dropped between
// queueing and now, release the waiting caller instead of arming a notify.
static void hid_ble_request_can_send_now(void* context) {
    UNUSED(context);
    if (ble_state.connection_handle == HCI_CON_HANDLE_INVALID || !ble_state.input_reports_enabled) {
        ble_state.last_tx_success = false;
        xSemaphoreGive(ble_state.sent_semaphore);
        return;
    }
    hids_device_request_can_send_now_event(ble_state.connection_handle);
}

static HidStatus hid_ble_get_status(void) {
    if (!ble_state.is_initialized || !radio_is_up() || hci_get_state() != HCI_STATE_WORKING) {
        return HID_STATUS_UNINITIALIZED;
    }
    if (ble_state.connection_handle == HCI_CON_HANDLE_INVALID) {
        return HID_STATUS_DISCONNECTED;
    }
    if (!ble_state.input_reports_enabled) {
        // connected but host hasn't enabled report notifications yet
        return HID_STATUS_BUSY;
    }
    return HID_STATUS_OK;
}

static HidStatus hid_ble_send_report(const HIDReport* report) {
    HidStatus status = hid_ble_get_status();
    if (status != HID_STATUS_OK) return status;

    xSemaphoreTake(ble_state.sent_semaphore, 0);
    ble_state.last_tx_success = false;

    switch (report->kind) {
        case REPORT_ID_KEYBOARD:
            ble_state.pending_id = REPORT_ID_KEYBOARD;
            ble_state.pending_length = 8;
            ble_state.pending_data[0] = report->keyboard.modifier;
            ble_state.pending_data[1] = 0;
            memcpy(&ble_state.pending_data[2], report->keyboard.keycodes, 6);
            break;
        case REPORT_ID_MOUSE:
            ble_state.pending_id = REPORT_ID_MOUSE;
            ble_state.pending_length = 4;
            ble_state.pending_data[0] = report->mouse.buttons;
            ble_state.pending_data[1] = (uint8_t)report->mouse.x;
            ble_state.pending_data[2] = (uint8_t)report->mouse.y;
            ble_state.pending_data[3] = (uint8_t)report->mouse.wheel;
            break;
        case REPORT_ID_CONSUMER_CONTROL:
            ble_state.pending_id = REPORT_ID_CONSUMER_CONTROL;
            ble_state.pending_length = 2;
            ble_state.pending_data[0] = (uint8_t)(report->consumer.usage & 0xff);
            ble_state.pending_data[1] = (uint8_t)(report->consumer.usage >> 8);
            break;
        default: return HID_STATUS_UNKNOWN;
    }

    ble_state.send_registration.callback = &hid_ble_request_can_send_now;
    btstack_run_loop_execute_on_main_thread(&ble_state.send_registration);

    if (xSemaphoreTake(ble_state.sent_semaphore, pdMS_TO_TICKS(BLE_SEND_TIMEOUT_MS)) == pdFAIL) {
        LOG_WARN(TAG, "BLE send timeout (CAN_SEND_NOW not received)");
        return HID_STATUS_TIMEOUT;
    }

    if (!ble_state.last_tx_success) {
        return HID_STATUS_BUSY;
    }

    return HID_STATUS_OK;
}

static bool hid_ble_start(void) {
    if (radio_acquire(RADIO_USER_KEEPALIVE) != 0) {
        LOG_ERROR(TAG, "radio acquire fail");
        return false;
    }
    ble_state.stopping = false;

    async_context_acquire_lock_blocking(cyw43_arch_async_context());

    if (!ble_state.is_initialized) {
        hid_ble_setup_le_keyboard();

        ble_state.sent_semaphore = xSemaphoreCreateBinary();
        PANIC_IF(ble_state.sent_semaphore == NULL, "BLE sent semaphore alloc failed");

        ble_state.teardown_semaphore = xSemaphoreCreateBinary();
        PANIC_IF(ble_state.teardown_semaphore == NULL, "BLE teardown semaphore alloc failed");
        ble_state.is_initialized = true;
    }

    hci_power_control(HCI_POWER_ON);  // Wake up BT -> wait for WORKING

    async_context_release_lock(cyw43_arch_async_context());

    return true;
}

static void hid_ble_stop_on_run_loop(void* context) {
    UNUSED(context);
    gap_advertisements_enable(0);
    if (ble_state.connection_handle != HCI_CON_HANDLE_INVALID) {
        gap_disconnect(ble_state.connection_handle);
    }
    if (hci_get_state() == HCI_STATE_OFF) {
        // Already off (double stop, or stop before power-on completed):
        // no BTSTACK_EVENT_STATE transition will fire, so release the
        // waiter directly instead of letting it burn the full timeout.
        xSemaphoreGive(ble_state.teardown_semaphore);
        return;
    }
    hci_power_control(HCI_POWER_OFF);
}

static void hid_ble_stop(void) {
    if (!ble_state.is_initialized) return;

    // Tell the disconnect handler not to re-enable advertising while we
    // tear down; the disconnect we may trigger below is intentional.
    ble_state.stopping = true;

    xSemaphoreTake(ble_state.teardown_semaphore, 0);
    ble_state.teardown_registration.callback = &hid_ble_stop_on_run_loop;
    btstack_run_loop_execute_on_main_thread(&ble_state.teardown_registration);
    if (xSemaphoreTake(ble_state.teardown_semaphore, pdMS_TO_TICKS(2000)) != pdTRUE) {
        LOG_WARN(TAG, "stop timeout");
    }

    ble_state.connection_handle = HCI_CON_HANDLE_INVALID;
    ble_state.input_reports_enabled = false;
    ble_state.stopping = false;

    // Releasing cyw43 in runtime is not safe because of ble stack.
    // radio_release(RADIO_USER_BLE);
}

static void hid_ble_handle_packet(uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
        case BTSTACK_EVENT_STATE: {
            uint8_t state = btstack_event_state_get_state(packet);
            LOG_DEBUG(TAG, "HCI state=%u", state);

            if (state == HCI_STATE_WORKING) {
                bd_addr_t local_addr;
                gap_local_bd_addr(local_addr);
                LOG_INFO(TAG, "BTstack up on %s", bd_addr_to_str(local_addr));

                gap_advertisements_enable(1);

            } else if (state == HCI_STATE_OFF) {
                xSemaphoreGive(ble_state.teardown_semaphore);
            }
            break;
        }
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            LOG_INFO(TAG, "Disconnected, reason=0x%02x", hci_event_disconnection_complete_get_reason(packet));
            ble_state.connection_handle = HCI_CON_HANDLE_INVALID;
            ble_state.input_reports_enabled = false;
            // NOTE: radio_release() removed from here -- radio ownership is
            // tied to start()/stop(), not to the link state.
            if (!ble_state.stopping) {
                // Host-initiated (or link-loss) disconnect: become
                // discoverable again so the host can reconnect.
                gap_advertisements_enable(1);
            }
            break;
        case SM_EVENT_JUST_WORKS_REQUEST:
            LOG_DEBUG(TAG, "Just Works requested");
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            break;
        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            LOG_DEBUG(TAG, "Numeric comparison: %" PRIu32, sm_event_numeric_comparison_request_get_passkey(packet));
            sm_numeric_comparison_confirm(sm_event_numeric_comparison_request_get_handle(packet));
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            LOG_INFO(TAG, "Display Passkey: %" PRIu32, sm_event_passkey_display_number_get_passkey(packet));
            break;
        case HCI_EVENT_HIDS_META:
            switch (hci_event_hids_meta_get_subevent_code(packet)) {
                case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
                    ble_state.connection_handle = hids_subevent_input_report_enable_get_con_handle(packet);
                    ble_state.input_reports_enabled = hids_subevent_input_report_enable_get_enable(packet) != 0;
                    LOG_INFO(TAG, "Report Characteristic Subscribed %u", ble_state.input_reports_enabled);
                    break;
                case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
                    ble_state.connection_handle =
                        hids_subevent_boot_keyboard_input_report_enable_get_con_handle(packet);
                    ble_state.input_reports_enabled =
                        hids_subevent_boot_keyboard_input_report_enable_get_enable(packet) != 0;
                    LOG_INFO(TAG, "Boot Keyboard Characteristic Subscribed %u", ble_state.input_reports_enabled);
                    break;
                case HIDS_SUBEVENT_PROTOCOL_MODE:
                    LOG_DEBUG(TAG, "Protocol Mode: %s",
                              hids_subevent_protocol_mode_get_protocol_mode(packet) ? "Report" : "Boot");
                    break;
                case HIDS_SUBEVENT_CAN_SEND_NOW: {
                    uint8_t btstack_status =
                        hids_device_send_input_report_for_id(ble_state.connection_handle, ble_state.pending_id,
                                                             ble_state.pending_data, ble_state.pending_length);
                    ble_state.last_tx_success = (btstack_status == 0);
                    xSemaphoreGive(ble_state.sent_semaphore);
                    break;
                }
                default: break;
            }
            break;

        default: break;
    }
}

const HidTransport* hid_ble_get_transport(void) { return &ble_state.transport; }