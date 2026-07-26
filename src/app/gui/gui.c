#include "gui.h"

#include <stdbool.h>

#include "FreeRTOS.h"
#include "config/task_config.h"
#include "hal/hal_display.h"
#include "middleware/input.h"
#include "middleware/log.h"
#include "middleware/sleep_manager.h"
#include "middleware/sys_fault.h"
#include "modules/menu.h"
#include "queue.h"
#include "task.h"
#include "u8g2.h"
#include "view_ids.h"
#include "view_manager.h"

#define TAG "gui"

static u8g2_t u8g2;
static ViewManager view_manager;
static Menu main_menu;
static MenuItem main_items[4];
static TaskHandle_t task_handle = NULL;

static void gui_shutdown_cb(void* context) {
    (void)context;
    xTaskNotifyGive(task_handle);
}

static void switch_to_ducky_cb(void* context) {
    ViewManager* vm = (ViewManager*)context;
    view_manager_push(vm, VIEW_ID_DUCKY);
}

static void switch_to_net_manager_cb(void* context) {
    ViewManager* vm = (ViewManager*)context;
    view_manager_push(vm, VIEW_ID_NET_MANAGER);
}

static void sleep_cb(void* context) {
    ViewManager* vm = (ViewManager*)context;
    view_manager_push(vm, VIEW_ID_SLEEP);
}

static void switch_to_battery_cb(void* context) {
    ViewManager* vm = (ViewManager*)context;
    view_manager_push(vm, VIEW_ID_BATTERY);
}

static void splash_view_draw(Canvas* canvas) {
    canvas_clear(canvas);
    canvas_set_font(canvas, CanvasFontPrimary);

    const char* text = "Picow Ducky";
    uint8_t x = (uint8_t)((CANVAS_WIDTH - canvas_string_width(canvas, text)) / 2);
    uint8_t y = (uint8_t)((CANVAS_HEIGHT + canvas_font_ascent(canvas)) / 2);
    canvas_draw_str(canvas, x, y, text);
    canvas_send(canvas);
}

ViewManager* gui_get_view_manager() { return &view_manager; }

void gui_init() {
    view_manager_init(&view_manager, &u8g2);

    menu_init_fixed(&main_menu, "Main Menu", main_items, count_of(main_items));
    menu_add_item(&main_menu, "Ducky", switch_to_ducky_cb, &view_manager);
    menu_add_item(&main_menu, "Net Manager", switch_to_net_manager_cb, &view_manager);
    menu_add_item(&main_menu, "Battery", switch_to_battery_cb, &view_manager);
    menu_add_item(&main_menu, "Sleep", sleep_cb, &view_manager);

    view_manager_add_view(&view_manager, VIEW_ID_MENU, menu_get_view(&main_menu));

    task_handle = task_create(&GUI_TASK_CONFIG, gui_task, NULL);
    PANIC_IF(task_handle == NULL, "gui task create failed");

    sleep_manager_register(gui_shutdown_cb, NULL);
}

void gui_task(void* pvParameters) {
    (void)pvParameters;

    hal_display_init(&u8g2);
    hal_display_on(&u8g2);

    view_manager_push(&view_manager, VIEW_ID_MENU);

    QueueHandle_t input_queue = input_get_event_queue();
    InputEvent input_event;
    TickType_t last_tick = xTaskGetTickCount();

    splash_view_draw(&view_manager.canvas);
    vTaskDelay(pdMS_TO_TICKS(500));
    xQueueReset(input_queue);

    while (1) {
        TickType_t now = xTaskGetTickCount();
        TickType_t elapsed = now - last_tick;
        TickType_t wait =
            (elapsed >= pdMS_TO_TICKS(GUI_TICK_PERIOD_MS)) ? 0 : pdMS_TO_TICKS(GUI_TICK_PERIOD_MS) - elapsed;

        /* Bir sonraki tick'e kalan süre kadar blokla */
        if (xQueueReceive(input_queue, &input_event, wait) == pdTRUE) {
            view_manager_input(&view_manager, &input_event);
            /* birikmiş diğer event'leri de boşalt */
            while (xQueueReceive(input_queue, &input_event, 0) == pdTRUE)
                view_manager_input(&view_manager, &input_event);
        }

        now = xTaskGetTickCount();
        if (now - last_tick >= pdMS_TO_TICKS(GUI_TICK_PERIOD_MS)) {
            last_tick = now;
            if (view_manager_tick(&view_manager)) {
                view_manager_request_redraw(&view_manager);
            }
        }

        if (view_manager_needs_redraw(&view_manager)) {
            view_manager_draw(&view_manager);
        }

        if (ulTaskNotifyTake(pdTRUE, 0)) break;
    }

    hal_display_off(&u8g2);
    vTaskDelay(pdMS_TO_TICKS(100));

    LOG_INFO(TAG, "shutdown");
    sleep_manager_ack_shutdown();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}
