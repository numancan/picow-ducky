#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "canvas.h"
#include "input.h"
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

void view_manager_init(ViewManager* vm, u8g2_t* u8g2);

void view_manager_add_view(ViewManager* vm, uint32_t id, View* view);
void view_manager_push(ViewManager* vm, uint32_t id);
void view_manager_pop(ViewManager* vm);

/* Route an event to the active view. */
void view_manager_input(ViewManager* vm, const InputEvent* event);

bool view_manager_tick(ViewManager* vm);

/* Clear -> draw active view -> send. Clears the needs_redraw flag. */
void view_manager_draw(ViewManager* vm);

/* Tepedeki view'ı `id` ile değiştir; eskisine geri dönülemez.
   Altındaki stack korunur, derinlik aynı kalır. */
void view_manager_replace(ViewManager* vm, uint32_t id);

/* Tüm stack'i temizle, `id`'yi tek (kök) view yap.
   Bundan sonra bir şey push edilene kadar pop() hiçbir şey yapmaz. */
void view_manager_reset(ViewManager* vm, uint32_t id);

/* `id` tepeye gelene kadar stack'i kes. id stack'te yoksa ABORT. */
void view_manager_pop_to(ViewManager* vm, uint32_t id);

/* For redraws not triggered by input (e.g. a worker pushed new data). */
bool view_manager_needs_redraw(ViewManager* vm);
void view_manager_request_redraw(ViewManager* vm);
