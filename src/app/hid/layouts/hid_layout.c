#include "hid_layout.h"

#include "hid_layout_tr_q.h"
#include "hid_layout_us_q.h"
#include "sys_fault.h"

static HIDLayoutID active_layout = HID_KEY_LAYOUT_US_Q;

const KeyEntry* hid_layout_get_active_table(void) {
#define X(id, str, tbl) \
    if (id == active_layout) return tbl;
    HID_KEY_LAYOUT_LIST(X)
#undef X

    ABORT_IF(true);
};

void hid_layout_set(HIDLayoutID layout) {
    ABORT_IF(layout >= HID_KEY_LAYOUT_COUNT);
    active_layout = layout;
}

bool hid_layout_set_by_name(const char* name) {
#define X(id, str, tbl)       \
    if (!strcmp(name, str)) { \
        hid_layout_set(id);   \
        return true;          \
    }
    HID_KEY_LAYOUT_LIST(X)
#undef X

    return false;
}

const char* hid_layout_name(HIDLayoutID layout) {
#define X(id, str, tbl) \
    if (id == layout) return str;
    HID_KEY_LAYOUT_LIST(X)
#undef X
    // ABORT_IF(true);
    return "UNKNOWN";
}
