#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "canvas.h"
#include "middleware/input.h"
#include "view.h"
#include "view_ids.h"

#define VIEW_STACK_MAX 6

typedef struct {
    Canvas canvas;
    View* views[VIEW_ID_COUNT];
    uint32_t stack[VIEW_STACK_MAX];
    uint8_t depth;
    bool needs_redraw;
} ViewManager;

/* Initializes the view manager and binds it to a u8g2 display instance. */
void view_manager_init(ViewManager* vm, u8g2_t* u8g2);

/* Registers a view under the given id. */
void view_manager_add_view(ViewManager* vm, uint32_t id, View* view);

/* Pushes a view onto the stack, making it active. */
void view_manager_push(ViewManager* vm, uint32_t id);

/* Pops the active view, returning to the previous one. */
void view_manager_pop(ViewManager* vm);

/* Route an event to the active view. */
void view_manager_input(ViewManager* vm, const InputEvent* event);

/* Ticks the active view; returns true if it needs a redraw. */
bool view_manager_tick(ViewManager* vm);

/* Clear -> draw active view -> send. Clears the needs_redraw flag. */
void view_manager_draw(ViewManager* vm);

/* Replaces the top of the stack with `id`; the replaced view is not returned to. */
void view_manager_replace(ViewManager* vm, uint32_t id);

/* Clears the stack down to `id` as the sole (root) view. */
void view_manager_reset(ViewManager* vm, uint32_t id);

/* Pops the stack until `id` is on top. Aborts if `id` isn't on the stack. */
void view_manager_pop_to(ViewManager* vm, uint32_t id);

/* For redraws not triggered by input (e.g. a worker pushed new data). */
bool view_manager_needs_redraw(ViewManager* vm);

/* Marks the active view as needing a redraw. */
void view_manager_request_redraw(ViewManager* vm);

/* Checks if `id` is currently on the stack. */
bool view_manager_stack_contains(ViewManager* vm, uint32_t id);
