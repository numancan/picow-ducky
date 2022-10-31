#ifndef _HID_KEYBOARD_H_
#define _HID_KEYBOARD_H_

#define HID_STRING_TO_KEYCODE       \
    { "GUI", HID_KEY_GUI_LEFT },    \
    { "ALT", HID_KEY_ALT_LEFT },    \
    { "F1",  HID_KEY_F1 }           


typedef struct {
  int16_t counter;      /* Buffer counter */
  // uint8_t size;      /* Current element in buffer */
  uint8_t b[128];
} key_buffer_t;

typedef struct {
    char *str;
    uint8_t keycode;
}str_conv_t;

enum  {
  BLINK_DISABLED = 0,
  BLINK_NOT_MOUNTED = 250,  // device not mounted
  BLINK_MOUNTED = 1000,     // device mounted
  BLINK_SUSPENDED = 2500,   // device is suspended
};

void hid_init(void);
void led_blinking_task(void);
bool hid_task();

void keyboard_report_key(uint8_t);
void keyboard_report_string(uint8_t*, uint8_t);
uint8_t keycode_from_string(char*);
void handle_ducky(uint8_t*);

#endif