#include "menu_view.h"

#include <stdio.h>
#include <string.h>

#include "hal/hal.h"
#include "ui_common.h"
#include "view_manager.h"

static const char dm_charset[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789 "
    "!@#$%^&*()_+-=[]{};':\",./<>?\\|";

// Internal callbacks
static void menu_view_draw(View* view, u8g2_t* u8g2);
static void menu_view_on_input(View* view, InputEvent* event, ViewManager* view_manager);

void menu_view_init(View* view, Menu* menu, View* parent) {
    view->draw = menu_view_draw;
    view->on_enter = NULL;
    view->on_exit = NULL;
    view->on_input = menu_view_on_input;
    view->context = menu;
    view->parent = parent;

    if (menu) {
        menu->selected_index = 0;
        menu->scroll_offset = 0;
        menu->is_input_mode = false;
        menu->input_cursor_pos = 0;
        menu->input_char_index = 0;
    }
}

static void menu_view_draw(View* view, u8g2_t* u8g2) {
    Menu* menu = (Menu*)view->context;
    if (!menu) return;

    u8g2_ClearBuffer(u8g2);
    u8g2_SetDrawColor(u8g2, 1);

    if (menu->is_input_mode) {
        MenuItem* item = &menu->items[menu->selected_index];

        // 1. Draw Header with prompt text
        ui_draw_header(u8g2, item->text);

        // 2. Prepare content
        u8g2_SetFont(u8g2, u8g2_font_profont12_tr);

        char temp_str[64];
        if (menu->input_cursor_pos > 0) {
            strncpy(temp_str, item->input_buffer, menu->input_cursor_pos);
            temp_str[menu->input_cursor_pos] = '\0';
        } else {
            temp_str[0] = '\0';
        }

        int text_w = u8g2_GetStrWidth(u8g2, temp_str);
        int offset_x = 2;

        // Scroll text if too long
        // if (text_w > 100) offset_x = 100 - text_w;

        // Draw existing text
        if (text_w > 0) {
            u8g2_DrawStr(u8g2, offset_x, 26, temp_str);
        }

        // Draw cursor character
        char current_char = dm_charset[menu->input_char_index];
        char char_str[2];
        char_str[0] = current_char;
        char_str[1] = '\0';

        int char_w = u8g2_GetStrWidth(u8g2, char_str);
        int cursor_x = text_w + 2;
        if (cursor_x < 2) cursor_x = 2;

        // Invert cursor: white box, black text
        u8g2_DrawBox(u8g2, cursor_x - 1, 26 - 9, char_w + 2, 11);
        u8g2_SetDrawColor(u8g2, 0);
        u8g2_DrawStr(u8g2, cursor_x, 26, char_str);
        u8g2_SetDrawColor(u8g2, 1);  // Reset back to white

        u8g2_SendBuffer(u8g2);
        return;
    }

    // 1. Draw Header
    ui_draw_header(u8g2, menu->title);

    // 2. Draw Menu Items
    int y_offset = UI_HEADER_HEIGHT;
    for (int i = 0; i < DM_VISIBLE_ITEMS; i++) {
        int item_index = menu->scroll_offset + i;
        if (item_index >= menu->item_count) break;

        MenuItem* item = &menu->items[item_index];
        int y_pos = y_offset + (i * UI_LINE_HEIGHT);

        if (item_index == menu->selected_index) {
            u8g2_DrawDisc(u8g2, 4, y_pos + (UI_LINE_HEIGHT / 2) + 1, 2, U8G2_DRAW_ALL);
        }

        if (item->type == ITEM_TYPE_INPUT) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%s: %s", item->text,
                     item->input_buffer ? item->input_buffer : "");
            buf[20] = '\0';
            u8g2_DrawStr(u8g2, 10, y_pos + UI_LINE_HEIGHT - 1, buf);
        } else if (item->type == ITEM_TYPE_TOGGLE) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%s: %s", item->text,
                     (item->state && *(item->state)) ? "ON" : "OFF");
            buf[20] = '\0';
            u8g2_DrawStr(u8g2, 10, y_pos + UI_LINE_HEIGHT - 1, buf);
        } else {
            u8g2_DrawStr(u8g2, 10, y_pos + UI_LINE_HEIGHT - 1, item->text);
            if (item->type == ITEM_TYPE_SUBMENU) {
                u8g2_DrawStr(u8g2, 115, y_pos + UI_LINE_HEIGHT - 1, ">");
            } else if (item->type == ITEM_TYPE_BACK) {
                u8g2_DrawStr(u8g2, 2, y_pos + UI_LINE_HEIGHT - 1, "<");
            }
        }
    }

    // 3. Draw Scrollbar
    if (menu->item_count > DM_VISIBLE_ITEMS) {
        int bar_height =
            (DM_VISIBLE_ITEMS * (DM_VISIBLE_ITEMS * UI_LINE_HEIGHT)) / menu->item_count;
        if (bar_height < 2) bar_height = 2;
        int bar_y = UI_HEADER_HEIGHT +
                    ((menu->scroll_offset * (DM_VISIBLE_ITEMS * UI_LINE_HEIGHT)) /
                     menu->item_count);
        u8g2_DrawBox(u8g2, 126, bar_y, 2, bar_height);
    }

    u8g2_SendBuffer(u8g2);
}

static void menu_view_on_input(View* view, InputEvent* event, ViewManager* view_manager) {
    Menu* menu = (Menu*)view->context;
    if (!menu) return;

    if (menu->is_input_mode) {
        MenuItem* item = &menu->items[menu->selected_index];

        if (event->key == INPUT_KEY_DOWN) {
            // DOWN button
            if (event->type == INPUT_EVENT_TYPE_PRESS) {
                // Change character
                menu->input_char_index++;
                if (dm_charset[menu->input_char_index] == '\0') {
                    menu->input_char_index = 0;
                }
            } else if (event->type == INPUT_EVENT_TYPE_LONG_PRESS) {
                // Exit input mode
                menu->is_input_mode = false;
            }
        } else if (event->key == INPUT_KEY_SELECT) {
            // SELECT button
            if (event->type == INPUT_EVENT_TYPE_PRESS) {
                // Select character
                char current_char = dm_charset[menu->input_char_index];
                if (menu->input_cursor_pos < item->input_max_len - 1) {
                    item->input_buffer[menu->input_cursor_pos] = current_char;
                    menu->input_cursor_pos++;
                    item->input_buffer[menu->input_cursor_pos] = '\0';
                    menu->input_char_index = 0;  // Reset for next character
                }
            } else if (event->type == INPUT_EVENT_TYPE_LONG_PRESS) {
                // Delete character (backspace)
                if (menu->input_cursor_pos > 0) {
                    menu->input_cursor_pos--;
                    item->input_buffer[menu->input_cursor_pos] = '\0';
                }
            }
        }
        return;
    }

    // List mode handling
    switch (event->key) {
        case INPUT_KEY_DOWN: {
            if (event->type != INPUT_EVENT_TYPE_PRESS) break;

            if (menu->selected_index < menu->item_count - 1) {
                menu->selected_index++;
                if (menu->selected_index >= menu->scroll_offset + DM_VISIBLE_ITEMS) {
                    menu->scroll_offset++;
                }
            } else {
                menu->selected_index = 0;
                menu->scroll_offset = 0;
            }
            break;
        }

        case INPUT_KEY_SELECT: {
            if (event->type != INPUT_EVENT_TYPE_PRESS) break;
            if (menu->item_count == 0) return;

            MenuItem* item = &menu->items[menu->selected_index];

            if (item->type == ITEM_TYPE_ACTION && item->action) {
                item->action();
            } else if (item->type == ITEM_TYPE_TOGGLE && item->state != NULL) {
                *(item->state) = !(*(item->state));
                if (item->action) item->action();
            } else if (item->type == ITEM_TYPE_INPUT && item->input_buffer != NULL) {
                menu->is_input_mode = true;
                int len = strlen(item->input_buffer);
                if (len >= item->input_max_len) len = item->input_max_len - 1;
                menu->input_cursor_pos = len;
                menu->input_char_index = 0;
                if (item->action) item->action();  // CHECK: maybe not needed
            } else if (item->type == ITEM_TYPE_SUBMENU && item->sub_view != NULL) {
                menu->selected_index = 0;
                menu->scroll_offset = 0;
                view_manager_change_view(view_manager, item->sub_view);
            } else if (item->type == ITEM_TYPE_BACK && view->parent != NULL) {
                menu->selected_index = 0;
                menu->scroll_offset = 0;
                view_manager_change_view(view_manager, view->parent);
            }
            break;
        }

        default:
            break;
    }
}
