#include "gui/view.h"
#include "gui/view_ids.h"
#include "middleware/sleep_manager.h"
#include "middleware/sys_fault.h"
#include "power_manager.h"

typedef struct {
    View view;
} SleepView;

static SleepView sleep_view;

static void sleep_view_draw(Canvas* canvas, void* context) {
    (void)context;

    canvas_set_font(canvas, CanvasFontPrimary);

    const char* text = "Sleeping";
    uint8_t x = (uint8_t)((CANVAS_WIDTH - canvas_string_width(canvas, text)) / 2);
    uint8_t y = (uint8_t)((CANVAS_HEIGHT + canvas_font_ascent(canvas)) / 2);
    canvas_draw_str(canvas, x, y, text);
}

static void sleep_view_enter(void* context) {
    (void)context;
    sleep_manager_request_sleep();
}

void sleep_view_init(ViewManager* view_manager) {
    ABORT_IF(view_manager == NULL);

    view_set_draw_callback(&sleep_view.view, sleep_view_draw);
    view_set_enter_callback(&sleep_view.view, sleep_view_enter);
    view_set_context(&sleep_view.view, &sleep_view);

    view_manager_add_view(view_manager, VIEW_ID_SLEEP, &sleep_view.view);
}
