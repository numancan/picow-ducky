#include "status_led.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "hal/hal.h"
#include "hal/hal_gpio.h"
#include "queue.h"
#include "task.h"

/*
 * Note: One LED pattern per priority is sufficient for this application,
 * but more priority slots can be added as the number of LEDs and tasks increases.
 */

const LedPattern STATUS_LED_PATTERN_NONE = {0, 0, STATUS_LED_REPEAT_STOP};
const LedPattern STATUS_LED_PATTERN_OFF = {0, 1, STATUS_LED_REPEAT_INFINITE};
const LedPattern STATUS_LED_PATTERN_ON = {1, 0, STATUS_LED_REPEAT_INFINITE};

typedef struct {
    StatusLedPriority prio;
    LedPattern pattern;
} LedMsg;

typedef struct {
    LedPattern pattern;
    bool is_on;
    int16_t current_repeat;
    TickType_t last_toggle_time;
} LedState;

static QueueHandle_t led_queue;
static LedState state[STATUS_LED_PRIORITY_COUNT];

static inline bool is_pattern_none(const LedPattern* p) {
    return (p->repeat == STATUS_LED_REPEAT_STOP) || (p->on_ms == 0 && p->off_ms == 0);
}

static int find_highest_active_priority(void) {
    for (int i = STATUS_LED_PRIORITY_COUNT - 1; i >= 0; i--) {
        if (!is_pattern_none(&state[i].pattern)) {
            return i;
        }
    }
    return -1;
}

static TickType_t handle_blink_pattern(TickType_t now, int* rendered_priority) {
    int idx = *rendered_priority;
    TickType_t current_duration =
        pdMS_TO_TICKS(state[idx].is_on ? state[idx].pattern.on_ms : state[idx].pattern.off_ms);
    TickType_t elapsed = now - state[idx].last_toggle_time;

    if (elapsed < current_duration) {
        return current_duration - elapsed;
    }

    // Time to toggle
    if (state[idx].is_on) {
        // Finished ON phase → switch to OFF
        state[idx].is_on = false;
        hal_gpio_write(STATUS_LED_PIN, false);
        state[idx].last_toggle_time = now;
        return pdMS_TO_TICKS(state[idx].pattern.off_ms);
    }

    // Finished OFF phase → one full cycle complete
    if (state[idx].current_repeat > 0) {
        state[idx].current_repeat--;
    }

    if (state[idx].current_repeat == 0) {
        // Pattern done
        state[idx].pattern = STATUS_LED_PATTERN_NONE;
        *rendered_priority = -1;
        return 0;
    }

    // Start next ON phase
    state[idx].is_on = true;
    hal_gpio_write(STATUS_LED_PIN, true);
    state[idx].last_toggle_time = now;
    return pdMS_TO_TICKS(state[idx].pattern.on_ms);
}

static TickType_t handle_static_pattern(uint32_t duration_ms, TickType_t now, int* rendered_priority) {
    int idx = *rendered_priority;
    if (state[idx].current_repeat == STATUS_LED_REPEAT_INFINITE) {
        return portMAX_DELAY;
    }

    TickType_t elapsed = now - state[idx].last_toggle_time;
    TickType_t duration = pdMS_TO_TICKS(duration_ms);

    if (elapsed >= duration) {
        state[idx].pattern = STATUS_LED_PATTERN_NONE;
        *rendered_priority = -1;
        return 0;  // Süre doldu, hemen uyan
    }

    return duration - elapsed;  // Kalan süre
}

static void process_incoming_message(const LedMsg* msg, int* rendered_priority) {
    int prio = msg->prio;
    LedPattern pattern = msg->pattern;

    // Normalize static patterns
    if (pattern.on_ms > 0 && pattern.off_ms == 0 && pattern.repeat > 0) {
        pattern.on_ms = pattern.on_ms * pattern.repeat;
        pattern.repeat = 1;
    } else if (pattern.off_ms > 0 && pattern.on_ms == 0 && pattern.repeat > 0) {
        pattern.off_ms = pattern.off_ms * pattern.repeat;
        pattern.repeat = 1;
    }

    state[prio].pattern = pattern;
    state[prio].current_repeat = pattern.repeat;
    state[prio].is_on = (pattern.on_ms != 0);  // ON phase first, unless on_ms is 0
    // state[prio].last_toggle_time = xTaskGetTickCount();

    // printf("[LED] Pattern set for priority %d: %u/%u (repeat:%d)\n", prio, pattern.on_ms, pattern.off_ms,
    //        pattern.repeat);
    // printf("[LED] Active priority: %d => %d\n", *rendered_priority, prio);
    if (prio >= *rendered_priority) {
        *rendered_priority = -1;
    }
}

void status_led_set_pattern(StatusLedPriority priority, LedPattern pattern) {
    if (priority >= STATUS_LED_PRIORITY_COUNT) return;
    LedMsg msg = {.prio = priority, .pattern = pattern};
    if (led_queue) {
        xQueueSend(led_queue, &msg, 0);
    }
}

void status_led_clear_pattern(StatusLedPriority priority) { status_led_set_pattern(priority, STATUS_LED_PATTERN_NONE); }

void status_led_print_priorities() {
    printf("\n");
    for (int i = 0; i < STATUS_LED_PRIORITY_COUNT; i++) {
        printf("Priority %d: %u/%u (repeat:%d)\n", i, state[i].pattern.on_ms, state[i].pattern.off_ms,
               state[i].pattern.repeat);
    }
}

void status_led_task(void* params) {
    (void)params;

    led_queue = xQueueCreate(10, sizeof(LedMsg));
    for (int i = 0; i < STATUS_LED_PRIORITY_COUNT; i++) {
        state[i].pattern = STATUS_LED_PATTERN_NONE;
    }

    int current_priority = -1;

    while (1) {
        TickType_t now = xTaskGetTickCount();
        TickType_t next_wake = portMAX_DELAY;
        int highest = find_highest_active_priority();

        if (highest != -1) {
            // Sync hardware and timing when the active priority slot changes
            if (current_priority != highest) {
                // printf("[LED] Active priority changed: %d \n", highest);
                current_priority = highest;

                state[current_priority].last_toggle_time = now;
                state[current_priority].is_on = (state[current_priority].pattern.on_ms != 0);
                hal_gpio_write(STATUS_LED_PIN, state[current_priority].is_on);
            }

            printf("[LED] Active priority: %d\n", current_priority);

            if (state[current_priority].pattern.off_ms == 0) {
                // Always ON
                next_wake = handle_static_pattern(state[current_priority].pattern.on_ms, now, &current_priority);
            } else if (state[current_priority].pattern.on_ms == 0) {
                // Always OFF
                next_wake = handle_static_pattern(state[current_priority].pattern.off_ms, now, &current_priority);
            } else {
                // Blinking
                next_wake = handle_blink_pattern(now, &current_priority);
            }
        } else {
            // No active patterns — turn LED off

            hal_gpio_write(STATUS_LED_PIN, false);
            current_priority = -1;
        }

        // Block on queue until next_wake or a new message arrives
        LedMsg msg;
        if (xQueueReceive(led_queue, &msg, next_wake) == pdPASS) {
            process_incoming_message(&msg, &current_priority);
        }

        if (ulTaskNotifyTake(pdTRUE, 0)) {
            break;
        }
    }

    if (led_queue != NULL) {
        vQueueDelete(led_queue);
        led_queue = NULL;
    }

    hal_gpio_write(STATUS_LED_PIN, false);
    vTaskDelete(NULL);
}