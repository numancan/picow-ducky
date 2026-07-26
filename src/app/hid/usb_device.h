#pragma once

#include <stdbool.h>
#include <stdint.h>

void usb_device_task(void* param);

// Create the USB device task if it isn't already running. Returns false if
// task creation failed. Idempotent while the task is alive.
bool usb_device_start(void);

// Ask the running USB device task to tear itself down (async). No-op if not running.
void usb_device_request_task_stop(void);

// True while the USB device task exists.
bool usb_device_is_running(void);
