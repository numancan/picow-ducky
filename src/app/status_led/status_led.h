#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    STATUS_LED_PRIORITY_LOW = 0,
    STATUS_LED_PRIORITY_NORMAL,
    STATUS_LED_PRIORITY_HIGH,
    STATUS_LED_PRIORITY_ERROR,
    STATUS_LED_PRIORITY_COUNT
} StatusLedPriority;

#define STATUS_LED_REPEAT_INFINITE (-1)
#define STATUS_LED_REPEAT_STOP (0)

typedef struct {
    uint32_t on_ms;
    uint32_t off_ms;
    int16_t repeat;  // -1 (INFINITE) = sonsuz, 0 = durdur, N = N kez yanar
} LedPattern;

// Some predefined common patterns for convenience
extern const LedPattern STATUS_LED_PATTERN_OFF;
extern const LedPattern STATUS_LED_PATTERN_ON;
extern const LedPattern STATUS_LED_PATTERN_NONE;  // Clears the priority

void status_led_task(void* params);
void status_led_set_pattern(StatusLedPriority priority, LedPattern pattern);
void status_led_clear_pattern(StatusLedPriority priority);
void status_led_test_task(void* params);
void status_led_print_priorities();