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

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 1UL)

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

    status_led_init();

    sd_card_mount();
    settings_init();

    bool should_inject = hal_gpio_read(gpio_inputs[INPUT_KEY_SELECT].pin) &&
                         hal_gpio_read(gpio_inputs[INPUT_KEY_DOWN].pin) &&
                         settings_get_bool(SETTINGS_ID_PAYLOAD_TO_INJECT);

    if (should_inject) {
        xTaskCreate(ducky_task, "ducky", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        ducky_play_script(settings_get_param_wID(SETTINGS_ID_PAYLOAD_NAME)->val.s);
    } else {
        input_init();
        xTaskCreate(input_service, "input", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
        xTaskCreate(gui_task, "gui", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    }

    if (settings_get_bool(SETTINGS_ID_WEB_SERVER_ENABLED)) {
        xTaskCreate(web_server_task, "webserver", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, NULL);
    }

    // xTaskCreate(battery_test_task, "battery_test", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, NULL);

    // TODO: settings_init içine al & if web server
    // cgi_event_subscribe(settings_set_from_sd);

    vTaskStartScheduler();

    for (;;);
}