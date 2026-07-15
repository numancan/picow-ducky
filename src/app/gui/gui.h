#pragma once

#include <stdint.h>

#include "gui/view_manager.h"

void gui_init();
void gui_task(void* pvParameters);

ViewManager* gui_get_view_manager();