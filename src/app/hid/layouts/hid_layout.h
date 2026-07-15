#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "middleware/enum_gen.h"

typedef struct {
    uint8_t modifier;
    uint8_t keycode;
} KeyEntry;

// $EXPORT=ID,NAME
#define HID_KEY_LAYOUT_LIST(X)                     \
    X(HID_KEY_LAYOUT_US_Q, "US-Q", ascii_table_us) \
    X(HID_KEY_LAYOUT_TR_Q, "TR-Q", ascii_table_tr)

DECLARE_ENUM(HIDLayoutID, HID_KEY_LAYOUT_COUNT, HID_KEY_LAYOUT_LIST)

void hid_layout_set(HIDLayoutID layout);
const char* hid_layout_name(HIDLayoutID layout);
const KeyEntry* hid_layout_get_active_table(void);
bool hid_layout_set_by_name(const char* name);