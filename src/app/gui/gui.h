#pragma once

#include <stdint.h>

#include "gui/view_manager.h"

#define GUI_TICK_PERIOD_MS 500

/* Builds the view manager and main menu, and spawns the GUI task. */
void gui_init();

/* GUI task entry point: drains input, ticks and redraws the active view. */
void gui_task(void* pvParameters);

/* Returns the singleton view manager. */
ViewManager* gui_get_view_manager();