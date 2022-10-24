#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hid_keyboard.h"

#include "pico/multicore.h"

/* Core 1 main function */
void main1(void)
{

}

/* Core 0 main function */
int main(void)
{
  stdio_init_all();

  multicore_launch_core1(main1);
  hid_init();
  
  while (1)
  {
    /* HID tasks */
    tud_task(); // tinyusb device task
    led_blinking_task();
    hid_task();


  }

  return 0;
}