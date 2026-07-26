#include "menu.h"

#include "../canvas.h"
#include "../elements.h"
#include "middleware/sys_fault.h"

static uint16_t menu_count(Menu* menu) {
    if (menu->type == MENU_TYPE_DYNAMIC && menu->model.dynamic.get_count) {
        return menu->model.dynamic.get_count(menu->model.dynamic.context);
    }
    return menu->model.fixed.count;
}

static void menu_clamp(Menu* menu, uint16_t count) {
    if (count == 0) {
        menu->selected = 0;
        return;
    }
    if (menu->selected >= count) menu->selected = count - 1;
}

static const char* menu_get_item_label(void* context, uint16_t index) {
    Menu* menu = (Menu*)context;
    if (menu->type == MENU_TYPE_FIXED) {
        return menu->model.fixed.items[index].label;
    }
    return menu->model.dynamic.get_item(menu->model.dynamic.context, index).label;
}

static void menu_draw(Canvas* canvas, void* context) {
    Menu* menu = (Menu*)context;

    uint16_t count = menu_count(menu);
    menu_clamp(menu, count);

    uint8_t y_top = 0;
    if (menu->title[0] != '\0') {
        elements_draw_header(canvas, menu->title);
        y_top = ELEMENTS_HEADER_HEIGHT;
    }
    elements_draw_list(canvas, y_top, (CANVAS_HEIGHT - y_top), menu->selected, count, menu_get_item_label, menu);
}

static bool menu_input(const InputEvent* event, void* context) {
    Menu* menu = (Menu*)context;

    uint16_t count = menu_count(menu);
    menu_clamp(menu, count);
    if (count == 0) return false;

    switch (event->key) {
        case INPUT_KEY_DOWN:
            if (event->type == INPUT_EVENT_TYPE_PRESS || event->type == INPUT_EVENT_TYPE_REPEAT) {
                menu->selected = (menu->selected + 1) % count;
                return true;
            }
            return false;

        case INPUT_KEY_SELECT:
            if (event->type == INPUT_EVENT_TYPE_PRESS) {
                MenuItem item = (menu->type == MENU_TYPE_FIXED)
                                    ? menu->model.fixed.items[menu->selected]
                                    : menu->model.dynamic.get_item(menu->model.dynamic.context, menu->selected);
                if (item.callback) item.callback(item.callback_context);
                return true;
            }
            return false;

        default: return false;
    }
}

static void menu_init_common(Menu* menu, const char* title) {
    *menu = (Menu){0};
    menu->title = title;
    view_set_draw_callback(&menu->view, menu_draw);
    view_set_input_callback(&menu->view, menu_input);
    view_set_context(&menu->view, menu);
}

void menu_init_fixed(Menu* menu, const char* title, MenuItem* buffer, uint16_t capacity) {
    ABORT_IF(menu == NULL || buffer == NULL || capacity == 0);

    menu_init_common(menu, title);
    menu->type = MENU_TYPE_FIXED;
    menu->model.fixed.items = buffer;
    menu->model.fixed.capacity = capacity;
}

void menu_init_dynamic(Menu* menu, const char* title, MenuGetItemCallback get_item, MenuGetCountCallback get_count,
                       void* context) {
    ABORT_IF(menu == NULL || get_item == NULL || get_count == NULL);

    menu_init_common(menu, title);
    menu->type = MENU_TYPE_DYNAMIC;
    menu->model.dynamic.get_item = get_item;
    menu->model.dynamic.get_count = get_count;
    menu->model.dynamic.context = context;
}

void menu_add_item(Menu* menu, const char* label, MenuItemCallback callback, void* callback_context) {
    ABORT_IF(menu->type != MENU_TYPE_FIXED);
    ABORT_IF(menu->model.fixed.count >= menu->model.fixed.capacity);

    menu->model.fixed.items[menu->model.fixed.count].label = label;
    menu->model.fixed.items[menu->model.fixed.count].callback = callback;
    menu->model.fixed.items[menu->model.fixed.count].callback_context = callback_context;
    menu->model.fixed.count++;
}

View* menu_get_view(Menu* menu) { return &menu->view; }