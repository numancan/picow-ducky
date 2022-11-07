#ifndef _HID_KEYBOARD_H_
#define _HID_KEYBOARD_H_

#define HID_STRING_TO_KEYCODE                 \
    { "GUI",          HID_KEY_GUI_LEFT },     \
    { "ALT",          HID_KEY_ALT_LEFT },     \
    { "CAPSLOCK",     HID_KEY_CAPS_LOCK },    \
    { "ENTER",        HID_KEY_ENTER },        \
    { "DELETE",       HID_KEY_DELETE },       \
    { "HOME",         HID_KEY_HOME },         \
    { "INSERT",       HID_KEY_INSERT },       \
    { "PAGEUP",       HID_KEY_PAGE_UP },      \
    { "PAGEDOWN",     HID_KEY_PAGE_DOWN },    \
    { "WINDOWS",      HID_KEY_GUI_LEFT },     \
    { "UPARROW",      HID_KEY_ARROW_UP },     \
    { "DOWNARROW",    HID_KEY_ARROW_DOWN },   \
    { "LEFTARROW",    HID_KEY_ARROW_LEFT },   \
    { "RIGHTARROW",   HID_KEY_ARROW_RIGHT },  \
    { "TAB",          HID_KEY_TAB },          \
    { "SHIFT",        HID_KEY_SHIFT_LEFT},    \
    { "SPACE",        HID_KEY_SPACE },        \
    { "PRINTSCREEN",  HID_KEY_PRINT_SCREEN }, \
    { "ESC",          HID_KEY_ESCAPE },       \
    { "ESCAPE",       HID_KEY_ESCAPE },       \
    { "PAUSE",        HID_KEY_PAUSE },        \
    { "HOME",         HID_KEY_HOME },         \
    { "F1",           HID_KEY_F1 },           \
    { "F2",           HID_KEY_F2 },           \
    { "F3",           HID_KEY_F3 },           \
    { "F4",           HID_KEY_F4 },           \
    { "F5",           HID_KEY_F5 },           \
    { "F6",           HID_KEY_F6 },           \
    { "F7",           HID_KEY_F7 },           \
    { "F8",           HID_KEY_F8 },           \
    { "F9",           HID_KEY_F9 },           \
    { "F10",          HID_KEY_F10 },          \
    { "F11",          HID_KEY_F11 },          \
    { "F12",          HID_KEY_F12 },          \
    { "MENU",         HID_KEY_MENU },         \
    { "APP",          HID_KEY_APPLICATION },  \
    { "SCROLLLOCK",   HID_KEY_SCROLL_LOCK }   \

typedef struct {
  uint8_t  *pKeys;
  uint8_t   modifier;
  uint8_t   keycodes[6];
  int16_t   counter;
  uint8_t   last_payload[128];
  uint8_t   repeat_count;
} key_buffer_t;

typedef struct {
    char    *str;
    uint8_t  keycode;
}str_conv_t;

typedef enum  {
  HID_TASK_NOT_READY = 0,
  HID_TASK_BUSY = 1,
  HID_TASK_NOT_BUSY = 2
}HID_STATUS;

enum  {
  BLINK_DISABLED = 0,
  BLINK_NOT_MOUNTED = 250,  // device not mounted
  BLINK_MOUNTED = 1000,     // device mounted
  BLINK_SUSPENDED = 2500,   // device is suspended
};

void hid_init(void);
void led_blinking_task(void);
HID_STATUS hid_task();

void handle_ducky(uint8_t*, bool);

// void keyboard_report_chr(uint8_t);
// void append_buff_string(uint8_t*, uint8_t);
// uint8_t keycode_from_string(char*);
#endif