#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "u8g2.h"
#include "view.h"

// View Manager structure
struct ViewManager {
  u8g2_t *u8g2;
  View *current_view;
};

// View Manager Functions
void view_manager_init(ViewManager *view_manager, u8g2_t *u8g2_inst, View *root_view);
void view_manager_draw(ViewManager *view_manager);
void view_manager_input(ViewManager *view_manager, InputEvent *event);
void view_manager_change_view(ViewManager *view_manager, View *new_view);

