#include "usb_device.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "config/task_config.h"
#include "middleware/log.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "tusb.h"

static const char* TAG = "USB_DEVICE";

// tud_task_ext() to observe the stop request when the transport is torn down.
#define USB_TASK_POLL_MS 10

// Single scalar shared with a lock-free reader (usb_device_is_running, called
// from the ducky task); relaxed load/store per CLAUDE.md §4.
static _Atomic(TaskHandle_t) usb_task_handle = NULL;

// USB Device Driver task
void usb_device_task(void* param) {
    (void)param;

    tud_init(BOARD_TUD_RHPORT);

    while (1) {
        tud_task_ext(USB_TASK_POLL_MS, false);
        if (ulTaskNotifyTake(pdTRUE, 0)) break;
    }

    tud_disconnect();
    vTaskDelay(pdMS_TO_TICKS(10));
    tud_deinit(BOARD_TUD_RHPORT);

    atomic_store_explicit(&usb_task_handle, NULL, memory_order_relaxed);
    vTaskDelete(NULL);  // TODO: dont delete task just stop
}

bool usb_device_start(void) {
    if (atomic_load_explicit(&usb_task_handle, memory_order_relaxed) != NULL) return true;

    TaskHandle_t handle = task_create(&USB_TASK_CONFIG, usb_device_task, NULL);
    if (handle == NULL) return false;

    atomic_store_explicit(&usb_task_handle, handle, memory_order_relaxed);
    return true;
}

void usb_device_request_task_stop(void) {
    TaskHandle_t handle = atomic_load_explicit(&usb_task_handle, memory_order_relaxed);
    if (handle != NULL) xTaskNotifyGive(handle);
}

bool usb_device_is_running(void) { return atomic_load_explicit(&usb_task_handle, memory_order_relaxed) != NULL; }

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) { LOG_INFO(TAG, "USB Device Mounted"); }

// Invoked when device is unmounted
void tud_umount_cb(void) { LOG_INFO(TAG, "USB Device Unmounted"); }

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    LOG_INFO(TAG, "USB Device Suspended");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) { LOG_INFO(TAG, "USB Device Resumed"); }
