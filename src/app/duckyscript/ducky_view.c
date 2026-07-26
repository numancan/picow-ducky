#include "ducky_view.h"

#include <stdint.h>
#include <stdio.h>

#include "ducky.h"
#include "ducky_config.h"
#include "ducky_settings_view.h"  // also provides DuckySettings
#include "gui/elements.h"
#include "gui/modules/menu.h"
#include "gui/view_manager.h"
#include "middleware/fat_io.h"
#include "middleware/log.h"
#include "middleware/sys_fault.h"
#include "payload.h"

static const char* TAG = "DUCKY_VIEW";

static void play_menu_cb(void* context) {
    DuckyView* dv = (DuckyView*)context;
    view_manager_push(dv->view_manager, VIEW_ID_DUCKY_PAYLOADS);
}

static void settings_menu_cb(void* context) {
    DuckyView* dv = (DuckyView*)context;
    view_manager_push(dv->view_manager, VIEW_ID_DUCKY_SETTINGS);
}

static void play_payload_cb(void* context) {
    DuckyView* dv = (DuckyView*)context;
    // Copy the name out now: the list window cache is invalidated by the next payload_list_get.
    const char* name = payload_list_get(&dv->payload_list, dv->payload_menu.selected);
    snprintf(dv->selected_payload, sizeof(dv->selected_payload), "%s", name);
    view_manager_push(dv->view_manager, VIEW_ID_DUCKY_TRANSPORT);
}

static uint16_t ducky_get_count(void* context) {
    DuckyView* dv = (DuckyView*)context;
    // TODO: Maybe pop-up? and mounted check will be done by payload_list_init ?
    /* SD may eject mid-view; report 0 so the menu never indexes a stale list. */
    if (!fat_io_is_mounted()) return 0;
    return (uint16_t)payload_list_count(&dv->payload_list);
}

static MenuItem ducky_get_item(void* context, uint16_t index) {
    DuckyView* dv = (DuckyView*)context;
    // The label points into the list's window cache, valid only until the next
    // payload_list_get; the owner is passed as context so the callback can re-fetch by index.
    const char* name = payload_list_get(&dv->payload_list, index);
    return (MenuItem){.label = name, .callback = play_payload_cb, .callback_context = dv};
}

static void payload_view_enter(void* context) {
    /* The view ctx is the Menu*; recover the owner from its dynamic context. */
    Menu* menu = (Menu*)context;
    DuckyView* dv = (DuckyView*)menu->model.dynamic.context;

    ABORT_IF(!fat_io_is_mounted());

    menu->selected = 0;

    payload_list_init(&dv->payload_list);
}

/* Replace self with the play view once the transport is ready; this view is a
 * one-way gate and must not be revisited via back. */
static void ducky_transport_advance_if_ready(DuckyView* dv) {
    dv->last_status = hid_transport_status();
    if (dv->last_status == HID_STATUS_OK) {
        view_manager_replace(dv->view_manager, VIEW_ID_DUCKY_PLAY);
    }
}

static void ducky_transport_view_draw(Canvas* canvas, void* context) {
    DuckyView* dv = (DuckyView*)context;
    elements_draw_header(canvas, "Transport");
    elements_draw_str_right(canvas, 0, hid_transport_name(dv->settings->transporter));
    elements_draw_list_row(canvas, ELEMENTS_HEADER_HEIGHT, 0, false, hid_report_status_name(dv->last_status));
    elements_draw_list_row(canvas, ELEMENTS_HEADER_HEIGHT, 1, false, "Long press to menu");
}

static void ducky_transport_view_enter(void* context) {
    DuckyView* dv = (DuckyView*)context;

    ducky_transport_arm_request();
    ducky_transport_advance_if_ready(dv);
}

static bool ducky_transport_view_tick(void* context) {
    DuckyView* dv = (DuckyView*)context;
    HidStatus prev = dv->last_status;
    ducky_transport_advance_if_ready(dv);
    return dv->last_status != prev;
}

static bool ducky_play_is_done(DuckyStatus status) { return status == DUCKY_STATUS_DONE; }

static void ducky_play_view_draw(Canvas* canvas, void* context) {
    DuckyView* dv = (DuckyView*)context;
    DuckyStatus status = ducky_get_status();
    elements_draw_header(canvas, dv->selected_payload);
    elements_draw_list_row(canvas, ELEMENTS_HEADER_HEIGHT, 0, false, ducky_get_status_str(status));
    const char* action_str = ducky_play_is_done(status) ? "Long press to menu" : "Press to stop";
    elements_draw_list_row(canvas, ELEMENTS_HEADER_HEIGHT, 1, false, action_str);
}

static void ducky_play_view_enter(void* context) {
    DuckyView* dv = (DuckyView*)context;
    LOG_INFO(TAG, "Playing payload: %s", dv->selected_payload);
    dv->last_ducky_status = ducky_get_status();
    ducky_play_payload_request(dv->selected_payload);
}

static bool ducky_play_view_tick(void* context) {
    DuckyView* dv = (DuckyView*)context;
    DuckyStatus status = ducky_get_status();
    if (status == dv->last_ducky_status) return false;
    dv->last_ducky_status = status;
    return true;
}

static bool ducky_play_view_input(const InputEvent* event, void* context) {
    DuckyView* dv = (DuckyView*)context;

    if (ducky_get_status() == DUCKY_STATUS_PLAYING && event->type == INPUT_EVENT_TYPE_LONG_PRESS &&
        event->key == INPUT_KEY_DOWN) {
        ducky_stop_payload_request();
        return true;
    } else if (ducky_play_is_done(ducky_get_status()) && event->type == INPUT_EVENT_TYPE_LONG_PRESS &&
               event->key == INPUT_KEY_DOWN) {
        view_manager_pop_to(dv->view_manager, VIEW_ID_DUCKY);
        return true;
    }

    return true;
}

static void ducky_menu_view_exit(void* context) {
    DuckyView* dv = (DuckyView*)context;
    if (!view_manager_stack_contains(dv->view_manager, VIEW_ID_DUCKY)) {
        ducky_transport_disarm_request();
    }
}

void ducky_view_init(DuckyView* ducky_view, DuckySettings* settings, ViewManager* view_manager) {
    ABORT_IF(ducky_view == NULL || view_manager == NULL);

    ducky_view->view_manager = view_manager;
    ducky_view->settings = settings;

    menu_init_fixed(&ducky_view->ducky_menu, "Ducky", ducky_view->ducky_menu_items, DUCKY_MENU_ITEM_COUNT);
    menu_add_item(&ducky_view->ducky_menu, "Play", play_menu_cb, ducky_view);
    menu_add_item(&ducky_view->ducky_menu, "Settings", settings_menu_cb, ducky_view);
    view_set_exit_callback(menu_get_view(&ducky_view->ducky_menu), ducky_menu_view_exit);

    menu_init_dynamic(&ducky_view->payload_menu, "", ducky_get_item, ducky_get_count, ducky_view);
    view_set_enter_callback(menu_get_view(&ducky_view->payload_menu), payload_view_enter);

    view_set_draw_callback(&ducky_view->transport_view, ducky_transport_view_draw);
    view_set_enter_callback(&ducky_view->transport_view, ducky_transport_view_enter);
    view_set_tick_callback(&ducky_view->transport_view, ducky_transport_view_tick);
    view_set_context(&ducky_view->transport_view, ducky_view);

    view_set_draw_callback(&ducky_view->play_view, ducky_play_view_draw);
    view_set_input_callback(&ducky_view->play_view, ducky_play_view_input);
    view_set_enter_callback(&ducky_view->play_view, ducky_play_view_enter);
    view_set_tick_callback(&ducky_view->play_view, ducky_play_view_tick);
    view_set_context(&ducky_view->play_view, ducky_view);

    view_manager_add_view(view_manager, VIEW_ID_DUCKY, menu_get_view(&ducky_view->ducky_menu));
    view_manager_add_view(view_manager, VIEW_ID_DUCKY_PAYLOADS, menu_get_view(&ducky_view->payload_menu));
    view_manager_add_view(view_manager, VIEW_ID_DUCKY_TRANSPORT, &ducky_view->transport_view);
    view_manager_add_view(view_manager, VIEW_ID_DUCKY_PLAY, &ducky_view->play_view);

    ducky_settings_view_init(&ducky_view->ducky_settings_view, view_manager, settings);
}
