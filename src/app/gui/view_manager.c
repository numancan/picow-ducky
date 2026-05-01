#include "view_manager.h"

void view_manager_init(ViewManager* view_manager, u8g2_t* u8g2_inst, View* root_view) {
    view_manager->u8g2 = u8g2_inst;
    view_manager->current_view = root_view;
    if (view_manager->current_view && view_manager->current_view->on_enter) {
        view_manager->current_view->on_enter(view_manager->current_view);
    }
}

void view_manager_draw(ViewManager* view_manager) {
    if (view_manager->u8g2 && view_manager->current_view &&
        view_manager->current_view->draw) {
        view_manager->current_view->draw(view_manager->current_view, view_manager->u8g2);
    }
}

void view_manager_input(ViewManager* view_manager, InputEvent* event) {
    if (view_manager->current_view && view_manager->current_view->on_input) {
        view_manager->current_view->on_input(view_manager->current_view, event,
                                             view_manager);
    }
}

void view_manager_change_view(ViewManager* view_manager, View* new_view) {
    if (new_view && view_manager->current_view != new_view) {
        if (view_manager->current_view && view_manager->current_view->on_exit) {
            view_manager->current_view->on_exit(view_manager->current_view);
        }

        view_manager->current_view = new_view;

        if (view_manager->current_view && view_manager->current_view->on_enter) {
            view_manager->current_view->on_enter(view_manager->current_view);
        }
    }
}
