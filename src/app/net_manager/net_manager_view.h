#pragma once

#include "gui/view.h"
#include "gui/view_manager.h"
#include "net_manager.h"

typedef enum {
    NET_MANAGER_VIEW_PHASE_IDLE,      // not started yet: "Press to start"
    NET_MANAGER_VIEW_PHASE_RUNNING,   // started: live NetState, back is blocked
    NET_MANAGER_VIEW_PHASE_STOPPING,  // stop requested: wait for idle, then leave
} NetManagerViewPhase;

typedef struct {
    View view;
    ViewManager* view_manager;
    NetManagerViewPhase phase;
    NetState last_state;             // last shown state (tick redraw debounce)
    char ip_str[NET_IP_STR_SIZE];    // STA IP cached on the CONNECTED transition
    uint8_t info_tick;               // ticks since the connected info line last changed
    uint8_t info_index;              // connected info line cycle item (see NetInfoLine)
} NetManagerView;

/* Initializes the net manager view and registers it with the view manager. */
void net_manager_view_init(NetManagerView* net_manager_view, ViewManager* view_manager);
