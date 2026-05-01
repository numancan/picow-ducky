#include "input.h"

#include "FreeRTOS.h"
#include "debug.h"
#include "hal/hal.h"
#include "hal/hal_gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include "task.h"
#include "timers.h"

#define DEBOUNCE_MS (50)
#define LONG_PRESS_MS (800)
#define TIMER_PERIOD_MS (20)

typedef struct {
    GpioInput* gpio;
    TimerHandle_t timer_handler;
    volatile uint16_t press_count_ms;
} InputState;

typedef struct {
    InputState* states;
    PubSubFree* pubsub;
} Input;

static Input* input = NULL;

static void input_timer_callback(TimerHandle_t xTimer) {
    uint8_t timer_id = (uint8_t)pvTimerGetTimerID(xTimer);
    input->states[timer_id].press_count_ms += TIMER_PERIOD_MS;
    // printf("ID:%d press_ms: %d\n",  (size_t) pvTimerGetTimerID( xTimer ),
    // inputs[timer_id].press_count_ms);
}

const char* input_event_get_name(InputEventType event_type) {
    switch (event_type) {
        case INPUT_EVENT_TYPE_PRESS:
            return "Press";
            break;

        case INPUT_EVENT_TYPE_LONG_PRESS:
            return "Long";
            break;

        case INPUT_EVENT_TYPE_RELEASE:
            return "Release";
            break;

        default:
            return "wtf?";
            break;
    }
}

static InputEventType input_find_event_type(uint16_t press_count_ms) {
    if (press_count_ms >= DEBOUNCE_MS && press_count_ms <= LONG_PRESS_MS) {
        return INPUT_EVENT_TYPE_PRESS;
    } else if (press_count_ms >= LONG_PRESS_MS) {
        return INPUT_EVENT_TYPE_LONG_PRESS;
    }

    return INPUT_EVENT_TYPE_NONE;
}

PubSubFree* input_get_pubsub() { return input->pubsub; }

void input_init() {
    input = (Input*)malloc(sizeof(Input));
    input->states = (InputState*)malloc(sizeof(InputState) * gpio_input_count);
    input->pubsub = pubsub_free_alloc();
}

void input_service() {
    for (size_t i = 0; i < gpio_input_count; i++) {
        input->states[i].gpio = (GpioInput*)&gpio_inputs[i];
        // inputs->pins[i].state = hal_gpio_read(inputs->pins[i].gpio->pin);
        input->states[i].press_count_ms = 0;
        input->states[i].timer_handler =
            xTimerCreate("TM_IN", pdMS_TO_TICKS(TIMER_PERIOD_MS), pdTRUE, (void*)i,
                         input_timer_callback);
    }

    while (true) {
        for (size_t i = 0; i < gpio_input_count; i++) {
            InputState input_pin = input->states[i];
            uint8_t pin_state = hal_gpio_read(input_pin.gpio->pin);

            // Key pressed
            if (pin_state != input_pin.gpio->default_state &&
                xTimerIsTimerActive(input_pin.timer_handler) == pdFALSE) {
                xTimerStart(input_pin.timer_handler, 0);
            } else if (pin_state == input_pin.gpio->default_state &&
                       xTimerIsTimerActive(input_pin.timer_handler)) {
                xTimerStop(input_pin.timer_handler, 0);

                InputEvent input_event;
                input_event.type = input_find_event_type(input_pin.press_count_ms);

                if (input_event.type != INPUT_EVENT_TYPE_NONE) {
                    input_event.key = input_pin.gpio->key;
                    pubsub_free_notify(input->pubsub, &input_event);

                    DEBUG_PRINTF("Buton Key: %d - Input Event: %s - Press(ms): %d\n",
                                 input_pin.gpio->key,
                                 input_event_get_name(input_event.type),
                                 input_pin.press_count_ms);
                }

                input->states[i].press_count_ms = 0;
            }
        }
    }

    free(input->states);
    free(input);
}