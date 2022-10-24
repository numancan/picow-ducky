#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hid_keyboard.h"

#include "pico/multicore.h"

int main(void)
{
  stdio_init_all();
  hid_init();
  
  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
  }

  return 0;
}