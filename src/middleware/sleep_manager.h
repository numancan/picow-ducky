#pragma once

// Coordinates a clean shutdown of every registered task before the MCU enters
// its lowest low-power state, then reboots on wake. Tasks register a shutdown
// callback and acknowledge once their cleanup is done; only then does the
// sleep_manager put the device to sleep.

// Shutdown trigger for a registered participant. Invoked from the sleep_manager
// task when a sleep is requested. Two flavors, by how the participant was
// registered:
//  - sleep_manager_register (task participant): must return promptly, only
//    signalling the participant's own task to begin cleanup (a task
//    notification or a queued event). That task acks when its cleanup is done.
//  - sleep_manager_register_finalizer: does its teardown synchronously here and
//    does NOT ack. Runs after every task participant has acked, so it is the
//    safe place to tear down a shared resource (e.g. unmount the SD card) once
//    no task can still be using it.
typedef void (*SleepShutdownFn)(void* context);

// Must run at the very top of main(), before hal_init and the scheduler. If the
// previous run armed a sleep, this drops the (now single-core, scheduler-less)
// MCU into dormant and reboots on wake; otherwise it returns immediately and
// normal boot continues.
void sleep_manager_boot_check(void);

// Creates the request queue, the ack semaphore and the sleep_manager task.
// Call once from main() BEFORE the other module _init()s so they can register
// during their own init, and before the scheduler starts.
void sleep_manager_init(void);

// Register a task to be shut down before sleep. Boot-time wiring only: call
// during module init, before any sleep can be requested.
void sleep_manager_register(SleepShutdownFn shutdown_fn, void* context);

// Register a teardown that must run LAST, after all task participants have
// acked -- for shared resources like the SD card that a task might still be
// using while it cleans up. Runs synchronously in the sleep_manager task and
// does not ack. Boot-time wiring only.
void sleep_manager_register_finalizer(SleepShutdownFn shutdown_fn, void* context);

// Request the coordinated sleep sequence. Safe from any task (never an ISR).
void sleep_manager_request_sleep(void);

// Acknowledge that the calling task has finished its cleanup. Every registered
// participant must call this exactly once, at the very end of its task.
void sleep_manager_ack_shutdown(void);

void sleep_manager_task(void* arg);
