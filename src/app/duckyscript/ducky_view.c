#include "ducky_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/gui/gui.h"
#include "app/gui/menu_view.h"
#include "app/gui/ui_common.h"
#include "app/gui/view_manager.h"
#include "ducky.h"
#include "middleware/sd_card.h"

static char** payload_list = NULL;
static MenuItem* ducky_items = NULL;
uint16_t payload_list_count = 0;

static char selected_payload[64] = "";

static Menu ducky_menu = {
    .title = "Payloads",
    .items = NULL,
    .item_count = 0,
};

static View ducky_play_view;

static void ducky_view_enter(View* view) {
    char payload_single_string[(20 * 24) + 24 + 1];
    payload_list_count = sd_card_list_dir("/payloads", payload_single_string,
                                          sizeof(payload_single_string), " ");

    if (payload_list_count < 1) {
        ducky_menu.item_count = 1;
        ducky_items = malloc(sizeof(MenuItem) * 1);
        ducky_items[0] = (MenuItem)MENU_ITEM_BACK("< Back");
        ducky_menu.items = ducky_items;
        return;
    }

    payload_list = malloc(sizeof(char*) * payload_list_count);
    ducky_items = malloc(sizeof(MenuItem) * (payload_list_count + 1));

    char* payload_name = strtok(payload_single_string, " ");

    for (int i = 0; i < payload_list_count; i++) {
        payload_list[i] = malloc(strlen(payload_name) + 1);
        snprintf(payload_list[i], strlen(payload_name) + 1, "%s", payload_name);

        ducky_items[i] = (MenuItem)MENU_ITEM_SUBMENU(payload_list[i], &ducky_play_view);

        payload_name = strtok(NULL, " ");
    }

    // Add Back button at the end
    ducky_items[payload_list_count] = (MenuItem)MENU_ITEM_BACK("< Back");

    ducky_menu.item_count = payload_list_count + 1;
    ducky_menu.items = ducky_items;

    // Check bounds if we were previously selecting an item
    if (ducky_menu.selected_index >= ducky_menu.item_count) {
        ducky_menu.selected_index = 0;
        ducky_menu.scroll_offset = 0;
    }
}

static void ducky_view_exit(View* view) {
    if (ducky_menu.item_count > 0 && ducky_menu.selected_index < payload_list_count) {
        snprintf(selected_payload, sizeof(selected_payload), "%s",
                 payload_list[ducky_menu.selected_index]);
    }

    if (ducky_items) {
        free(ducky_items);
        ducky_items = NULL;
    }

    ducky_menu.items = NULL;
    if (!payload_list) return;
    for (int i = 0; i < payload_list_count; i++) free(payload_list[i]);
    free(payload_list);
    payload_list = NULL;
    payload_list_count = 0;
}

static void ducky_play_view_draw(View* view, u8g2_t* u8g2) {
    u8g2_ClearBuffer(u8g2);
    ui_draw_header(u8g2, "Playing Payload");

    u8g2_SetDrawColor(u8g2, 1);
    u8g2_SetFont(u8g2, u8g2_font_profont11_tr);
    u8g2_DrawStr(u8g2, 10, 26, selected_payload);

    u8g2_SendBuffer(u8g2);
}

static void ducky_play_view_enter(View* view) {
    printf("Playing payload: %s\n", selected_payload);
    ducky_play_script(selected_payload);
}

static void ducky_play_view_on_input(View* view, InputEvent* event,
                                     ViewManager* view_manager) {
    if (event->key == INPUT_KEY_SELECT || event->key == INPUT_KEY_DOWN) {
        if (event->type == INPUT_EVENT_TYPE_PRESS) {
            view_manager_change_view(view_manager, view->parent);
        }
    }
}

void ducky_view_init(View* view, View* parent) {
    menu_view_init(view, &ducky_menu, parent);

    view->on_enter = ducky_view_enter;
    view->on_exit = ducky_view_exit;

    // Setup ducky_play_view
    ducky_play_view.draw = ducky_play_view_draw;
    ducky_play_view.on_enter = ducky_play_view_enter;
    ducky_play_view.on_exit = NULL;
    ducky_play_view.on_input = ducky_play_view_on_input;
    ducky_play_view.context = NULL;
    ducky_play_view.parent = view;
}