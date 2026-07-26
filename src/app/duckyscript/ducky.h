#pragma once

#include "ducky_settings.h"
#include "middleware/enum_gen.h"

// $EXPORT=ID,DISPLAY_NAME
#define DUCKY_STATUS_LIST(X)                       \
    X(DUCKY_STATUS_NOT_INITIATED, "Not Initiated") \
    X(DUCKY_STATUS_IDLE, "Idle")                   \
    X(DUCKY_STATUS_PLAYING, "Playing")             \
    X(DUCKY_STATUS_DONE, "Done")

DECLARE_ENUM(DuckyStatus, DUCKY_STATUS_COUNT, DUCKY_STATUS_LIST)

// Human-readable display name for a status value.
const char* ducky_get_status_str(DuckyStatus status);

// Create the event queue, load settings and start the ducky task.
void ducky_init();

// Ducky task entry point: consumes queued events (arm/disarm/play/shutdown).
void ducky_task(void* pvParameters);

// Parse and queue one duckyscript line for live playback; rejected while idle.
// line_buffer is mutated in place by the parser (a copy is queued).
void ducky_play_line_request(char* line_buffer);

// Queue a payload file from /payloads for playback; rejected while idle (arm first).
void ducky_play_payload_request(const char* payload_name);

// Abort the running playback; the transport stays armed and queued items are kept.
void ducky_stop_payload_request();

// Queue an arm request: switch to the configured transport. Async; readiness via
// hid_transport_status().
void ducky_transport_arm_request(void);

// Abort playback, disarm the transport and return to idle; queued items are dropped.
void ducky_transport_disarm_request(void);

// Current playback/transport status.
DuckyStatus ducky_get_status();

// Live in-RAM settings shared with the GUI. Read-only for callers.
const DuckySettings* ducky_get_settings(void);

// Parse the config file at `path` onto the live settings and persist atomically; keys
// absent from the file keep their current value. Returns false on parse/I/O failure.
bool ducky_apply_settings_file(const char* path);