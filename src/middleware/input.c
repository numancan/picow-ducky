#include "input.h"

#include "FreeRTOS.h"
#include "app/pubsub/pubsub.h"
#include "app/task_manager/task_manager.h"
#include "hal/hal.h"
#include "hal/hal_gpio.h"
#include "log.h"
#include "stdio.h"
#include "stdlib.h"
#include "sys_fault.h"
#include "task.h"
#include "timers.h"

#define DEBOUNCE_MS (50)
#define LONG_PRESS_MS (600)
#define REPEAT_DELAY_MS (1200)
#define REPEAT_INTERVAL_MS (200)
#define TIMER_PERIOD_MS (20)
#define INPUT_PUBSUB_MAX_SUBS (3)

typedef struct {
    GpioInput* gpio;
    TimerHandle_t timer_handle;
    volatile uint16_t press_count_ms;
    volatile uint16_t last_repeat_ms;
} InputState;

typedef struct {
    InputState* states;
    PubSub* pubsub;
} Input;

static const char* TAG = "INPUT";

static Input* input = NULL;

static void input_timer_callback(TimerHandle_t xTimer) {
    size_t timer_id = (size_t)pvTimerGetTimerID(xTimer);
    input->states[timer_id].press_count_ms += TIMER_PERIOD_MS;
}

const char* input_event_get_name(InputEventType event_type) {
    switch (event_type) {
        case INPUT_EVENT_TYPE_PRESS: return "Press"; break;

        case INPUT_EVENT_TYPE_LONG_PRESS: return "Long"; break;

        case INPUT_EVENT_TYPE_REPEAT: return "Repeat"; break;

        default: return ""; break;
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

QueueHandle_t input_subscribe(uint8_t depth) { return pubsub_subscribe(input->pubsub, depth); }

void input_unsubscribe(QueueHandle_t queue) { pubsub_unsubscribe(input->pubsub, queue); }

void input_init() {
    input = (Input*)malloc(sizeof(Input));
    input->states = (InputState*)malloc(sizeof(InputState) * gpio_input_count);
    input->pubsub = pubsub_alloc(sizeof(InputEvent), INPUT_PUBSUB_MAX_SUBS);
    PANIC_IF(input->pubsub == NULL, "pubsub alloc failed");

    for (size_t i = 0; i < gpio_input_count; i++) {
        input->states[i].gpio = (GpioInput*)&gpio_inputs[i];
        input->states[i].press_count_ms = 0;
        input->states[i].last_repeat_ms = 0;
        input->states[i].timer_handle =
            xTimerCreate("TM_IN", pdMS_TO_TICKS(TIMER_PERIOD_MS), pdTRUE, (void*)i, input_timer_callback);
    }
}

void input_task(void* params) {
    while (true) {
        for (size_t i = 0; i < gpio_input_count; i++) {
            InputState input_pin = input->states[i];
            uint8_t pin_state = hal_gpio_read(input_pin.gpio->pin);

            if (pin_state != input_pin.gpio->default_state && xTimerIsTimerActive(input_pin.timer_handle) == pdFALSE) {
                xTimerStart(input_pin.timer_handle, 0);
            } else if (pin_state != input_pin.gpio->default_state && xTimerIsTimerActive(input_pin.timer_handle)) {
                // Key held: emit a REPEAT every interval once the initial delay has elapsed
                if (input_pin.press_count_ms >= REPEAT_DELAY_MS &&
                    (uint16_t)(input_pin.press_count_ms - input_pin.last_repeat_ms) >= REPEAT_INTERVAL_MS) {
                    InputEvent input_event;
                    input_event.type = INPUT_EVENT_TYPE_REPEAT;
                    input_event.key = input_pin.gpio->key;
                    pubsub_notify(input->pubsub, &input_event);

                    input->states[i].last_repeat_ms = input_pin.press_count_ms;

                    LOG_DEBUG(TAG, "Button Key: %d - Input Event: %s - Press(ms): %d", input_pin.gpio->key,
                              input_event_get_name(input_event.type), input_pin.press_count_ms);
                }
            } else if (pin_state == input_pin.gpio->default_state && xTimerIsTimerActive(input_pin.timer_handle)) {
                xTimerStop(input_pin.timer_handle, 0);

                // Suppress the release event if the press already turned into a repeat (no trailing LONG_PRESS)
                if (input_pin.last_repeat_ms == 0) {
                    InputEvent input_event;
                    input_event.type = input_find_event_type(input_pin.press_count_ms);

                    if (input_event.type != INPUT_EVENT_TYPE_NONE) {
                        input_event.key = input_pin.gpio->key;
                        pubsub_notify(input->pubsub, &input_event);

                        LOG_DEBUG(TAG, "Button Key: %d - Input Event: %s - Press(ms): %d", input_pin.gpio->key,
                                  input_event_get_name(input_event.type), input_pin.press_count_ms);
                    }
                }

                input->states[i].press_count_ms = 0;
                input->states[i].last_repeat_ms = 0;
            }
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1))) {
            break;
        }
    }

    // Cleanup
    if (input != NULL) {
        for (size_t i = 0; i < gpio_input_count; i++) {
            if (input->states[i].timer_handle != NULL) {
                xTimerStop(input->states[i].timer_handle, 0);
                xTimerDelete(input->states[i].timer_handle, 0);
            }
        }
        pubsub_dealloc(input->pubsub);
        free(input->states);
        free(input);
        input = NULL;
    }

    task_manager_report_stopped(TASK_MANAGER_TASK_INPUT);
    vTaskDelete(NULL);
}