#include "task_config.h"

#include "middleware/sys_fault.h"

const TaskConfig USB_TASK_CONFIG = {
    .name = "usb_device",
    .stack_size = 1024,
    .priority = configMAX_PRIORITIES - 1,
    .core_affinity = CORE_1,
};

const TaskConfig INPUT_TASK_CONFIG = {
    .name = "input",
    .stack_size = 512,
    .priority = tskIDLE_PRIORITY + 4,
    .core_affinity = CORE_SMP,
};

const TaskConfig DUCKY_TASK_CONFIG = {
    .name = "ducky",
    .stack_size = 1024,
    .priority = tskIDLE_PRIORITY + 3,
    .core_affinity = CORE_SMP,
};

const TaskConfig NET_MANAGER_TASK_CONFIG = {
    .name = "netmgr",
    .stack_size = 2048,
    .priority = tskIDLE_PRIORITY + 2,
    .core_affinity = CORE_SMP,
};

const TaskConfig GUI_TASK_CONFIG = {
    .name = "gui",
    .stack_size = 1024,
    .priority = tskIDLE_PRIORITY + 2,
    .core_affinity = CORE_SMP,
};

const TaskConfig SLEEP_MANAGER_TASK_CONFIG = {
    .name = "sleep",
    .stack_size = 1024,
    .priority = tskIDLE_PRIORITY + 1,
    .core_affinity = CORE_SMP,
};

// CYW43 Async Context Task Configuration
const TaskConfig RADIO_MANAGER_TASK_CONFIG = {
    .name = "radio",
    .stack_size = 1024,
    .priority = tskIDLE_PRIORITY + 4,
    .core_affinity = CORE_1,
};

TaskHandle_t task_create(const TaskConfig* config, TaskFunction_t func, void* param) {
    ABORT_IF(config == NULL || func == NULL);

    TaskHandle_t handle = NULL;
    BaseType_t ret;
#if configNUM_CORES > 1 && defined(configUSE_CORE_AFFINITY) && configUSE_CORE_AFFINITY
    ret = xTaskCreateAffinitySet(func, config->name, config->stack_size, param, config->priority, config->core_affinity,
                                 &handle);
#else
    ret = xTaskCreate(func, config->name, config->stack_size, param, config->priority, &handle);
#endif

    if (ret != pdPASS) return NULL;
    return handle;
}
