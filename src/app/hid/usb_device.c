#include "usb_device.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "app/task_manager/task_manager.h"
#include "bsp/board_api.h"
#include "middleware/log.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "tusb.h"

static const char* TAG = "USB_DEVICE";

// tud_task_ext() to observe the stop request when the transport is torn down.
#define USB_TASK_POLL_MS 10

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

    task_manager_report_stopped(TASK_MANAGER_TASK_USB_DEVICE);
    vTaskDelete(NULL);
}

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
