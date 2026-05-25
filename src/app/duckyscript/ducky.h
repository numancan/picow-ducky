#pragma once

typedef enum {
    DUCKY_EVENT_NOT_INITIATED = 0,
    DUCKY_EVENT_INITIATED,
    DUCKY_EVENT_PAYLOAD_PLAYING,
    DUCKY_EVENT_PAYLOAD_STOPPED
} DuckyEventType;

typedef void (*DuckyEventCallback)(DuckyEventType *event);

void ducky_init();
void ducky_task();
void ducky_play_script(char* payload_name);
void ducky_event_subscribe(DuckyEventCallback callback);
void ducky_event_unsubscribe(DuckyEventCallback callback);