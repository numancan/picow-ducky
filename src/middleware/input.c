#include "input.h"

#include "FreeRTOS.h"
#include "config/task_config.h"
#include "hal/hal.h"
#include "hal/hal_gpio.h"
#include "middleware/log.h"
#include "middleware/sys_fault.h"
#include "sleep_manager.h"
#include "stdio.h"
#include "stdlib.h"
#include "task.h"
#include "timers.h"

#define DEBOUNCE_MS (50)
#define LONG_PRESS_MS (600)
#define REPEAT_DELAY_MS (1200)
#define REPEAT_INTERVAL_MS (200)
#define TIMER_PERIOD_MS (20)
#define INPUT_EVENT_QUEUE_DEPTH (6)

typedef struct {
    GpioInput* gpio;
    TimerHandle_t timer_handle;
    volatile uint32_t press_count_ms; /* grows unbounded while the key is held; must not wrap */
    volatile uint32_t last_repeat_ms;
} InputState;

typedef struct {
    InputState* states;
    QueueHandle_t queue;
} Input;

static const char* TAG = "INPUT";

static Input* input = NULL;
static TaskHandle_t task_handle = NULL;

static void input_shutdown_cb(void* context) {
    (void)context;
    xTaskNotifyGive(task_handle);
}

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

static InputEventType input_find_event_type(uint32_t press_count_ms) {
    if (press_count_ms >= DEBOUNCE_MS && press_count_ms <= LONG_PRESS_MS) {
        return INPUT_EVENT_TYPE_PRESS;
    } else if (press_count_ms >= LONG_PRESS_MS) {
        return INPUT_EVENT_TYPE_LONG_PRESS;
    }

    return INPUT_EVENT_TYPE_NONE;
}

static void input_publish(const InputEvent* event) {
    if (xQueueSend(input->queue, event, 0) != pdTRUE) {
        LOG_WARN(TAG, "Input queue full, event dropped");
    }
}

QueueHandle_t input_get_event_queue(void) {
    ABORT_IF(input == NULL);
    return input->queue;
}

void input_init() {
    input = (Input*)malloc(sizeof(Input));
    input->states = (InputState*)malloc(sizeof(InputState) * gpio_input_count);
    input->queue = xQueueCreate(INPUT_EVENT_QUEUE_DEPTH, sizeof(InputEvent));
    PANIC_IF(input->queue == NULL, "input queue alloc failed");

    for (size_t i = 0; i < gpio_input_count; i++) {
        input->states[i].gpio = (GpioInput*)&gpio_inputs[i];
        input->states[i].press_count_ms = 0;
        input->states[i].last_repeat_ms = 0;
        input->states[i].timer_handle =
            xTimerCreate("TM_IN", pdMS_TO_TICKS(TIMER_PERIOD_MS), pdTRUE, (void*)i, input_timer_callback);
    }

    task_handle = task_create(&INPUT_TASK_CONFIG, input_task, NULL);
    PANIC_IF(task_handle == NULL, "input task create failed");

    sleep_manager_register(input_shutdown_cb, NULL);
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
                    (input_pin.press_count_ms - input_pin.last_repeat_ms) >= REPEAT_INTERVAL_MS) {
                    InputEvent input_event;
                    input_event.type = INPUT_EVENT_TYPE_REPEAT;
                    input_event.key = input_pin.gpio->key;
                    input_publish(&input_event);

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
                        input_publish(&input_event);

                        LOG_DEBUG(TAG, "Button Key: %d - Input Event: %s - Press(ms): %d", input_pin.gpio->key,
                                  input_event_get_name(input_event.type), input_pin.press_count_ms);
                    }
                }

                input->states[i].press_count_ms = 0;
                input->states[i].last_repeat_ms = 0;
            }
        }

        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10))) {
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
        vQueueDelete(input->queue);
        free(input->states);
        free(input);
        input = NULL;
    }

    LOG_INFO(TAG, "shutdown");
    sleep_manager_ack_shutdown();
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}