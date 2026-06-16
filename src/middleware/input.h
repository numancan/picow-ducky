#pragma once

#include "FreeRTOS.h"
#include "hal/hal.h"
#include "hal/hal_gpio.h"
#include "queue.h"

typedef enum {
    INPUT_EVENT_TYPE_PRESS,
    INPUT_EVENT_TYPE_LONG_PRESS,
    INPUT_EVENT_TYPE_REPEAT,
    INPUT_EVENT_TYPE_NONE
} InputEventType;

typedef struct {
    InputEventType type;
    InputKey key;
} InputEvent;

void input_init();
void input_task(void* params);
const char* input_event_get_name(InputEventType);
QueueHandle_t input_subscribe(uint8_t depth);
void input_unsubscribe(QueueHandle_t queue);