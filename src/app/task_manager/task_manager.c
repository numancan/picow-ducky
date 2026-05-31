#include "task_manager.h"

#include <stdio.h>

// Headers for task functions
#include "app/duckyscript/ducky.h"
#include "app/gui/gui.h"
#include "app/hid/usb_device.h"
#include "app/status_led/status_led.h"
#include "app/web_server/web_server.h"
#include "middleware/input.h"

typedef struct {
    TaskFunction_t func;
    const char* name;
    uint32_t stack_size;
    UBaseType_t default_prio;
    TaskHandle_t handle;
    bool notify_on_stop;
    void (*init_func)(void);  // Called once before task starts, may be NULL
} task_config_t;

static task_config_t tasks[TASK_MANAGER_TASK_COUNT] = {
    [TASK_MANAGER_TASK_USB_HID] = {usb_device_task, "usb_device", 1024, configMAX_PRIORITIES - 1, NULL, false,
                                   usb_device_init},
    [TASK_MANAGER_TASK_INPUT] = {input_task, "input", 512, tskIDLE_PRIORITY + 4, NULL, true, input_init},
    [TASK_MANAGER_TASK_DUCKY] = {ducky_task, "ducky", 1024, tskIDLE_PRIORITY + 3, NULL, true, ducky_init},
    [TASK_MANAGER_TASK_WEB_SERVER] = {web_server_task, "webserver", 1024, tskIDLE_PRIORITY + 2, NULL, true, NULL},
    [TASK_MANAGER_TASK_GUI] = {gui_task, "gui", 1024, tskIDLE_PRIORITY + 2, NULL, true, NULL},
    [TASK_MANAGER_TASK_STATUS_LED] = {status_led_task, "status_led", 512, tskIDLE_PRIORITY + 1, NULL, false, NULL},
};

bool task_manager_start(TaskManagerId id) {
    if (id >= TASK_MANAGER_TASK_COUNT) return false;
    if (tasks[id].handle != NULL) return true;  // Already running

    if (tasks[id].init_func != NULL) {
        tasks[id].init_func();
    }

    BaseType_t ret = xTaskCreate(tasks[id].func, tasks[id].name, tasks[id].stack_size,
                                 (void*)(uintptr_t)id,  // Pass ID as parameter if needed
                                 tasks[id].default_prio, &tasks[id].handle);

    if (ret == pdPASS) {
        printf("[TM] Started task: %s\n", tasks[id].name);
        return true;
    }
    return false;
}

bool task_manager_stop(TaskManagerId id) {
    if (id >= TASK_MANAGER_TASK_COUNT) return false;
    if (tasks[id].handle == NULL) return true;  // Already stopped

    if (tasks[id].notify_on_stop) {
        // Graceful stop: Send notification
        printf("[TM] Sending stop signal to: %s\n", tasks[id].name);
        xTaskNotifyGive(tasks[id].handle);
    } else {
        // Forced stop: Delete task
        printf("[TM] Deleting task: %s\n", tasks[id].name);
        vTaskDelete(tasks[id].handle);
        tasks[id].handle = NULL;
    }
    return true;
}

bool task_manager_is_running(TaskManagerId id) {
    if (id >= TASK_MANAGER_TASK_COUNT) return false;
    return tasks[id].handle != NULL;
}

bool task_manager_set_priority(TaskManagerId id, UBaseType_t priority) {
    if (id >= TASK_MANAGER_TASK_COUNT || tasks[id].handle == NULL) return false;
    vTaskPrioritySet(tasks[id].handle, priority);
    return true;
}

void task_manager_report_stopped(TaskManagerId id) {
    if (id < TASK_MANAGER_TASK_COUNT) {
        printf("[TM] Task reported stopped: %s\n", tasks[id].name);
        tasks[id].handle = NULL;
    }
}
