#include "power_manager.h"

#include "gui/gui.h"

void power_manager_init(void) {
    sleep_view_init(gui_get_view_manager());
    battery_view_init(gui_get_view_manager());
}
