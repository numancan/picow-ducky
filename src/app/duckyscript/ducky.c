#include "ducky.h"

#include "handler.h"
#include "hid/keyboard.h"
#include "middleware/sd_card.h"
#include "parser.h"
#include "pubsub/pubsub.h"
#include "stdio.h"
#include "string.h"
#include "web_server/cgi.h"

static FIL current_file;
static char next_payload[24];  // TODO: max payload name
static bool send_payload = false;

static DuckyEventType ducky_status = DUCKY_EVENT_NOT_INITIATED;

static PubSub ducky_pubsub = {.subs_callbacks = NULL, .subscribe_count = 0};

void ducky_event_subscribe(DuckyEventCallback callback) { pubsub_subscribe(&ducky_pubsub, callback); }

void ducky_event_unsubscribe(DuckyEventCallback callback) { pubsub_subscribe(&ducky_pubsub, callback); }

static void ducky_change_status(DuckyEventType event) {
    ducky_status = event;
    pubsub_notify(&ducky_pubsub, &event);
}

static void ducky_trigger_callback(CgiEvent* event) {
    if (event->cgi_event_type != CGI_EVENT_TRIGGER) return;

    printf("ducky_trigger_callback: %s %d\n", event->value, strlen(event->value));
    ducky_play_script(event->value);

    // // TODO: get payload path
    // snprintf(next_payload, strlen(event->value) + 1, "%s", event->value);

    // sd_card_change_dir("/payloads");
    // if (sd_card_open_read(&current_file, next_payload) == FR_OK && !send_payload)
    //     send_payload = true;
}

// TODO: return status make callback use this func
void ducky_play_script(char* payload_name) {
    printf("ducky_play_script: %s\n", payload_name);

    // TODO: get payload path
    snprintf(next_payload, strlen(payload_name) + 1, "%s", payload_name);

    sd_card_change_dir("/payloads");
    if (sd_card_open_read(&current_file, next_payload) == FR_OK && !send_payload) {
        send_payload = true;
        ducky_change_status(DUCKY_EVENT_PAYLOAD_PLAYING);
    }
}

void ducky_task() {
    kb_init();

    // TODO: web server
    // cgi_event_subscribe(ducky_trigger_callback);

    if (ducky_status == DUCKY_EVENT_NOT_INITIATED) {
        ducky_change_status(DUCKY_EVENT_INITIATED);
    }

    while (1) {
        if (send_payload && dh_is_ready()) {
            ducky_line_t* dl;
            char line_buffer[256];

            if (sd_card_read_line(&current_file, line_buffer, sizeof(line_buffer))) {
                dl = dp_parse_line(line_buffer);
                dh_handle_dline(dl);

            } else {
                // End of file
                memset(next_payload, 0, sizeof(next_payload));

                sd_card_change_dir("..");
                sd_card_close(&current_file);
                send_payload = false;
                ducky_change_status(DUCKY_EVENT_PAYLOAD_STOPPED);
            }
        }

        kb_task();
    }
}