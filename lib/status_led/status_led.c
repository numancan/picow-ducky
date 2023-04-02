#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "status_led.h"

#define LPIN 7

static sl_interval_t blink_interval_ms;
static sl_blink_count_t blink_count;
static uint16_t pulse_interval_ms = SL_PULSE_SLOW;

void status_led_init()
{
  gpio_init(LPIN);
  gpio_set_dir(LPIN, GPIO_OUT);
  status_led_set_interval(SL_INT_500MS);
}

void led_blinking_task(void)
{
  static uint32_t start_ms = 0;

  // blink is disabled or status led always high
  if (blink_interval_ms < 2) return;

  if ( board_millis() - start_ms < blink_interval_ms) return;
  start_ms += blink_interval_ms;

  if (blink_count > 1) {
    for (size_t i = 0; i < blink_count; i++)
    {
      gpio_put(LPIN, 1);
      sleep_ms(pulse_interval_ms);
      gpio_put(LPIN, 0);
      sleep_ms(pulse_interval_ms);
    }

    return;
  }

  gpio_put(LPIN, !gpio_get(LPIN));
}

void status_led_set_interval(sl_interval_t interval_ms) 
{
  if (interval_ms == SL_HIGH || interval_ms == SL_DISABLED) {
    gpio_put(LPIN, interval_ms);
  }

  blink_interval_ms = interval_ms;
}

void status_led_set_pulse(sl_blink_count_t count, sl_pulse_t pulse)
{
  blink_interval_ms = SL_INT_3S;
  blink_count = count;
  pulse_interval_ms = pulse;
}

void status_led_task(void)
{
  status_led_init();

  while (1)
  {
    led_blinking_task();
    sleep_ms(1);
  }
}