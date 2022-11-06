#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bsp/board.h"
#include "hid_keyboard.h"
#include "http_server.h"

// #include "sd_memory.h"

bool is_ducky_working = false;
HID_STATUS hid_status = HID_TASK_NOT_READY;

/* Core 1 main function */
void main(void)
{
  // sdm_init();
  hid_init();

  static uint8_t i = 0;

  while (1)
  {
    if (board_button_read()) {
      while (board_button_read()) {};
      is_ducky_working = true;
      // sdm_mount();
      // sdm_open_read("deneme.txt");
    }

    if (is_ducky_working && hid_status == HID_TASK_NOT_BUSY) {
      char buf[][128]= { "GUI R\r\n", "STRING CMD\r\n", "ENTER\r\n", "DELAY 3000\r\n" ,"STRING ping 8.8.8.8\r\n", "ENTER\r\n"};
/*

*/
      // if(f_gets(buf, sizeof(buf), &tempFil)) {
      if(buf[i]) {
        char *r = strstr(buf[i], "\r");
        if (r) *r = '\0';

        handle_ducky(buf[i++]);
      }
      else {
        is_ducky_working = false;
        i = 0;
        // sdm_close_file();
        // sdm_unmount();
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
