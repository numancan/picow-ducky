#include "usb_device.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp/board_api.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

// Increase stack size when debug log is enabled
#define USBD_STACK_SIZE (3 * configMINIMAL_STACK_SIZE / 2) * (CFG_TUSB_DEBUG ? 2 : 1)

// CFG_TUSB_OS

void usb_device_init() { board_init(); }

// USB Device Driver task
void usb_device_task(void* param) {
    (void)param;

    // This should be called after scheduler/kernel is started.
    // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
    tusb_rhport_init_t dev_init = {.role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO};
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    if (board_init_after_tusb) {
        board_init_after_tusb();
    }

    // TickType_t wake = xTaskGetTickCount();
    while (1) {
        tud_task();

        // vTaskDelay(pdMS_TO_TICKS(1));
        // if (tud_suspended() || !tud_connected()) xTaskDelayUntil(&wake, pdMS_TO_TICKS(1));
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) { printf("USB Device Mounted\n"); }

// Invoked when device is unmounted
void tud_umount_cb(void) { printf("USB Device Unmounted\n"); }

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
    printf("USB Device Suspended\n");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) { printf("USB Device Resumed\n"); }
