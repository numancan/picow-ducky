#pragma once

#include "FreeRTOS.h"
#include "hal/hal.h"
#include "hal/hal_gpio.h"
#include "queue.h"

typedef enum {
    INPUT_EVENT_TYPE_PRESS,
    INPUT_EVENT_TYPE_LONG_PRESS,
    INPUT_EVENT_TYPE_REPEAT,
    INPUT_EVENT_TYPE_NONE /* below debounce threshold; never published */
} InputEventType;

typedef struct {
    InputEventType type;
    InputKey key;
} InputEvent;

// Allocates input state, creates the debounce timers, and starts input_task.
void input_init();
// Polls GPIO inputs, debounces them, and publishes InputEvent to the event
// queue. Runs until shutdown is requested via the power manager.
void input_task(void* params);
// Returns a short human-readable name for event_type, e.g. "Press".
const char* input_event_get_name(InputEventType event_type);
// Queue that input_task publishes InputEvent to. Valid only after input_init.
QueueHandle_t input_get_event_queue(void);