#include <stdlib.h>

#include <hardware/gpio.h>
#include "bsp/board.h"
#include "hid_keyboard.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define BLINK_LED 15

const uint8_t ascii_conv_table[128][2] =  { HID_ASCII_TO_KEYCODE };
const str_conv_t str_conv_table[] = { HID_STRING_TO_KEYCODE };

uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

static key_buffer_t key_buff = { 0, 0, 0, 0};

/* Buffer must be ended with \0 */
bool is_buffer_not_empty() { return key_buff.counter > 0; }

void hid_init() 
{ 
  gpio_init(BLINK_LED);
  gpio_set_dir(BLINK_LED, GPIO_OUT);
  board_init();
  tusb_init();

  gpio_put(BLINK_LED, 1);
}

void _keyboard_release() { tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);  }

void keyboard_report_keycode(uint8_t keycode[]) 
{
  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
}

void keyboard_report_chr(uint8_t chr)
{
  uint8_t keycode[6] = { };
  uint8_t modifier   = 0;

  if ( ascii_conv_table[chr][0] ) modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  keycode[0] = ascii_conv_table[chr][1];

  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
}

// void append_buff_keycode(uint8_t keycode) 
// {
//   key_buffer.b[key_buffer.counter++][0] = keycode;
// }

// void append_buff_string(uint8_t* str, uint8_t len)
// {
//   if (len > sizeof(key_buffer.b)) return;

//   for (uint8_t i = 0; i < len; i++)
//   {
//     key_buffer.b[i][0] = ascii_conv_table[str[len - i - 1]][1];
//     key_buffer.counter++;
//   }
  
//   // memcpy(key_buffer.b, str, len);
//   // key_buffer.counter = len - 1;
// }

uint8_t keycode_from_string(char *str)
{
    uint8_t len = sizeof(str_conv_table)/sizeof(str_conv_t);
    
    for(uint8_t i = 0; i < len; i++) {
        if (strcmp(str_conv_table[i].str, str) == 0 ) 
            return str_conv_table[i].keycode;
    }
    
    return 0;
}

volatile bool is_delayed = false;

int64_t delay_callback(alarm_id_t id, void *user_data) {
  is_delayed = false;
  gpio_put(BLINK_LED, 0);
  
  return 0;
}

void handle_ducky(uint8_t *payload)
{
  char *pTemp;

  // if (payload) {
  //   key_buff.pKeys = payload; 
  //   key_buff.counter = strlen(key_buff.pKeys); 
  //   return;
  // }
  /* Splitting command from payload, pTemp pointing the command */
  pTemp = strtok(payload, " ");

  uint8_t keycode = keycode_from_string(pTemp);
  
  if (keycode) {
    pTemp = strtok(NULL, "");
    size_t len = strlen(pTemp);

    key_buff.keycodes[0] = keycode;

    /* TODO: sarmadı */
    if (len > 5) return;
    for (size_t i = 0; i < len; i++)
    {
      key_buff.keycodes[i + 1] = ascii_conv_table[pTemp[i]][1];
    }    

    key_buff.counter++;
  }
  else if (strcmp(pTemp, "STRING") == 0) {
    /* Splitting string that will press \0 */
    pTemp = strtok(NULL, "");

    if (pTemp) {
      key_buff.pKeys = pTemp; 
      key_buff.counter = strlen(key_buff.pKeys); 
    }
  }
  else if (strcmp(pTemp, "DELAY") == 0) {
    /* Splitting delay time */
    pTemp = strtok(NULL, "");
    uint32_t delay_ms = strtol(pTemp, (char **)NULL, 10);

    add_alarm_in_ms(delay_ms, delay_callback, NULL, false);
    is_delayed = true;
  }
  else {
    printf("%s COMMAND NOT FOUND \n", pTemp);
  }
}

// static bool start = false;

void _send_key_report()
{
  static bool is_key_pressed = false;

  if (!tud_hid_ready()) return;

  if (!is_key_pressed) {
    if (key_buff.keycodes[0]) {
      keyboard_report_keycode(key_buff.keycodes);
      tu_memclr(key_buff.keycodes, 6);
    }
    else keyboard_report_chr(*key_buff.pKeys++);
    // keyboard_report_keycode(key_buffer.b[key_buffer.counter]);
    is_key_pressed = true;
  }
  else if (is_key_pressed) {

    // send empty key report if previously has key pressed
    _keyboard_release();
    key_buff.counter--;
    is_key_pressed = false;
  }
}

/* HID BUSY RETURN 1 */
HID_STATUS hid_task()
{
  // Poll every 50ms
  const uint32_t interval_ms = 50;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return 0;
  start_ms += interval_ms;

  if (is_delayed) return 0;
  if (is_buffer_not_empty()) {
    _send_key_report();
    return HID_TASK_BUSY;
  }

  return HID_TASK_NOT_BUSY;
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
