#include "net_manager_view.h"

#include "gui/elements.h"
#include "gui/view_ids.h"
#include "hal/hal.h"
#include "middleware/sys_fault.h"

#define NET_INFO_TOGGLE_TICKS 4 /* 4 * GUI_TICK_PERIOD_MS (500ms) = 2s per item */
#define NET_MDNS_HOST DEVICE_NAME ".local"

/* Connected info line rotates through these on the 2-line screen. */
typedef enum {
    NET_INFO_IP,
    NET_INFO_MDNS,
    NET_INFO_HINT,
    NET_INFO_COUNT,
} NetInfoLine;

static const char* net_manager_view_conn_info(const NetManagerView* view) {
    switch ((NetInfoLine)view->info_index) {
        case NET_INFO_IP: return view->ip_str;
        case NET_INFO_MDNS: return NET_MDNS_HOST;
        case NET_INFO_HINT: return "Hold to stop";
        default: return "";
    }
}

/* Re-arms the connected info cycle at the IP item and caches the current STA IP. */
static void net_manager_view_reset_info(NetManagerView* view) {
    view->info_tick = 0;
    view->info_index = NET_INFO_IP;
    if (!net_manager_get_ip_str(view->ip_str, sizeof view->ip_str)) view->ip_str[0] = '\0';
}

static void net_manager_view_draw(Canvas* canvas, void* context) {
    NetManagerView* view = (NetManagerView*)context;

    elements_draw_header(canvas, "Net Manager");

    const char* status_str;
    const char* action_str;
    switch (view->phase) {
        case NET_MANAGER_VIEW_PHASE_RUNNING:
            status_str = net_manager_get_status_str(view->last_state);
            action_str = (view->last_state == NET_STATE_CONNECTED) ? net_manager_view_conn_info(view) : "Hold to stop";
            break;
        case NET_MANAGER_VIEW_PHASE_STOPPING:
            status_str = "Stopping";
            action_str = "";
            break;
        case NET_MANAGER_VIEW_PHASE_IDLE:
        default:
            status_str = net_manager_get_status_str(NET_STATE_IDLE);
            action_str = "Press to start";
            break;
    }

    elements_draw_list_row(canvas, ELEMENTS_HEADER_HEIGHT, 0, false, status_str);
    elements_draw_list_row(canvas, ELEMENTS_HEADER_HEIGHT, 1, false, action_str);
}

static void net_manager_view_enter(void* context) {
    NetManagerView* view = (NetManagerView*)context;

    /* If networking is already up (started elsewhere), show the live status
     * instead of "Press to start". */
    view->last_state = net_manager_get_state();
    view->phase = (view->last_state == NET_STATE_IDLE) ? NET_MANAGER_VIEW_PHASE_IDLE : NET_MANAGER_VIEW_PHASE_RUNNING;

    if (view->last_state == NET_STATE_CONNECTED) net_manager_view_reset_info(view);
}

static bool net_manager_view_input(const InputEvent* event, void* context) {
    NetManagerView* view = (NetManagerView*)context;

    if (view->phase == NET_MANAGER_VIEW_PHASE_IDLE) {
        if (event->key == INPUT_KEY_SELECT && event->type == INPUT_EVENT_TYPE_PRESS) {
            net_manager_start();
            view->phase = NET_MANAGER_VIEW_PHASE_RUNNING;
            view->last_state = net_manager_get_state();
            return true;
        }
        /* Let the global handler pop back to the menu when idle. */
        return false;
    }

    if (view->phase == NET_MANAGER_VIEW_PHASE_RUNNING && event->key == INPUT_KEY_DOWN &&
        event->type == INPUT_EVENT_TYPE_LONG_PRESS) {
        net_manager_stop();
        view->phase = NET_MANAGER_VIEW_PHASE_STOPPING;
        return true;
    }

    /* While running or stopping, block back navigation: the only way out is the
     * stop flow completing. */
    return true;
}

static bool net_manager_view_tick(void* context) {
    NetManagerView* view = (NetManagerView*)context;

    NetState state = net_manager_get_state();

    if (view->phase == NET_MANAGER_VIEW_PHASE_STOPPING && state == NET_STATE_IDLE) {
        view_manager_pop_to(view->view_manager, VIEW_ID_MENU);
        return true;
    }

    if (view->phase == NET_MANAGER_VIEW_PHASE_IDLE) return false;

    bool redraw = false;

    if (state != view->last_state) {
        view->last_state = state;
        if (state == NET_STATE_CONNECTED) net_manager_view_reset_info(view);
        redraw = true;
    }

    /* While connected, cycle the info line (IP -> mDNS -> hint) so both the IP
       and the mDNS host stay discoverable on the 2-line screen. */
    if (state == NET_STATE_CONNECTED && ++view->info_tick >= NET_INFO_TOGGLE_TICKS) {
        view->info_tick = 0;
        view->info_index = (uint8_t)((view->info_index + 1) % NET_INFO_COUNT);
        redraw = true;
    }

    return redraw;
}

void net_manager_view_init(NetManagerView* net_manager_view, ViewManager* view_manager) {
    ABORT_IF(net_manager_view == NULL || view_manager == NULL);

    net_manager_view->view_manager = view_manager;
    net_manager_view->phase = NET_MANAGER_VIEW_PHASE_IDLE;
    net_manager_view->last_state = NET_STATE_IDLE;
    net_manager_view->info_tick = 0;
    net_manager_view->info_index = NET_INFO_IP;
    net_manager_view->ip_str[0] = '\0';

    view_set_draw_callback(&net_manager_view->view, net_manager_view_draw);
    view_set_input_callback(&net_manager_view->view, net_manager_view_input);
    view_set_enter_callback(&net_manager_view->view, net_manager_view_enter);
    view_set_tick_callback(&net_manager_view->view, net_manager_view_tick);
    view_set_context(&net_manager_view->view, net_manager_view);

    view_manager_add_view(view_manager, VIEW_ID_NET_MANAGER, &net_manager_view->view);
}
