#pragma once

#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

typedef enum {
    TASK_MANAGER_TASK_GUI,
    TASK_MANAGER_TASK_INPUT,
    TASK_MANAGER_TASK_WEB_SERVER,
    TASK_MANAGER_TASK_DUCKY,
    TASK_MANAGER_TASK_USB_HID,
    TASK_MANAGER_TASK_STATUS_LED,
    TASK_MANAGER_TASK_COUNT
} TaskManagerId;

bool task_manager_start(TaskManagerId id);
bool task_manager_stop(TaskManagerId id);
bool task_manager_is_running(TaskManagerId id);
bool task_manager_set_priority(TaskManagerId id, UBaseType_t priority);
void task_manager_report_stopped(TaskManagerId id);
