#include "bsp/board.h"
#include "tusb.h"

#include "keyboard.h"

const uint8_t ascii_conv_table[128][2] =  { HID_ASCII_TO_KEYCODE };
const str_conv_t str_conv_table[] = { HID_STRING_TO_KEYCODE };

static uint32_t interval_ms = INTERVAL_MS_DEF;

// is there any keycode in buffer
static inline bool kb_has_keycode () { return kb_buffer.keycodes[0]; }

static inline void kb_report_keycodes() { tud_hid_keyboard_report(1, 0, kb_buffer.keycodes); }

static inline void kb_release() { tud_hid_keyboard_report(1, 0, NULL); }

static inline void kb_set_poll_ms(uint32_t ms) { if (ms >= INTERVAL_MS_MIN) interval_ms = ms; }

void kb_report_chr(uint8_t chr)
{
  uint8_t keycode[6] = { ascii_conv_table[chr][1] };
  uint8_t modifier   = ascii_conv_table[chr][0];

  tud_hid_keyboard_report(1, modifier, keycode);
}

void kb_init() 
{
  board_init();
  tusb_init();
}

void kb_send_reports()
{
  static bool is_key_pressed = false;

  if (!tud_hid_ready()) return;

  if (!is_key_pressed) {

    // TODO: açıklama
    if (kb_has_keycode()) {
      kb_report_keycodes();
      tu_memclr(kb_buffer.keycodes, 6);
    } else kb_report_chr(*kb_buffer.pString++);

    is_key_pressed = true;
  }
  else if (is_key_pressed) {

    // send empty key report if previously has key pressed
    kb_release();
    is_key_pressed = false;
    kb_buffer.counter--;
  }
}

void kb_task()
{
  // tinyusb device task
  tud_task(); 

  // Poll every 20ms
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return;
  start_ms += interval_ms;

  if (kb_is_buffer_empty()) return;
  
  kb_send_reports();
}

void kb_send_string(uint8_t *str)
{
  if (!(*str)) return;

  kb_buffer.pString = str;
  kb_buffer.counter = strlen(str) + 1;
}

void kb_send_keycodes(uint8_t keycodes[6])
{
  // TODO: 6 olması kritik mi? ya 2 elemanlı gelirse ne oluyor?
  memcpy(kb_buffer.keycodes, keycodes, 6);
  kb_buffer.counter = 1;
}

uint8_t kb_str_to_keycode(uint8_t *str)
{
  // TODO: sizeof(strconv_t)
  uint8_t len = count_of(str_conv_table);

  for(uint8_t i = 0; i < len; i++) {
    if (strcmp(str_conv_table[i].str, str) == 0 ) 
        return str_conv_table[i].keycode;
  }

  return 0;
}

// TODO:
uint8_t kb_ascii_to_keycode(uint8_t c)
{
  // TODO: 127?
  return c < 128 ? ascii_conv_table[c][1] : 0;
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, /*uint16_t*/ uint8_t len)
{
  if (kb_is_buffer_empty()) kb_sent_complete_cb();
}

void tud_mount_cb(void){}
void tud_umount_cb(void){}
void tud_suspend_cb(bool remote_wakeup_en){}
void tud_resume_cb(void){}
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize){}
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen){return 0;}