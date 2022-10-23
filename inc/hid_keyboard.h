#ifndef _HID_KEYBOARD_H_
#define _HID_KEYBOARD_H_

#include "bsp/board.h"

#define BLINK_LED 15

enum  {
  BLINK_DISABLED = 0,
  BLINK_NOT_MOUNTED = 250,  // device not mounted
  BLINK_MOUNTED = 1000,     // device mounted
  BLINK_SUSPENDED = 2500,   // device is suspended
};



void hid_init(void);
void led_blinking_task(void);
void hid_task(void);

#endif