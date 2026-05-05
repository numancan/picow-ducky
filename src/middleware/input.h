#pragma once

#include "app/pubsub/pubsub_free.h"
#include "hal/hal.h"
#include "hal/hal_gpio.h"

typedef enum {
    INPUT_EVENT_TYPE_PRESS,
    INPUT_EVENT_TYPE_LONG_PRESS,
    INPUT_EVENT_TYPE_DOUBLE_PRESS,
    INPUT_EVENT_TYPE_RELEASE,
    INPUT_EVENT_TYPE_NONE
} InputEventType;

typedef struct {
    InputEventType type;
    InputKey key;
} InputEvent;

void input_init();
void input_task(void* params);
const char* input_event_get_name(InputEventType);
PubSubFree* input_get_pubsub();