#include <stdlib.h>
#include <stdio.h>

#include <hardware/gpio.h>
#include "hid_keyboard.h"
#include "tusb.h"

#include "usb_descriptors.h"

#define BLINK_LED 15

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
uint8_t keys[] = { HID_KEY_GUI_LEFT, HID_KEY_C, HID_KEY_M, HID_KEY_D, HID_KEY_ENTER, HID_KEY_P, HID_KEY_I, HID_KEY_N, HID_KEY_G, HID_KEY_SPACE, HID_KEY_KEYPAD_8, HID_KEY_PERIOD, HID_KEY_KEYPAD_8, HID_KEY_PERIOD, HID_KEY_KEYPAD_8, HID_KEY_PERIOD, HID_KEY_KEYPAD_8, HID_KEY_ENTER, 0x00 };


void hid_init() 
{
  gpio_init(BLINK_LED);
  gpio_set_dir(BLINK_LED, GPIO_OUT);

  board_init();
  tusb_init();
}

static bool start = false;

void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 20;
  static uint32_t start_ms = 0;
  static uint8_t index = 0;
  static bool has_keyboard_key = false;
  

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  if (board_button_read()) {start = true;}

  // Remote wakeup
  if ( tud_suspended())
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    if (start == false) return;
    if ( !tud_hid_ready()) return;
    if (!has_keyboard_key) {
      uint8_t keycode[6] = { 0 };
      keycode[0] = keys[index++];
      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
      has_keyboard_key = true;
      
      if (index == 19) {sleep_ms(50000); tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);};
    }
    else if (has_keyboard_key) {
      // send empty key report if previously has key pressed
      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
      if (index == 5) sleep_ms(1000);
      has_keyboard_key = false;
    }
  }
}


//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  gpio_put(BLINK_LED, led_state);
  led_state = 1 - led_state; // toggle
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// HID callbacks
//--------------------------------------------------------------------+

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, /*uint16_t*/ uint8_t len)
{
  (void) instance;
  (void) report;
  (void) len;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}
