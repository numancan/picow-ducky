
#pragma once

#include "FreeRTOS.h"
#include "task.h"

enum {
    CORE_SMP = tskNO_AFFINITY,
    CORE_0 = (1 << 0),
    CORE_1 = (1 << 1),
};

typedef struct {
    const char* name;
    uint32_t stack_size;
    UBaseType_t priority;
    UBaseType_t core_affinity;
} TaskConfig;

extern const TaskConfig USB_TASK_CONFIG;
extern const TaskConfig INPUT_TASK_CONFIG;
extern const TaskConfig DUCKY_TASK_CONFIG;
extern const TaskConfig NET_MANAGER_TASK_CONFIG;
extern const TaskConfig GUI_TASK_CONFIG;
extern const TaskConfig SLEEP_MANAGER_TASK_CONFIG;
extern const TaskConfig RADIO_MANAGER_TASK_CONFIG;

// Create a FreeRTOS task from a config. Returns the task handle, or NULL if
// creation failed. Applies core affinity when the SMP build supports it.
TaskHandle_t task_create(const TaskConfig* config, TaskFunction_t func, void* param);
