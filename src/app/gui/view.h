#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "canvas.h"
#include "middleware/input.h"

typedef void (*ViewCallback)(void* context);
typedef void (*ViewDrawCallback)(Canvas* canvas, void* context);
typedef bool (*ViewInputCallback)(const InputEvent* event, void* context);
typedef bool (*ViewTickCallback)(void* context);

typedef struct View {
    ViewCallback on_enter;
    ViewCallback on_exit;
    ViewDrawCallback draw;
    ViewInputCallback input;
    ViewTickCallback tick;
    void* context;
} View;

/* Sets the callback invoked when the view becomes active. */
void view_set_enter_callback(View* view, ViewCallback callback);

/* Sets the callback invoked when the view is left. */
void view_set_exit_callback(View* view, ViewCallback callback);

/* Sets the callback that renders the view. */
void view_set_draw_callback(View* view, ViewDrawCallback callback);

/* Sets the callback invoked on each tick. */
void view_set_tick_callback(View* view, ViewTickCallback callback);

/* Sets the callback that handles input events. */
void view_set_input_callback(View* view, ViewInputCallback callback);

/* Sets the opaque context passed to the view's callbacks. */
void view_set_context(View* view, void* context);
