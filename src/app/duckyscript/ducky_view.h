#pragma once

#include "ducky.h"
#include "ducky_settings_view.h"
#include "gui/modules/menu.h"
#include "gui/view.h"
#include "gui/view_manager.h"
#include "hid/hid_transport.h"
#include "payload.h"

#define DUCKY_MENU_ITEM_COUNT 2

typedef struct {
    Menu ducky_menu;  // must be first (VIEW_ID_DUCKY: Play/Settings submenu)
    MenuItem ducky_menu_items[DUCKY_MENU_ITEM_COUNT];
    Menu payload_menu;  // VIEW_ID_DUCKY_PAYLOADS: dynamic payload list
    ViewManager* view_manager;
    View transport_view;  // VIEW_ID_DUCKY_TRANSPORT: gates entry to play on transport readiness
    View play_view;
    PayloadList payload_list;
    DuckySettingsView ducky_settings_view;
    DuckySettings* settings;

    HidStatus last_status;          // last shown transport status (transport tick redraw debounce)
    DuckyStatus last_ducky_status;  // last shown play status (play tick redraw debounce)

    char selected_payload[DUCKY_MAX_PAYLOAD_FNAME_LEN + 1];
} DuckyView;

void ducky_view_init(DuckyView* ducky_view, DuckySettings* settings, ViewManager* view_manager);
