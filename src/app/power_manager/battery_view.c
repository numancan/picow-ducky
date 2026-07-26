#include <stdio.h>

#include "gui/elements.h"
#include "gui/view.h"
#include "gui/view_ids.h"
#include "hal/hal_battery.h"
#include "middleware/sys_fault.h"
#include "power_manager.h"

#define BATTERY_VOLTAGE_STR_SIZE 8 /* ceiling for "0.00 V" style values */
/* Ignore ADC jitter below the displayed 2-decimal (10 mV) resolution. */
#define BATTERY_MILLIVOLTS_REDRAW_THRESHOLD 10

typedef struct {
    View view;
    BatteryChargeState charge_state;
    uint16_t millivolts;
} BatteryView;

static BatteryView battery_view;

static void battery_view_draw(Canvas* canvas, void* context) {
    BatteryView* view = (BatteryView*)context;

    elements_draw_header(canvas, "Battery");

    char voltage_str[BATTERY_VOLTAGE_STR_SIZE];
    snprintf(voltage_str, sizeof(voltage_str), "%u.%02u V", view->millivolts / 1000u, (view->millivolts % 1000u) / 10u);

    elements_draw_list_row(canvas, ELEMENTS_HEADER_HEIGHT, 0, false, hal_battery_charge_state_str(view->charge_state));

    elements_draw_list_row(canvas, ELEMENTS_HEADER_HEIGHT, 1, false, "Voltage:");
    elements_draw_str_right(canvas, (uint8_t)(ELEMENTS_HEADER_HEIGHT + ELEMENTS_LINE_HEIGHT), voltage_str);
}

static void battery_view_enter(void* context) {
    BatteryView* view = (BatteryView*)context;

    view->charge_state = hal_battery_get_charge_state();
    view->millivolts = hal_battery_get_millivolts();
}

static bool battery_view_tick(void* context) {
    BatteryView* view = (BatteryView*)context;

    BatteryChargeState charge_state = hal_battery_get_charge_state();
    uint16_t millivolts = hal_battery_get_millivolts();

    uint16_t diff = (millivolts > view->millivolts) ? (uint16_t)(millivolts - view->millivolts)
                                                    : (uint16_t)(view->millivolts - millivolts);

    if (charge_state == view->charge_state && diff < BATTERY_MILLIVOLTS_REDRAW_THRESHOLD) return false;

    view->charge_state = charge_state;
    view->millivolts = millivolts;
    return true;
}

void battery_view_init(ViewManager* view_manager) {
    ABORT_IF(view_manager == NULL);

    view_set_draw_callback(&battery_view.view, battery_view_draw);
    view_set_enter_callback(&battery_view.view, battery_view_enter);
    view_set_tick_callback(&battery_view.view, battery_view_tick);
    view_set_context(&battery_view.view, &battery_view);

    view_manager_add_view(view_manager, VIEW_ID_BATTERY, &battery_view.view);
}
