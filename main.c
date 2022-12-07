#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
// #include "pico/cyw43_arch.h"
#include "pico/multicore.h"

#include "lib/hid/keyboard.h"
#include "lib/sd_memory/sd_memory.h"
#include "lib/duckyscript/handler.h"
#include "lib/webserver/webserver.h"
#include "lib/webserver/post.h"
#include "lib/settings/settings.h"

bool is_ducky_working = false;

void post_trigger_cb(char *pname)
{
  printf("%s\n", pname);
}

void post_settings_cb()
{
  settings_init("settings.txt");
}

void led_init()
{
  const uint LPIN = 6;
  gpio_init(LPIN);
  gpio_set_dir(LPIN, GPIO_OUT);
  gpio_put(LPIN, 1);
}

int main(void)
{
  stdio_init_all();
  led_init();

  if (sdm_mount() == FR_OK) {
    settings_init("settings.txt");
    ws_init(S_WIFI_SSID, S_WIFI_PASS);
  }

  // uint8_t files[256];
  // sdm_read_dir("/payloads", files, sizeof(files));

  // printf("files %s\n", files);

  while (1)
  {
    tight_loop_contents();
  }
}

// int main(void)
// {
//   stdio_init_all();

//   led_init();
//   sdm_mount();
//   sleep_ms(500);

//   ws_init();
//   sleep_ms(1000);
  
//   kb_init();

//   uint8_t i = 0;
//   ducky_line_t *dl;
//   TCHAR buf[256];

//   while (1)
//   {
//     if (!is_ducky_working && board_button_read())  {
//       is_ducky_working = true;

//       sdm_open_read("payloads/firefox_pass.txt");
//       // sdm_close_file();
//       // gpio_put(BLINK_LED, !gpio_get_out_level(BLINK_LED));
//     }

//     if (is_ducky_working && dh_is_ready()) {

//       // char buf1[][256] = { "STRING 123123123123123123123123123123120123456789\r\n", "DELAY 1500\r\n", "STRING 0123456789\r\n", "DELAY 500\r\n", "STRING 0123456789\r\n", "DELAY 2000\r\n", "STRING end\r\n" };
      
//       if(sdm_gets(buf)) {
//       // if(i < 7) {
//         // dl = dp_parse_line(buf1[i++]);
//         dl = dp_parse_line(buf);
//         dh_handle_dline(dl);
//       }
//       else {
//         is_ducky_working = false;
//         sdm_close_file();
//         i = 0;
//       }
//     }

//     kb_task();
//     // led_blinking_task();
//   }

//   sdm_unmount();
// }