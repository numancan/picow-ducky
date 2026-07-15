#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "canvas.h"
#include "input.h"

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

void view_set_enter_callback(View* view, ViewCallback callback);
void view_set_exit_callback(View* view, ViewCallback callback);
void view_set_draw_callback(View* view, ViewDrawCallback callback);
void view_set_tick_callback(View* view, ViewTickCallback callback);
void view_set_input_callback(View* view, ViewInputCallback callback);
void view_set_context(View* view, void* context);
