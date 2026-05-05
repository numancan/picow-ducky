#include <hardware/adc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "FreeRTOS.h"
#include "app/duckyscript/ducky.h"
#include "app/gui/gui.h"
#include "app/hid/keyboard.h"
#include "app/settings/settings.h"
#include "app/status_led/status_led.h"
#include "app/task_manager/task_manager.h"
#include "app/web_server/web_server.h"
#include "hal/hal.h"
#include "hal/hal_battery.h"
#include "hal/hal_gpio.h"
#include "middleware/input.h"
#include "middleware/sd_card.h"
#include "my_rtc.h"
#include "pico/stdlib.h"
#include "task.h"

// mdns aç picow_ducky.local

#include "hal/hal_display.h"

static void battery_test_task(void* params) {
    (void)params;
    while (1) {
        BatteryChargeState charge_state = hal_battery_get_charge_state();
        printf("%s %.2f V\n", hal_battery_charge_state_str(charge_state), hal_battery_get_voltage());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// TODO: Check sd card inserted
int main() {
    // Initialize stdio
    stdio_init_all();
    time_init();
    adc_init();
    hal_init();

    sd_card_mount();
    settings_init();

    task_manager_start(TASK_MANAGER_TASK_STATUS_LED);

    bool should_inject = hal_gpio_read(gpio_inputs[INPUT_KEY_SELECT].pin) &&
                         hal_gpio_read(gpio_inputs[INPUT_KEY_DOWN].pin) &&
                         settings_get_bool(SETTINGS_ID_PAYLOAD_TO_INJECT);

    if (should_inject) {
        task_manager_start(TASK_MANAGER_TASK_USB_HID);
        task_manager_start(TASK_MANAGER_TASK_DUCKY);
        ducky_play_script(settings_get_param_wID(SETTINGS_ID_PAYLOAD_NAME)->val.s);
    } else {
        input_init();
        task_manager_start(TASK_MANAGER_TASK_INPUT);
        task_manager_start(TASK_MANAGER_TASK_GUI);
    }

    // if (settings_get_bool(SETTINGS_ID_WEB_SERVER_ENABLED)) {
    //     task_manager_start(TASK_MANAGER_TASK_WEB_SERVER);
    // }

    vTaskStartScheduler();

    for (;;);
}