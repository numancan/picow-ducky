#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "bsp/board.h"
#include "hid_keyboard.h"
#include "http_server.h"

/* Core 1 main function */
void main1(void)
{
  hid_init();
  
  while (1)
  {
    if (board_button_read()) {
      while (board_button_read()) {};
      
      uint8_t chr[100] = "STRING ping 8.8.8.8\n";

      handle_ducky(chr);

      // keyboard_report_string(chr, strlen(chr));
    }

    /* HID tasks */
    tud_task(); // tinyusb device task
    hid_task();
    led_blinking_task();
  } 
}

/* Core 0 main function */
int main(void)
{
  stdio_init_all();

  // http_server_init();

  multicore_launch_core1(main1);

  while (1)
  {
    tight_loop_contents();
  }

  return 0;
}
