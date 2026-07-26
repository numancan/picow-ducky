#include "provisioning_web.h"

#include <string.h>

#include "middleware/log.h"
#include "net_manager/net_manager.h"

static const char* TAG = "PVG_CGI";

typedef enum {
    CGI_EVENT_NONE,
    CGI_EVENT_FORGET,
} CgiEventType;

static CgiEventType cgi_event_type_from_str(const char* str) {
    if (strcmp(str, "forget") == 0) return CGI_EVENT_FORGET;
    return CGI_EVENT_NONE;
}

void provisioning_cgi_dispatch(const char* name, const char* value) {
    LOG_INFO(TAG, "CGI HANDLER: %s %s", name, value);

    switch (cgi_event_type_from_str(name)) {
        case CGI_EVENT_FORGET: net_manager_forget(); break;
        default: LOG_ERROR(TAG, "Invalid CGI event type"); break;
    }
}
