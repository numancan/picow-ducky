#include "view_manager.h"

#include <stdint.h>

#include "hal/hal.h"
#include "middleware/input.h"
#include "middleware/sys_fault.h"

void view_manager_init(ViewManager* vm, u8g2_t* u8g2) {
    canvas_init(&vm->canvas, u8g2);
    for (int i = 0; i < VIEW_ID_COUNT; i++) {
        vm->views[i] = NULL;
    }
    vm->depth = 0;
    vm->needs_redraw = true; /* force first draw */
}

void view_manager_add_view(ViewManager* vm, uint32_t id, View* view) {
    ABORT_IF(id >= VIEW_ID_COUNT);
    ABORT_IF(view == NULL);
    ABORT_IF(vm->views[id] != NULL); /* eager setup registers each id exactly once */
    vm->views[id] = view;
}

/* Current view = stack top, or NULL if the stack is empty. */
static View* view_manager_current(ViewManager* vm) {
    if (vm->depth == 0) return NULL;
    return vm->views[vm->stack[vm->depth - 1]];
}

void view_manager_push(ViewManager* vm, uint32_t id) {
    ABORT_IF(id >= VIEW_ID_COUNT);
    ABORT_IF(vm->views[id] == NULL);       /* target must be registered before any push */
    ABORT_IF(vm->depth >= VIEW_STACK_MAX); /* nesting too deep */

    View* old = view_manager_current(vm);
    if (old && old->on_exit) old->on_exit(old->context);

    vm->stack[vm->depth++] = id;

    View* next = vm->views[id];
    if (next->on_enter) next->on_enter(next->context);

    vm->needs_redraw = true;
}

void view_manager_pop(ViewManager* vm) {
    if (vm->depth <= 1) return;

    View* old = vm->views[vm->stack[--vm->depth]];
    if (old && old->on_exit) old->on_exit(old->context);

    View* prev = view_manager_current(vm);
    if (prev && prev->on_enter) prev->on_enter(prev->context);

    vm->needs_redraw = true;
}

void view_manager_input(ViewManager* vm, const InputEvent* event) {
    View* view = view_manager_current(vm);
    if (view == NULL) return;

    if (view->input && view->input(event, view->context)) {
        vm->needs_redraw = true;
        return;
    }

    if (event->key == INPUT_KEY_DOWN &&
        (event->type == INPUT_EVENT_TYPE_LONG_PRESS || event->type == INPUT_EVENT_TYPE_REPEAT)) {
        view_manager_pop(vm);
    }
}

bool view_manager_tick(ViewManager* vm) {
    View* view = view_manager_current(vm);
    if (view && view->tick) {
        return view->tick(view->context);
    }
    return false;
}

void view_manager_draw(ViewManager* vm) {
    canvas_clear(&vm->canvas);

    View* view = view_manager_current(vm);
    if (view && view->draw) {
        view->draw(&vm->canvas, view->context);
    }

    canvas_send(&vm->canvas);
    vm->needs_redraw = false;
}

void view_manager_reset(ViewManager* vm, uint32_t id) {
    ABORT_IF(id >= VIEW_ID_COUNT);
    ABORT_IF(vm->views[id] == NULL);

    View* old = view_manager_current(vm);
    if (old && old->on_exit) old->on_exit(old->context);

    vm->depth = 0;
    vm->stack[vm->depth++] = id; /* stack[0] = id, depth = 1 */

    View* next = vm->views[id];
    if (next->on_enter) next->on_enter(next->context);

    vm->needs_redraw = true;
}

void view_manager_replace(ViewManager* vm, uint32_t id) {
    ABORT_IF(id >= VIEW_ID_COUNT);
    ABORT_IF(vm->views[id] == NULL);
    ABORT_IF(vm->depth == 0); /* use push for the first view */

    View* old = view_manager_current(vm);
    if (old && old->on_exit) old->on_exit(old->context);

    vm->stack[vm->depth - 1] = id;

    View* next = vm->views[id];
    if (next->on_enter) next->on_enter(next->context);

    vm->needs_redraw = true;
}

void view_manager_pop_to(ViewManager* vm, uint32_t id) {
    ABORT_IF(id >= VIEW_ID_COUNT);

    int target = -1;
    for (int i = (int)vm->depth - 1; i >= 0; i--) {
        if (vm->stack[i] == id) {
            target = i;
            break;
        }
    }
    ABORT_IF(target < 0);                     /* id not on the stack */
    if (target == (int)vm->depth - 1) return; /* already on top */

    View* old = view_manager_current(vm);
    if (old && old->on_exit) old->on_exit(old->context);

    vm->depth = (uint8_t)(target + 1); /* cut the stack so id ends up on top */

    View* next = vm->views[id];
    if (next->on_enter) next->on_enter(next->context);

    vm->needs_redraw = true;
}

bool view_manager_needs_redraw(ViewManager* vm) { return vm->needs_redraw; }
void view_manager_request_redraw(ViewManager* vm) { vm->needs_redraw = true; }

bool view_manager_stack_contains(ViewManager* vm, uint32_t id) {
    for (uint32_t i = 0; i < vm->depth; i++) {
        if (vm->stack[i] == id) return true;
    }
    return false;
}