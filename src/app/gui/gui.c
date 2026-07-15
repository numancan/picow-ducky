#include <stdbool.h>

#include "FreeRTOS.h"
#include "app/task_manager/task_manager.h"
#include "hal/hal_display.h"
#include "input.h"
#include "middleware/sys_fault.h"
#include "modules/menu.h"
#include "queue.h"
#include "task.h"
#include "u8g2.h"
#include "view_ids.h"
#include "view_manager.h"

#define GUI_INPUT_QUEUE_DEPTH 6
#define GUI_TICK_PERIOD_MS 100

static u8g2_t u8g2;
static ViewManager view_manager;
static Menu main_menu;
static MenuItem main_items[2];

static void switch_to_ducky_cb(void* context) {
    ViewManager* vm = (ViewManager*)context;
    view_manager_push(vm, VIEW_ID_DUCKY);
}

static void switch_to_net_manager_cb(void* context) {
    ViewManager* vm = (ViewManager*)context;
    view_manager_push(vm, VIEW_ID_NET_MANAGER);
}

ViewManager* gui_get_view_manager() { return &view_manager; }

void gui_init() {
    view_manager_init(&view_manager, &u8g2);

    menu_init_fixed(&main_menu, "Main Menu", main_items, count_of(main_items));
    menu_add_item(&main_menu, "Ducky", switch_to_ducky_cb, &view_manager);
    menu_add_item(&main_menu, "Net Manager", switch_to_net_manager_cb, &view_manager);

    view_manager_add_view(&view_manager, VIEW_ID_MENU, menu_get_view(&main_menu));
}

void gui_task(void* pvParameters) {
    (void)pvParameters;

    hal_display_init(&u8g2);

    view_manager_push(&view_manager, VIEW_ID_MENU);

    // TODO: no need pubsub maybe use pico-sdk queue_try_add
    QueueHandle_t input_queue = input_subscribe(GUI_INPUT_QUEUE_DEPTH);
    InputEvent input_event;
    TickType_t last_tick = xTaskGetTickCount();

    while (1) {
        /* Drain the queue, processing input before drawing so changes show
         * this frame. */
        while (xQueueReceive(input_queue, &input_event, 0) == pdTRUE) {
            view_manager_input(&view_manager, &input_event);
        }

        TickType_t now = xTaskGetTickCount();
        if (now - last_tick >= pdMS_TO_TICKS(GUI_TICK_PERIOD_MS)) {
            last_tick = now;
            if (view_manager_tick(&view_manager)) {
                view_manager_request_redraw(&view_manager);
            }
        }

        /* needs_redraw is set by consumed input, view switches, and (later)
         * worker updates. */
        if (view_manager_needs_redraw(&view_manager)) {
            view_manager_draw(&view_manager);
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10))) {
            break;
        }
    }

    input_unsubscribe(input_queue);

    task_manager_report_stopped(TASK_MANAGER_TASK_GUI);
    vTaskDelete(NULL);
}
