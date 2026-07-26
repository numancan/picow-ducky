#include "hid_report.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "hid_transport.h"
#include "middleware/sys_fault.h"
#include "semphr.h"
#include "task.h"

// Included layout systems
#include "layouts/hid_layout.h"

static const struct {
    const char* name;
    uint8_t keycode;
} special_conv_table[] = {HID_STRING_TO_SPECIAL_KEY};

static const struct {
    const char* name;
    uint16_t keycode;
} consumer_conv_table[] = {HID_STRING_TO_CONSUMER_KEY};

static const struct {
    const char* name;
    uint8_t modifier_bit;
} modifier_conv_table[] = {HID_STRING_TO_MODIFIER};

static const HIDReport KEYBOARD_RELEASE = {.kind = REPORT_ID_KEYBOARD};
static const HIDReport MOUSE_RELEASE = {.kind = REPORT_ID_MOUSE};
static const HIDReport CONSUMER_RELEASE = {.kind = REPORT_ID_CONSUMER_CONTROL};

HidStatus hid_report_keys(uint8_t modifier, const uint8_t keycodes[6]) {
    HIDReport press = {.kind = REPORT_ID_KEYBOARD, .keyboard = {.modifier = modifier}};

    memcpy(press.keyboard.keycodes, keycodes, 6);

    HidStatus status = hid_transport_send(&press);
    if (status != HID_STATUS_OK) return status;

    status = hid_transport_send(&KEYBOARD_RELEASE);
    return status;
}

HidStatus hid_report_char(char c, uint32_t char_delay) {
    uint8_t modifier = 0;
    uint8_t keycode = hid_report_char_to_keycode(c, &modifier);
    if (keycode == 0) return HID_STATUS_UNKNOWN;

    HIDReport press = {.kind = REPORT_ID_KEYBOARD,
                       .keyboard = {.modifier = modifier, .keycodes = {keycode, 0, 0, 0, 0, 0}}};

    HidStatus status = hid_transport_send(&press);
    if (status != HID_STATUS_OK) return status;

    status = hid_transport_send(&KEYBOARD_RELEASE);
    vTaskDelay(pdMS_TO_TICKS(char_delay));
    return status;
}

HidStatus hid_report_string(const char* str, uint32_t char_delay) {
    HidStatus status;
    while (*str) {
        status = hid_report_char(*str++, char_delay);
        if (status != HID_STATUS_OK) return status;
    }

    return status;
}

HidStatus hid_report_mouse_move(int8_t x, int8_t y) {
    HIDReport r = {.kind = REPORT_ID_MOUSE, .mouse = {.buttons = 0, .x = x, .y = y, .wheel = 0}};

    HidStatus status = hid_transport_send(&r);
    if (status != HID_STATUS_OK) return status;

    status = hid_transport_send(&MOUSE_RELEASE);
    return status;
}

HidStatus hid_report_mouse_click(uint8_t buttons) {
    HIDReport press = {.kind = REPORT_ID_MOUSE, .mouse = {.buttons = buttons, .x = 0, .y = 0, .wheel = 0}};

    HidStatus status = hid_transport_send(&press);
    if (status != HID_STATUS_OK) return status;

    status = hid_transport_send(&MOUSE_RELEASE);
    return status;
}

HidStatus hid_report_mouse_scroll(int8_t wheel) {
    HIDReport r = {.kind = REPORT_ID_MOUSE, .mouse = {.buttons = 0, .x = 0, .y = 0, .wheel = wheel}};

    return hid_transport_send(&r);
}

HidStatus hid_report_consumer(uint16_t usage) {
    HIDReport press = {.kind = REPORT_ID_CONSUMER_CONTROL, .consumer = {.usage = usage}};

    HidStatus status = hid_transport_send(&press);
    if (status != HID_STATUS_OK) return status;

    status = hid_transport_send(&CONSUMER_RELEASE);
    return status;
}

const char* hid_report_status_name(HidStatus status){ENUM_TO_STR_SWITCH(status, HID_STATUS_LIST)}

uint8_t hid_report_str_to_mod(const char* str) {
    for (size_t i = 0; i < MODIFIER_ARRAY_SIZE; i++) {
        if (strcmp(modifier_conv_table[i].name, str) == 0) {
            return modifier_conv_table[i].modifier_bit;
        }
    }
    return 0;
}

uint8_t hid_report_str_to_special(const char* str) {
    for (size_t i = 0; i < SPECIAL_KEY_ARRAY_SIZE; i++) {
        if (strcmp(special_conv_table[i].name, str) == 0) {
            return special_conv_table[i].keycode;
        }
    }
    return 0;
}

uint16_t hid_report_str_to_consumer(const char* str) {
    for (size_t i = 0; i < CONSUMER_KEY_ARRAY_SIZE; i++) {
        if (strcmp(consumer_conv_table[i].name, str) == 0) {
            return consumer_conv_table[i].keycode;
        }
    }
    return 0;
}

uint8_t hid_report_char_to_keycode(char c, uint8_t* modifier) {
    uint8_t idx = (uint8_t)c;
    if (idx >= 128) return 0;

    const KeyEntry* e = &hid_layout_get_active_table()[idx];
    if (modifier) *modifier |= e->modifier;
    return e->keycode;
}
