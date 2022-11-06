#ifndef _HID_KEYBOARD_H_
#define _HID_KEYBOARD_H_

#define HID_STRING_TO_KEYCODE           \
    { "GUI",    HID_KEY_GUI_LEFT },     \
    { "ALT",    HID_KEY_ALT_LEFT },     \
    { "F1",     HID_KEY_F1 },           \
    { "ENTER",  HID_KEY_ENTER }          


typedef struct {
  uint8_t  *pKeys;
  uint8_t   modifier;
  uint8_t   keycodes[6];
  int16_t   counter;
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

void handle_ducky(uint8_t*);

// void keyboard_report_chr(uint8_t);
// void append_buff_string(uint8_t*, uint8_t);
// uint8_t keycode_from_string(char*);
#endif