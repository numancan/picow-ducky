#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "middleware/enum_gen.h"
#include "tusb.h"

#define HID_STRING_TO_MODIFIER                                                                    \
    {"CTRL", KEYBOARD_MODIFIER_LEFTCTRL}, {"CONTROL", KEYBOARD_MODIFIER_LEFTCTRL},                \
        {"SHIFT", KEYBOARD_MODIFIER_LEFTSHIFT}, {"ALT", KEYBOARD_MODIFIER_LEFTALT},               \
        {"GUI", KEYBOARD_MODIFIER_LEFTGUI}, {"WINDOWS", KEYBOARD_MODIFIER_LEFTGUI},               \
        {"RIGHTCTRL", KEYBOARD_MODIFIER_RIGHTCTRL}, {"RIGHTSHIFT", KEYBOARD_MODIFIER_RIGHTSHIFT}, \
        {"RIGHTALT", KEYBOARD_MODIFIER_RIGHTALT}, {"RIGHTGUI", KEYBOARD_MODIFIER_RIGHTGUI}, {     \
        "RIGHTWINDOWS", KEYBOARD_MODIFIER_RIGHTGUI                                                \
    }

#define MODIFIER_ARRAY_SIZE sizeof(modifier_conv_table) / sizeof(modifier_conv_table[0])

// $EXPORT=NAME,KEYCODE
#define HID_STRING_TO_SPECIAL_KEY                                                                                     \
    {"CTRL", HID_KEY_CONTROL_LEFT}, {"RCTRL", HID_KEY_CONTROL_RIGHT}, {"RIGHTCTRL", HID_KEY_CONTROL_RIGHT},           \
        {"CONTROL", HID_KEY_CONTROL_LEFT}, {"SHIFT", HID_KEY_SHIFT_LEFT}, {"RSHIFT", HID_KEY_SHIFT_RIGHT},            \
        {"RIGHTSHIFT", HID_KEY_SHIFT_RIGHT}, {"ALT", HID_KEY_ALT_LEFT}, {"RALT", HID_KEY_ALT_RIGHT},                  \
        {"RIGHTALT", HID_KEY_ALT_RIGHT}, {"WINDOWS", HID_KEY_GUI_LEFT}, {"RWINDOWS", HID_KEY_GUI_RIGHT},              \
        {"RIGHTWINDOWS", HID_KEY_GUI_RIGHT}, {"GUI", HID_KEY_GUI_LEFT}, {"RGUI", HID_KEY_GUI_RIGHT},                  \
        {"RIGHTGUI", HID_KEY_GUI_RIGHT}, {"COMMAND", HID_KEY_GUI_LEFT}, {"RCOMMAND", HID_KEY_GUI_RIGHT},              \
        {"OPTION", HID_KEY_ALT_LEFT}, {"ROPTION", HID_KEY_ALT_RIGHT}, {"ESC", HID_KEY_ESCAPE},                        \
        {"ESCAPE", HID_KEY_ESCAPE}, {"ENTER", HID_KEY_ENTER}, {"UP", HID_KEY_ARROW_UP}, {"DOWN", HID_KEY_ARROW_DOWN}, \
        {"LEFT", HID_KEY_ARROW_LEFT}, {"RIGHT", HID_KEY_ARROW_RIGHT}, {"UPARROW", HID_KEY_ARROW_UP},                  \
        {"DOWNARROW", HID_KEY_ARROW_DOWN}, {"LEFTARROW", HID_KEY_ARROW_LEFT}, {"RIGHTARROW", HID_KEY_ARROW_RIGHT},    \
        {"SPACE", HID_KEY_SPACE}, {"BACKSPACE", HID_KEY_BACKSPACE}, {"TAB", HID_KEY_TAB},                             \
        {"CAPSLOCK", HID_KEY_CAPS_LOCK}, {"PRINTSCREEN", HID_KEY_PRINT_SCREEN}, {"SCROLLLOCK", HID_KEY_SCROLL_LOCK},  \
        {"PAUSE", HID_KEY_PAUSE}, {"BREAK", HID_KEY_PAUSE}, {"INSERT", HID_KEY_INSERT}, {"HOME", HID_KEY_HOME},       \
        {"PAGEUP", HID_KEY_PAGE_UP}, {"PAGEDOWN", HID_KEY_PAGE_DOWN}, {"DELETE", HID_KEY_DELETE},                     \
        {"END", HID_KEY_END}, {"MENU", HID_KEY_MENU}, {"APP", HID_KEY_APPLICATION}, {"POWER", HID_KEY_POWER},         \
        {"F1", HID_KEY_F1}, {"F2", HID_KEY_F2}, {"F3", HID_KEY_F3}, {"F4", HID_KEY_F4}, {"F5", HID_KEY_F5},           \
        {"F6", HID_KEY_F6}, {"F7", HID_KEY_F7}, {"F8", HID_KEY_F8}, {"F9", HID_KEY_F9}, {"F10", HID_KEY_F10},         \
        {"F11", HID_KEY_F11}, {"F12", HID_KEY_F12}, {"F13", HID_KEY_F13}, {"F14", HID_KEY_F14}, {"F15", HID_KEY_F15}, \
        {"F16", HID_KEY_F16}, {"F17", HID_KEY_F17}, {"F18", HID_KEY_F18}, {"F19", HID_KEY_F19}, {"F20", HID_KEY_F20}, \
        {"F21", HID_KEY_F21}, {"F22", HID_KEY_F22}, {"F23", HID_KEY_F23}, {"F24", HID_KEY_F24},                       \
        {"NUMLOCK", HID_KEY_NUM_LOCK}, {"KP_SLASH", HID_KEY_KEYPAD_DIVIDE}, {"KP_ASTERISK", HID_KEY_KEYPAD_MULTIPLY}, \
        {"KP_MINUS", HID_KEY_KEYPAD_SUBTRACT}, {"KP_PLUS", HID_KEY_KEYPAD_ADD}, {"KP_ENTER", HID_KEY_KEYPAD_ENTER},   \
        {"KP_0", HID_KEY_KEYPAD_0}, {"KP_1", HID_KEY_KEYPAD_1}, {"KP_2", HID_KEY_KEYPAD_2},                           \
        {"KP_3", HID_KEY_KEYPAD_3}, {"KP_4", HID_KEY_KEYPAD_4}, {"KP_5", HID_KEY_KEYPAD_5},                           \
        {"KP_6", HID_KEY_KEYPAD_6}, {"KP_7", HID_KEY_KEYPAD_7}, {"KP_8", HID_KEY_KEYPAD_8},                           \
        {"KP_9", HID_KEY_KEYPAD_9}, {"KP_DOT", HID_KEY_KEYPAD_DECIMAL}, {                                             \
        "KP_EQUAL", HID_KEY_KEYPAD_EQUAL                                                                              \
    }

#define SPECIAL_KEY_ARRAY_SIZE sizeof(special_conv_table) / sizeof(special_conv_table[0])

// $EXPORT=NAME,USAGE
#define HID_STRING_TO_CONSUMER_KEY                                                                          \
    {"MK_VOLUP", HID_USAGE_CONSUMER_VOLUME_INCREMENT}, {"MK_VOLDOWN", HID_USAGE_CONSUMER_VOLUME_DECREMENT}, \
        {"MK_MUTE", HID_USAGE_CONSUMER_MUTE}, {"MK_PREV", HID_USAGE_CONSUMER_SCAN_PREVIOUS},                \
        {"MK_NEXT", HID_USAGE_CONSUMER_SCAN_NEXT}, {"MK_PP", HID_USAGE_CONSUMER_PLAY_PAUSE}, {              \
        "MK_STOP", HID_USAGE_CONSUMER_STOP                                                                  \
    }

#define CONSUMER_KEY_ARRAY_SIZE sizeof(consumer_conv_table) / sizeof(consumer_conv_table[0])

// NOTE: The HID_STRING_TO_* macros and layout tables below reference TinyUSB HID
// constants (HID_KEY_*, KEYBOARD_MODIFIER_*, HID_USAGE_CONSUMER_*). They are only
// expanded in hid_report.c, which includes "tusb.h". This header deliberately
// does NOT include tusb.h so it can be used by the BLE backend without pulling in
// TinyUSB's hid_report_type_t (which collides with btstack's).
typedef enum {
    REPORT_ID_KEYBOARD = 1,
    REPORT_ID_MOUSE,
    REPORT_ID_CONSUMER_CONTROL,
    REPORT_ID_GAMEPAD,
    REPORT_ID_COUNT,
} HIDReportKind;

// $EXPORT=ID,STATUS_NAME
#define HID_STATUS_LIST(X)                                                                                       \
    X(HID_STATUS_OK, "OK")                                                                                       \
    X(HID_STATUS_UNINITIALIZED, "UNINITIALIZED") /* Driver/stack not yet started or stoped. */                   \
    X(HID_STATUS_DISCONNECTED, "DISCONNECTED")   /* Device (BLE/USB) working but not connected */                \
    X(HID_STATUS_BUSY, "BUSY")                   /* Connected but buffer is full; previous data not sent yet. */ \
    X(HID_STATUS_SUSPENDED, "SUSPENDED")         /* Host (PC/phone) is suspended and not accepting data. */      \
    X(HID_STATUS_TIMEOUT, "TIMEOUT")                                                                             \
    X(HID_STATUS_UNKNOWN, "UNKNOWN")

DECLARE_ENUM(HidStatus, HID_STATUS_COUNT, HID_STATUS_LIST)

typedef struct {
    HIDReportKind kind;

    union {
        struct {
            uint8_t modifier;
            uint8_t keycodes[6];
        } keyboard;

        struct {
            uint8_t buttons;
            int8_t x;
            int8_t y;
            int8_t wheel;
        } mouse;

        struct {
            uint16_t usage;
        } consumer;
    };
} HIDReport;

HidStatus hid_report_key(uint8_t modifier, uint8_t keycode);
HidStatus hid_report_keys(uint8_t modifier, const uint8_t keycodes[6]);
HidStatus hid_report_char(char c, uint32_t char_delay);
HidStatus hid_report_string(const char* str, uint32_t char_delay);

HidStatus hid_report_mouse_move(int8_t x, int8_t y);
HidStatus hid_report_mouse_click(uint8_t buttons);
HidStatus hid_report_mouse_scroll(int8_t wheel);
HidStatus hid_report_consumer(uint16_t usage);

const char* hid_report_status_name(HidStatus status);

uint8_t hid_report_str_to_mod(const char* str);
uint8_t hid_report_str_to_special(const char* str);
uint16_t hid_report_str_to_consumer(const char* str);
uint8_t hid_report_char_to_keycode(char c, uint8_t* modifier);
