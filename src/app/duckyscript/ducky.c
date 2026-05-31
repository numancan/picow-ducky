#include "ducky.h"

#include <stdlib.h>

#include "FreeRTOS.h"
#include "ducky_config.h"
#include "ducky_handler.h"
#include "ducky_parser.h"
#include "hal/hal.h"
#include "hal/hal_gpio.h"
#include "hid/usb_hid.h"
#include "log.h"
#include "middleware/fat_io.h"
#include "queue.h"
#include "string.h"
#include "web_server/cgi.h"

static const char* TAG = "DUCKY";

static FIL current_file;
static QueueHandle_t payload_queue = NULL;
static DuckyEventType ducky_status = DUCKY_EVENT_NOT_INITIATED;

// static PubSub ducky_pubsub = {.subs_callbacks = NULL, .subscribe_count = 0};

// void ducky_event_subscribe(DuckyEventCallback callback) { pubsub_subscribe(&ducky_pubsub, callback); }

// void ducky_event_unsubscribe(DuckyEventCallback callback) { pubsub_subscribe(&ducky_pubsub, callback); }

static void ducky_change_status(DuckyEventType event) {
    ducky_status = event;
    // pubsub_notify(&ducky_pubsub, &event);
}

static void ducky_trigger_callback(CgiEvent* event) {
    if (event->cgi_event_type != CGI_EVENT_TRIGGER) return;

    LOG_INFO(TAG, "ducky_trigger_callback: %s %d", event->value, strlen(event->value));
    ducky_play_script(event->value);

    // // TODO: get payload path
    // snprintf(next_payload, strlen(event->value) + 1, "%s", event->value);

    // sd_card_change_dir("/payloads");
    // if (sd_card_open_read(&current_file, next_payload) == FR_OK && !send_payload)
    //     send_payload = true;
}

void ducky_init() {
    if (payload_queue == NULL) payload_queue = xQueueCreate(5, DUCKY_MAX_PAYLOAD_FNAME_LEN + 1);
}

void ducky_play_script(const char* payload_name) {
    if (payload_queue == NULL) {
        LOG_ERROR(TAG, "ducky_play_script: error, queue not initialized");
        return;
    }
    LOG_INFO(TAG, "ducky_play_script: queueing %s\n", payload_name);

    // TODO: max payload name
    char payload_buffer[DUCKY_MAX_PAYLOAD_FNAME_LEN + 1] = {0};
    snprintf(payload_buffer, sizeof(payload_buffer), "%s", payload_name);
    xQueueSend(payload_queue, payload_buffer, 0);
}

// TODO: web server
// cgi_event_subscribe(ducky_trigger_callback);

void ducky_task() {
    ducky_line_t last_line = {0};
    char line_buffer[DUCKY_MAX_LINE_LEN + 3];  // +3 for \r\n\0
    char current_payload[24];

    while (1) {
        // if (hal_gpio_read(gpio_inputs[INPUT_KEY_SELECT].pin)) {
        //     vTaskDelay(pdMS_TO_TICKS(500));
        //     if (hal_gpio_read(gpio_inputs[INPUT_KEY_SELECT].pin)) {
        //         xQueueSend(payload_queue, "timing_test.txt", 0);
        //     }
        //     vTaskDelay(pdMS_TO_TICKS(500));
        // }

        if (xQueueReceive(payload_queue, current_payload, pdMS_TO_TICKS(100)) == pdPASS) {
            fat_io_change_dir("/payloads");
            if (fat_io_open_read(&current_file, current_payload) == FR_OK) {
                ducky_change_status(DUCKY_EVENT_PAYLOAD_PLAYING);

                while (1) {
                    ducky_line_t ducky_line;
                    FatIoReadResult res = fat_io_read_line(&current_file, line_buffer, sizeof(line_buffer));

                    if (res == FAT_IO_READ_OK) {
                        if (ducky_parser_parse_line(line_buffer, &ducky_line)) {
                            LOG_DEBUG(TAG, "command: %s args:%s", ducky_line.command, ducky_line.args);

                            if (strncmp(ducky_line.command, "REPEAT", 6) == 0) {
                                uint32_t repeat_count = (uint32_t)atoi(ducky_line.args);
                                for (uint32_t i = 0; i < repeat_count; i++) {
                                    ducky_handler_exec_line(&last_line);
                                }
                            } else {
                                ducky_handler_exec_line(&ducky_line);
                                last_line = ducky_line;
                            }
                        }
                    } else {
                        if (res == FAT_IO_READ_LINE_TOO_LONG) {
                            LOG_ERROR(TAG, "Error: Max allowed characters per line is %d. Stopping payload.",
                                      DUCKY_MAX_LINE_LEN);
                        } else if (res == FAT_IO_READ_ERROR) {
                            LOG_ERROR(TAG, "Error: Failed to read from SD card. Stopping payload.");
                        }

                        break;
                    }
                }

                fat_io_change_dir("..");
                fat_io_close(&current_file);
                ducky_change_status(DUCKY_EVENT_PAYLOAD_STOPPED);
            } else {
                LOG_ERROR(TAG, "Error: Could not open payload file %s", current_payload);
            }
        }
    }

    // TODO: cleanup
    usb_hid_deinit();
    vTaskDelete(NULL);
}