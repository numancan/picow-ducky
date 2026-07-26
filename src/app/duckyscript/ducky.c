#include "ducky.h"

#include <stdlib.h>

#include "FreeRTOS.h"
#include "config/task_config.h"
#include "ducky_config.h"
#include "ducky_handler.h"
#include "ducky_parser.h"
#include "ducky_view.h"
#include "gui/gui.h"
#include "hid/hid_transport.h"
#include "middleware/fat_io.h"
#include "middleware/log.h"
#include "middleware/sleep_manager.h"
#include "middleware/sys_fault.h"
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "string.h"

static const char* TAG = "DUCKY";

#define MAX_EVENT_QUEUE 8

// ARMED needs no live connection; play items are accepted only while armed, dropped in IDLE.
typedef enum { DUCKY_STATE_IDLE, DUCKY_STATE_ARMED } DuckyState;

typedef enum {
    DUCKY_EVENT_ARM,
    DUCKY_EVENT_DISARM,
    DUCKY_EVENT_SHUTDOWN,
    DUCKY_EVENT_PLAY_PAYLOAD,
    DUCKY_EVENT_PLAY_LINE
} DuckyEventType;

typedef struct {
    DuckyEventType type;
    union {
        char payload_path[DUCKY_MAX_PAYLOAD_PATH_LEN + 1];
        DuckyLine line;
    };
} DuckyEvent;

static FIL current_file;
static QueueHandle_t event_queue = NULL;
static _Atomic bool stop_requested = false;
static _Atomic DuckyState ducky_state = DUCKY_STATE_IDLE;
static DuckyStatus ducky_status = DUCKY_STATUS_NOT_INITIATED;
static DuckyView ducky_view;
static DuckySettings settings;
static bool should_shutdown = false;

/* Shared sender for the payload-less control events; drops with a warning on a full queue. */
static void send_event(DuckyEventType type) {
    DuckyEvent event = {.type = type};
    if (xQueueSend(event_queue, &event, 0) != pdPASS) {
        LOG_WARN(TAG, "event %d dropped, queue full", type);
    }
}

static void ducky_shutdown_cb(void* context) {
    (void)context;
    stop_requested = true;
    send_event(DUCKY_EVENT_SHUTDOWN);
}

static void ducky_change_status(DuckyStatus status) {
    ABORT_IF(status >= DUCKY_STATUS_COUNT);
    ducky_status = status;
}

const char* ducky_get_status_str(DuckyStatus status) { ENUM_TO_STR_SWITCH(status, DUCKY_STATUS_LIST) }

/* Disarm + drop anything still queued. The state transition itself stays in the task. */
static void disarm_transport(void) {
    hid_transport_stop();
    xQueueReset(event_queue);
}

// IDLE -> ARMED on a successful switch; stays IDLE when the transport cannot be started.
static DuckyState switch_transport(void) {
    ducky_change_status(DUCKY_STATUS_IDLE);
    if (!hid_transport_switch(settings.transporter)) {
        LOG_WARN(TAG, "Not active transport found and cannot switch to %s", hid_transport_name(settings.transporter));
        return DUCKY_STATE_IDLE;
    }
    return DUCKY_STATE_ARMED;
}

// Executes the whole payload file. On a not-ready transport the first line fails and playback ends DONE.
static void play_payload_file(const char* payload_path) {
    if (fat_io_open_read(&current_file, payload_path) != FAT_IO_OK) {
        LOG_ERROR(TAG, "Error: Could not open payload file %s", payload_path);
        return;
    }

    ducky_change_status(DUCKY_STATUS_PLAYING);

    DuckyLine last_line = {0};
    char line_buffer[DUCKY_MAX_LINE_LEN + 3];  // +3 for \r\n\0
    DuckyStatus end_status = DUCKY_STATUS_DONE;

    while (1) {
        if (stop_requested) {
            end_status = DUCKY_STATUS_DONE;
            break;
        }

        DuckyLine ducky_line;
        FatIoResult res = fat_io_read_line(&current_file, line_buffer, sizeof(line_buffer));

        if (res != FAT_IO_OK) {
            switch (res) {
                case FAT_IO_EOF: break;
                case FAT_IO_BUFFER_SIZE_ERROR:
                    fat_io_skip_to_newline(&current_file);
                    LOG_ERROR(TAG, "Error: Max allowed characters per line is %d. Stopping payload.",
                              DUCKY_MAX_LINE_LEN);
                    end_status = DUCKY_STATUS_DONE;
                    break;
                case FAT_IO_ERROR:
                    LOG_ERROR(TAG, "Error: Failed to read from SD card. Stopping payload.");
                    end_status = DUCKY_STATUS_DONE;
                    break;
                default:
                    LOG_ERROR(TAG, "Error: Unexpected read result %d. Stopping payload.", res);
                    end_status = DUCKY_STATUS_DONE;
                    break;
            }
            break;
        }
        if (!ducky_parse_line(line_buffer, &ducky_line)) continue;

        LOG_DEBUG(TAG, "command: %s args:%s", ducky_line.command, ducky_line.args);

        bool ok;
        if (strncmp(ducky_line.command, "REPEAT", 6) == 0) {
            uint32_t repeat_count = (uint32_t)atoi(ducky_line.args);
            ok = true;
            for (uint32_t i = 0; i < repeat_count; i++) {
                ok = ducky_handler_exec_line(&last_line);
                if (!ok) break;
            }
        } else {
            ok = ducky_handler_exec_line(&ducky_line);
            last_line = ducky_line;
        }

        if (!ok) {
            LOG_WARN(TAG, "HID transport error, stopping payload");
            end_status = DUCKY_STATUS_DONE;
            break;
        }
    }

    fat_io_close(&current_file);
    ducky_change_status(end_status);
    vTaskDelay(pdMS_TO_TICKS(100));
}

static void play_single_line(DuckyLine* line) {
    ducky_change_status(DUCKY_STATUS_PLAYING);
    ducky_handler_exec_line(line);
    ducky_change_status(DUCKY_STATUS_DONE);
}

void ducky_transport_arm_request(void) {
    ABORT_IF(event_queue == NULL);
    send_event(DUCKY_EVENT_ARM);
}

void ducky_stop_payload_request(void) { stop_requested = true; }

void ducky_transport_disarm_request(void) {
    ABORT_IF(event_queue == NULL);
    stop_requested = true;
    send_event(DUCKY_EVENT_DISARM);
}

void ducky_play_line_request(char* line_buffer) {
    ABORT_IF(event_queue == NULL);

    if (ducky_state == DUCKY_STATE_IDLE) {
        LOG_WARN(TAG, "ducky_play_line_request: transport not armed, line rejected");
        return;
    }

    DuckyEvent event = {.type = DUCKY_EVENT_PLAY_LINE};
    if (!ducky_parse_line(line_buffer, &event.line)) return;

    if (xQueueSend(event_queue, &event, 0) != pdPASS) {
        LOG_WARN(TAG, "line dropped, queue full");
    }
}

void ducky_play_payload_request(const char* payload_name) {
    ABORT_IF(event_queue == NULL);

    if (ducky_state == DUCKY_STATE_IDLE) {
        LOG_WARN(TAG, "ducky_play_payload_request: transport not armed, rejecting %s", payload_name);
        return;
    }

    DuckyEvent event = {.type = DUCKY_EVENT_PLAY_PAYLOAD};
    snprintf(event.payload_path, sizeof(event.payload_path), "%s/%s", DUCKY_PAYLOAD_DIR, payload_name);
    if (fat_io_check_file_exist(event.payload_path) != FAT_IO_OK) {
        LOG_WARN(TAG, "ducky_play_payload_request: payload not found: %s", event.payload_path);
        return;
    }

    LOG_INFO(TAG, "ducky_play_payload_request: queueing %s", event.payload_path);
    if (xQueueSend(event_queue, &event, 0) != pdPASS) {
        LOG_WARN(TAG, "payload dropped, queue full");
    }
}

DuckyStatus ducky_get_status() { return ducky_status; }
const DuckySettings* ducky_get_settings(void) { return &settings; }

bool ducky_apply_settings_file(const char* path) {
    ABORT_IF(path == NULL);
    DuckySettings s = settings;
    if (!ducky_settings_parse_file(path, &s)) return false;

    settings = s;
    return ducky_settings_save(&settings);
}

void ducky_init() {
    if (event_queue == NULL) event_queue = xQueueCreate(MAX_EVENT_QUEUE, sizeof(DuckyEvent));
    PANIC_IF(event_queue == NULL, "ducky queue create failed");

    ABORT_IF(!ducky_settings_load(&settings));

    ducky_view_init(&ducky_view, &settings, gui_get_view_manager());
    ducky_change_status(DUCKY_STATUS_IDLE);

    TaskHandle_t task_handle = task_create(&DUCKY_TASK_CONFIG, ducky_task, NULL);
    PANIC_IF(task_handle == NULL, "ducky task create failed");

    sleep_manager_register(ducky_shutdown_cb, NULL);
}

void ducky_task(void* pvParameters) {
    (void)pvParameters;

    while (!should_shutdown) {
        DuckyEvent event;

        // ARMED uses a bounded wait so an unused transport auto-disarms; each event resets the window.
        TickType_t wait = (ducky_state == DUCKY_STATE_ARMED) ? pdMS_TO_TICKS(DUCKY_ARM_TIMEOUT_MS) : portMAX_DELAY;

        if (xQueueReceive(event_queue, &event, wait) != pdPASS) {
            // Timed out with no event: armed and idle past the window -> disarm.
            if (ducky_state == DUCKY_STATE_ARMED) {
                LOG_INFO(TAG, "arm timeout, disarming transport");
                disarm_transport();
                ducky_state = DUCKY_STATE_IDLE;
                ducky_change_status(DUCKY_STATUS_IDLE);
            }
            continue;
        }

        switch (event.type) {
            case DUCKY_EVENT_ARM: ducky_state = switch_transport(); break;
            case DUCKY_EVENT_DISARM:
                if (ducky_state != DUCKY_STATE_IDLE) {
                    disarm_transport();
                    ducky_state = DUCKY_STATE_IDLE;
                    ducky_change_status(DUCKY_STATUS_IDLE);
                }
                break;
            case DUCKY_EVENT_PLAY_PAYLOAD:
            case DUCKY_EVENT_PLAY_LINE:
                if (ducky_state == DUCKY_STATE_IDLE) {
                    LOG_WARN(TAG, "play dropped, transport not armed");
                    break;
                }
                stop_requested = false;
                ducky_handler_init(&settings);
                if (event.type == DUCKY_EVENT_PLAY_PAYLOAD) {
                    play_payload_file(event.payload_path);
                } else {
                    play_single_line(&event.line);
                }
                break;
            case DUCKY_EVENT_SHUTDOWN: should_shutdown = true; break;
            default: break;
        }
    }

    disarm_transport();
    ducky_change_status(DUCKY_STATUS_IDLE);
    vQueueDelete(event_queue);
    event_queue = NULL;

    LOG_INFO(TAG, "shutdown");
    sleep_manager_ack_shutdown();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}
