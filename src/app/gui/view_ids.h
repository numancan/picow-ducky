#pragma once

#include <stdint.h>

#include "app/duckyscript/ducky_view_ids.h"
#include "app/net_manager/net_manager_view_ids.h"

typedef enum {
    VIEW_ID_MENU = 0,

#define ADD_VIEW(name) name,

    DUCKY_VIEWS
    NET_MANAGER_VIEWS

#undef ADD_VIEW

        VIEW_ID_COUNT, /* keep last — sizes the view table */
} ViewId;

/* Sentinel: "this view has no parent / back does nothing". */
#define VIEW_ID_NONE ((uint32_t)0xFFFFFFFFu)
