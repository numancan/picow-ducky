#include "gui.h"

#include <stdio.h>
#include <stdlib.h>

#include "duckyscript/ducky_view.h"
#include "hal/hal_display.h"
#include "menu_view.h"
#include "middleware/input.h"
#include "pico/stdlib.h"
#include "settings/settings_view.h"
#include "view_manager.h"
#include "hal/hal_power.h"
#include "app/task_manager/task_manager.h"

static void sleep_action() {
    hal_power_deep_sleep();
}

static u8g2_t u8g2;
ViewManager view_manager;
QueueHandle_t input_queue = NULL;

void gui_task(void *pvParameters) {
    (void)pvParameters;
    hal_display_init(&u8g2);

    u8g2_SetFont(&u8g2, u8g2_font_profont11_tr);
    u8g2_SetDrawColor(&u8g2, 1);

    input_queue = xQueueCreate(6, sizeof(InputEvent*));
    pubsub_free_subscribe(input_get_pubsub(), input_queue);

    View main_view;
    View settings_view;
    View ducky_view;

    settings_view_init(&settings_view, &main_view);
    ducky_view_init(&ducky_view, &main_view);

    MenuItem main_items[] = {MENU_ITEM_SUBMENU("Play", &ducky_view),
                             MENU_ITEM_SUBMENU("Settings", &settings_view),
                             MENU_ITEM_ACTION("Sleep", sleep_action)};

    Menu main_menu = {
        .title = "Main Menu", .items = main_items, .item_count = count_of(main_items)};

    menu_view_init(&main_view, &main_menu, NULL);

    view_manager_init(&view_manager, &u8g2, &main_view);

    InputEvent tmp_input_event;

    while (1) {
        view_manager_draw(&view_manager);

        if (xQueueReceive(input_queue, &tmp_input_event, 0) == pdTRUE) {
            view_manager_input(&view_manager, &tmp_input_event);
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10))) {
            break;
        }
    }

    // Cleanup
    if (input_queue != NULL) {
        pubsub_free_unsubscribe(input_get_pubsub(), input_queue);
        vQueueDelete(input_queue);
        input_queue = NULL;
    }

    printf("[GUI] Task stopping...\n");
    task_manager_report_stopped(TASK_MANAGER_TASK_GUI);
    vTaskDelete(NULL);
}