#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "bsp/board.h"

#include "lib/hid/keyboard.h"
#include "lib/sd_memory/sd_memory.h"
#include "lib/duckyscript/handler.h"

bool is_ducky_working = false;
// HID_STATUS hid_status = HID_TASK_NOT_READY;
void led_blinking_task(void);

void main(void)
{

  const uint8_t BLINK_LED = 15;

  gpio_init(BLINK_LED);
  gpio_set_dir(BLINK_LED, GPIO_OUT);
  gpio_put(BLINK_LED, 1);

  kb_init();
  sdm_mount();

  uint8_t i = 0;
  ducky_line_t *dl;
  TCHAR buf[256];

  
  while (1)
  {
    if (board_button_read())  {
      while (board_button_read()) {};
      is_ducky_working = true;
      // sdm_mount();
      sdm_open_read("payloads/firefox_pass.txt");
    }

    if (is_ducky_working && dh_is_ready()) {

      // char buf1[][256] = {"STRING 0123456789\r\n", "STRING asASas\r\n"};
      
      
      if(sdm_gets(buf)) {
        dl = dp_parse_line(buf);
        dh_handle_dline(dl);

      }
      else {
        is_ducky_working = false;
        // sdm_close_file();
        i = 0;
      }

    }

    kb_task();
    led_blinking_task();
  } 
}

void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < 500) return; // not enough time
  start_ms += 500;

  gpio_put(15, led_state);
  led_state = 1 - led_state; // toggle
}






    // char buf[][128]= { "GUI r\r\n", "DELAY 500\r\n", "STRING firefox\r\n", "ENTER\r\n", "DELAY 500\r\n", "STRING about:logins", "ENTER\r\n", "DELAY 500\r\n", "TAB", "REPEAT 9"};
    // char buf[][128] = {"DELAY 500\r\n", "STRING 3\r\n", "REPEAT 5\r\n" , "TAB\r\n", "REPEAT 4\r\n" };
    // char buf[][256] = {"STRING 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!#$%&()*+,-./:;<=>?@[\\]^_`{|}~\r\n"};
    // if (is_ducky_working && kb_status() == KB_STATUS_READY) {

    //   // if(f_gets(buf, sizeof(buf), &temp_fil)) {
    //   if(buf[i]) {
    //     char *r = strstr(buf[i], "\r");
    //     if (r) *r = '\0';

    //     handle_ducky(buf[i], 0);
    //     i++;
    //   }
    //   else {
    //     is_ducky_working = false;
    //     i = 0;
    //     sdm_close_file();
    //     sdm_unmount();
    //     gpio_put(15, 0);
    //   }
    // }


    
// /* Core 0 main function */
// int main(void)
// {
//   stdio_init_all();

//   // http_server_init();

//   multicore_launch_core1(main1);

//   while (1)
//   {
//     tight_loop_contents();
//   }

//   return 0;
// }