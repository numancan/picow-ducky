#include "ducky_settings_view.h"

#include <stdint.h>

#include "ducky_settings.h"
#include "gui/canvas.h"
#include "gui/elements.h"
#include "gui/view_ids.h"
#include "middleware/log.h"
#include "middleware/sys_fault.h"

static const char* TAG = "DUCKY_SETTINGS";

#define SETTINGS_VALUE_MAX 16  // max value string length ("2000", "US-Q", ...)

static void settings_view_draw(Canvas* canvas, void* context) {
    DuckySettingsView* sv = (DuckySettingsView*)context;

    elements_draw_header(canvas, "Ducky Settings");
    uint8_t y_top = ELEMENTS_HEADER_HEIGHT;
    uint8_t height = (uint8_t)(CANVAS_HEIGHT - y_top);
    uint16_t visible = elements_list_visible_rows(height);
    uint16_t window = (sv->selected < visible) ? 0 : (uint16_t)(sv->selected - visible + 1);

    char value_buf[SETTINGS_VALUE_MAX];

    for (uint16_t row = 0; row < visible; row++) {
        uint16_t idx = window + row;
        if (idx >= DUCKY_SETTINGS_ITEM_COUNT) break;

        elements_draw_list_row(canvas, y_top, row, idx == sv->selected, ducky_settings_label((DuckySettingsItem)idx));

        ducky_settings_format(sv->settings, (DuckySettingsItem)idx, value_buf, sizeof(value_buf));
        elements_draw_str_right(canvas, y_top + (row * ELEMENTS_LINE_HEIGHT), value_buf);
    }

    elements_scrollbar(canvas, y_top, window, visible, DUCKY_SETTINGS_ITEM_COUNT);
}

static bool settings_view_input(const InputEvent* event, void* context) {
    DuckySettingsView* sv = (DuckySettingsView*)context;

    switch (event->key) {
        case INPUT_KEY_DOWN:
            if (event->type == INPUT_EVENT_TYPE_PRESS || event->type == INPUT_EVENT_TYPE_REPEAT) {
                sv->selected = (uint8_t)((sv->selected + 1u) % DUCKY_SETTINGS_ITEM_COUNT);
                return true;
            }
            return false;

        case INPUT_KEY_SELECT:
            if (event->type == INPUT_EVENT_TYPE_PRESS || event->type == INPUT_EVENT_TYPE_REPEAT) {
                ducky_settings_next(sv->settings, (DuckySettingsItem)sv->selected);
                sv->dirty = true;
                return true;
            }
            return false;

        default: return false;  // LONG press falls through to the view manager's global back
    }
}

static void settings_view_enter(void* context) {
    DuckySettingsView* sv = (DuckySettingsView*)context;
    sv->selected = 0;
}

static void settings_view_exit(void* context) {
    DuckySettingsView* sv = (DuckySettingsView*)context;
    if (!sv->dirty) return;

    if (ducky_settings_save(sv->settings)) {
        sv->dirty = false;
    } else {
        LOG_WARN(TAG, "settings save failed");
    }
}

void ducky_settings_view_init(DuckySettingsView* ducky_settings_view, ViewManager* vm, DuckySettings* settings) {
    ABORT_IF(ducky_settings_view == NULL || vm == NULL || settings == NULL);

    *ducky_settings_view = (DuckySettingsView){0};
    ducky_settings_view->settings = settings;

    view_set_draw_callback(&ducky_settings_view->view, settings_view_draw);
    view_set_input_callback(&ducky_settings_view->view, settings_view_input);
    view_set_enter_callback(&ducky_settings_view->view, settings_view_enter);
    view_set_exit_callback(&ducky_settings_view->view, settings_view_exit);
    view_set_context(&ducky_settings_view->view, ducky_settings_view);

    view_manager_add_view(vm, VIEW_ID_DUCKY_SETTINGS, &ducky_settings_view->view);
}