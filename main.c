#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "pico/multicore.h"

#include "lib/hid/keyboard.h"
#include "lib/sd_memory/sd_memory.h"
#include "lib/duckyscript/handler.h"
#include "lib/settings/settings.h"
#include "lib/status_led/status_led.h"

// #define PICOW

#ifdef PICOW
// #include "pico/cyw43_arch.h"
#include "lib/webserver/webserver.h"
#include "lib/webserver/post.h"
#define WS_SWITCH_PIN 4
#endif

bool is_ducky_working = false;
char next_payload[FILE_MAX_NAME_LEN + 9];

void trigger_payload(char *pname)
{
  snprintf(next_payload, sizeof(next_payload), "payloads/%s\0", pname);
  sdm_open_read(next_payload); /** TODO: böyle olmaz reis*/
}

#ifdef PICOW
void post_trigger_cb(char *pname)
{
  printf(pname);
  trigger_payload(pname);
}

void post_settings_cb()
{
  /** TODO: Reis böyle sıkıntı değil mi bari update olsun */
  settings_init("settings.txt");
}

void switch_init()
{
  gpio_init(WS_SWITCH_PIN);
  gpio_set_dir(WS_SWITCH_PIN, GPIO_IN);
}
#endif

int main(void)
{
  stdio_init_all();

  multicore_launch_core1(status_led_task);
  sleep_ms(500);

  if (sdm_mount() != FR_OK) {
    status_led_set_pulse(SL_TWICE, SL_PULSE_SLOW);
    while(1){};
  }
  settings_init("settings.txt");
  settings_print(true);

  bool is_ws_enabled = false;

#ifdef PICOW
  switch_init();
  is_ws_enabled = gpio_get(WS_SWITCH_PIN);

  status_led_set_pulse(SL_THRICE, SL_PULSE_FAST);
  if (is_ws_enabled && ws_init(S_WIFI_SSID, S_WIFI_PASS) != WS_ERR_OK) {
    status_led_set_pulse(SL_THRICE, SL_PULSE_SLOW);
    while(1){};
  }
#endif

  if (S_PTI || !is_ws_enabled) {
    trigger_payload(S_PAYLOAD_NAME);
  }

  kb_init();
  status_led_set_interval(SL_HIGH);

  while (1)
  {
    /** TODO: bu belki ducky icinde olabilir ducky_task?*/
    if (*next_payload && dh_is_ready()) {
      status_led_set_interval(SL_INT_250MS);

      ducky_line_t *dl;
      TCHAR buf[256];

      if(sdm_gets(buf)) {
        dl = dp_parse_line(buf);
        dh_handle_dline(dl);

      } else {
        memset(next_payload, 0, sizeof(next_payload));
        sdm_close_file();
        status_led_set_interval(SL_HIGH);
      }
    }

    kb_task();
  }
}