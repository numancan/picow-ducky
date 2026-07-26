#include <malloc.h>
#include <stdint.h>

#include "duckyscript/ducky.h"
#include "gui/gui.h"
#include "hal/hal.h"
#include "middleware/fat_io.h"
#include "middleware/input.h"
#include "middleware/radio_manager.h"
#include "middleware/sleep_manager.h"
#include "middleware/sys_fault.h"
#include "my_rtc.h"
#include "net_manager/net_manager.h"
#include "power_manager/power_manager.h"
#include "task.h"

int main() {
    stdio_uart_init();

    // If the previous run armed a sleep, drop into dormant here (bare, single
    // core, no scheduler) and reboot on wake; otherwise return and boot on.
    sleep_manager_boot_check();

    time_init();
    adc_init();
    hal_init();

    sleep_manager_init();

    PANIC_IF(fat_io_init() != FAT_IO_OK, "Failed to mount sd card!");

    radio_manager_init();
    input_init();
    gui_init();
    power_manager_init();
    ducky_init();
    net_manager_init();

    vTaskStartScheduler();

    for (;;);
}