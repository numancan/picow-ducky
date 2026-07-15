#include "view.h"

#include "middleware/sys_fault.h"

void view_set_enter_callback(View* view, ViewCallback callback) {
    ABORT_IF(!view);
    view->on_enter = callback;
}

void view_set_exit_callback(View* view, ViewCallback callback) {
    ABORT_IF(!view);
    view->on_exit = callback;
}

void view_set_draw_callback(View* view, ViewDrawCallback callback) {
    ABORT_IF(!view);
    view->draw = callback;
}

void view_set_tick_callback(View* view, ViewTickCallback callback) {
    ABORT_IF(!view);
    view->tick = callback;
}

void view_set_input_callback(View* view, ViewInputCallback callback) {
    ABORT_IF(!view);
    view->input = callback;
}

void view_set_context(View* view, void* context) {
    ABORT_IF(!view);
    view->context = context;
}
