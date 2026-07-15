#pragma once

#include "hid_transport.h"

// BLE HID-over-GATT transport. start() inits cyw43/BTstack and advertises
// "HID Combo"; stop() powers the radio down and deinits cyw43.
const HidTransport* hid_ble_get_transport(void);
