#include <stdio.h>
#include <string.h>

#include "ducky_web.h"
#include "duckyscript/ducky.h"
#include "duckyscript/ducky_config.h"
#include "middleware/enum_gen.h"
#include "middleware/fat_io.h"
#include "middleware/log.h"

static const char* TAG = "CGI";

#define CGI_EVENT_LIST(X)             \
    X(CGI_EVENT_NONE, "none")         \
    X(CGI_EVENT_ARM, "arm")           \
    X(CGI_EVENT_TRIGGER, "trigger")   \
    X(CGI_EVENT_STOP, "stop")         \
    X(CGI_EVENT_DELETE, "delete")     \
    X(CGI_EVENT_SETTINGS, "settings") \
    X(CGI_EVENT_LIVE, "live")         \
    X(CGI_EVENT_KEY, "key")

DECLARE_ENUM(CgiEventType, CGI_EVENT_COUNT, CGI_EVENT_LIST);

static CgiEventType cgi_event_type_from_str(const char* str) {
#define X(event, value) \
    if (strcmp(str, value) == 0) return event;
    CGI_EVENT_LIST(X)
#undef X
    return CGI_EVENT_NONE;
}

void ducky_cgi_dispatch(const char* name, const char* value) {
    LOG_INFO(TAG, "CGI HANDLER: %s %s", name, value);

    switch (cgi_event_type_from_str(name)) {
        // Start the configured transport so the browser can poll readiness before triggering.
        case CGI_EVENT_ARM: ducky_transport_arm_request(); break;
        case CGI_EVENT_TRIGGER: ducky_play_payload_request(value); break;
        case CGI_EVENT_STOP: ducky_stop_payload_request(); break;
        case CGI_EVENT_DELETE: break;  // TODO: payload delete
        case CGI_EVENT_SETTINGS:
            // post.c staged the config body at DUCKY_CONFIG_RX_PATH; parse + apply it, then clean up.
            ducky_apply_settings_file(DUCKY_CONFIG_RX_PATH);
            fat_io_remove_file(DUCKY_CONFIG_RX_PATH);
            break;
        // post.c streamed the live editor body to DUCKY_LIVE_PATH (in DUCKY_PAYLOAD_DIR); queue it
        // by filename. The file is left in place — playback is async, so removing it here would race.
        case CGI_EVENT_LIVE:
            ducky_play_payload_request(DUCKY_LIVE_FNAME);
            break;  // TODO: we can use trigger
        // Play one duckyscript line (e.g. a consumer key from the web UI). ducky_play_line_request
        // mutates its buffer, so copy the query value into a local mutable buffer first.
        case CGI_EVENT_KEY:
            if (value != NULL) {
                char line[DUCKY_MAX_LINE_LEN + 1];
                snprintf(line, sizeof(line), "%s", value);
                ducky_play_line_request(line);
            }
            break;
        default: LOG_ERROR(TAG, "Invalid CGI event type"); break;
    }
}
