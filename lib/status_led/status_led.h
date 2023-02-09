#ifndef _STATUS_LED_H_
#define _STATUS_LED_H_

typedef enum {
  SL_DISABLED       = 0,
  SL_HIGH           = 1,    /** Always Enabled */
  SL_INT_250MS      = 250,
  SL_INT_500MS      = 500,
  SL_INT_1S         = 1000,
  SL_INT_2S         = 2000,
} sl_interval_t;

typedef enum {
  SL_ONCE         = 1,
  SL_TWICE        = 2,
  SL_THRICE       = 3,
} sl_blink_count_t;

void status_led_task();
void status_led_set_blink(sl_interval_t interval_ms, sl_blink_count_t count);

#endif