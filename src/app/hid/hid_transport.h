#pragma once

#include <stdbool.h>

#include "hid_report.h"
#include "middleware/enum_gen.h"

typedef struct HidTransport HidTransport;
struct HidTransport {
    const char* name;
    bool (*start)(void);
    void (*stop)(void);
    HidStatus (*get_status)(void);
    HidStatus (*send_report)(const HIDReport* report);
};

// $EXPORT=ID,NAME
#define HID_TRANSPORT_LIST(X)                          \
    X(HID_TRANSPORT_USB, "USB", hid_usb_get_transport) \
    X(HID_TRANSPORT_BLE, "BLE", hid_ble_get_transport)

DECLARE_ENUM(HidTransportId, HID_TRANSPORT_COUNT, HID_TRANSPORT_LIST)

// Switch the active transport, starting the new one and rolling back to the old
// one if startup fails. Returns true if the new transport is now active.
// new_transport_id must be a valid HidTransportId (caller-guaranteed); aborts
// otherwise.
bool hid_transport_switch(uint8_t new_transport_id);

void hid_transport_stop(void);

// Send a report through the active transport (mutex-protected). Aborts if no
// transport is active.
HidStatus hid_transport_send(const HIDReport* report);

// Current availability of the active transport. Returns HID_STATUS_UNINITIALIZED
// if no transport has been selected yet.
HidStatus hid_transport_status(void);

bool hid_transport_is_active_transport(HidTransportId transport_id);

const char* hid_transport_name(uint8_t index);