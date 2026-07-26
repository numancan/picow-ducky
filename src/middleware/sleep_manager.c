#include "sleep_manager.h"

#include <stdbool.h>

#include "FreeRTOS.h"
#include "config/task_config.h"
#include "hal/hal.h" /* BUTTON_SELECT_PIN */
#include "hal/hal_gpio.h"
#include "hardware/gpio.h"
#include "hardware/structs/watchdog.h"
#include "hardware/watchdog.h"
#include "middleware/log.h"
#include "middleware/sys_fault.h"
#include "pico/low_power.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define SLEEP_MAX_PARTICIPANTS 8
#define SLEEP_REQUEST_QUEUE_LEN 1
#define SLEEP_SHUTDOWN_TIMEOUT_MS 3000

// watchdog scratch[0..3] are free for application use; scratch[4..7] are
// reserved by the SDK's reboot path. One register carries the boot phase.
#define SLEEP_SCRATCH_PHASE 0
#define SLEEP_MAGIC_ENTER 0x50534C50u /* "PSLP" */

static const char* TAG = "SLEEP";

typedef enum {
    SLEEP_REQ_SLEEP = 0,
} SleepRequest;

typedef struct {
    SleepShutdownFn shutdown_fn;
    void* context;
    bool is_finalizer; /* true: run last, synchronously, after all task acks */
} SleepParticipant;

// All registration happens single-threaded during init before the scheduler
// starts, so the registry needs no lock.
static SleepParticipant participants[SLEEP_MAX_PARTICIPANTS];
static size_t participant_count = 0;

static QueueHandle_t request_queue = NULL;
static SemaphoreHandle_t ack_sem = NULL;

static void sleep_manager_add(SleepShutdownFn shutdown_fn, void* context, bool is_finalizer) {
    ABORT_IF(shutdown_fn == NULL);
    ABORT_IF(participant_count >= SLEEP_MAX_PARTICIPANTS);

    participants[participant_count].shutdown_fn = shutdown_fn;
    participants[participant_count].context = context;
    participants[participant_count].is_finalizer = is_finalizer;
    participant_count++;
}

void sleep_manager_register(SleepShutdownFn shutdown_fn, void* context) {
    sleep_manager_add(shutdown_fn, context, false);
}

void sleep_manager_register_finalizer(SleepShutdownFn shutdown_fn, void* context) {
    sleep_manager_add(shutdown_fn, context, true);
}

void sleep_manager_request_sleep(void) {
    SleepRequest req = SLEEP_REQ_SLEEP;
    xQueueSend(request_queue, &req, 0);
}

void sleep_manager_ack_shutdown(void) { xSemaphoreGive(ack_sem); }

// Hand control to a clean context before sleeping: reboot so the dormant call
// runs from bare main() (single core, no scheduler), where no core1 or tick
// activity can race the clock reconfiguration. Runs only after every
// participant has acked (or the shutdown timeout fired).
static void sleep_manager_reboot_into_sleep(void) {
    LOG_INFO(TAG, "Arming sleep; rebooting into bare-metal dormant entry");
    watchdog_hw->scratch[SLEEP_SCRATCH_PHASE] = SLEEP_MAGIC_ENTER;
    watchdog_reboot(0, 0, 0);
    for (;;);
}

static void sleep_manager_handle_sleep(void) {
    LOG_INFO(TAG, "Sleep requested; shutting down tasks");

    // Phase 1: shut the task participants down one at a time, in reverse
    // registration order, waiting for each to ack before signalling the next.
    // Sequencing (not just ordering the signals) is what makes teardown safe: a
    // consumer may touch a lower-level task's resources during its own cleanup
    // -- e.g. gui reads from the event queue owned by input, so input must stay
    // alive (and keep that queue) until gui has stopped reading. A per-task
    // timeout keeps one stuck task from blocking sleep forever.
    for (size_t i = participant_count; i-- > 0;) {
        if (participants[i].is_finalizer) continue;

        participants[i].shutdown_fn(participants[i].context);

        if (xSemaphoreTake(ack_sem, pdMS_TO_TICKS(SLEEP_SHUTDOWN_TIMEOUT_MS)) != pdTRUE) {
            LOG_WARN(TAG, "Shutdown ack timeout for participant %u", (unsigned)i);
        }
    }

    // Phase 2: finalizers run last, synchronously, now that every task has
    // finished. They do not ack.
    for (size_t i = participant_count; i-- > 0;) {
        if (participants[i].is_finalizer) participants[i].shutdown_fn(participants[i].context);
    }

    sleep_manager_reboot_into_sleep();
}

void sleep_manager_task(void* arg) {
    (void)arg;

    SleepRequest req;
    for (;;) {
        if (xQueueReceive(request_queue, &req, portMAX_DELAY) == pdTRUE) {
            if (req == SLEEP_REQ_SLEEP) sleep_manager_handle_sleep();
        }
    }
}

void sleep_manager_boot_check(void) {
    if (watchdog_hw->scratch[SLEEP_SCRATCH_PHASE] != SLEEP_MAGIC_ENTER) return;

    // Reached only via sleep_manager_reboot_into_sleep(): we are in bare main()
    // now -- single core, no scheduler, no tick ISR -- the safe context to stop
    // the clocks. RP2040 has no Pstate, so dormant is the lowest state; ROSC
    // needs no external clock and a GPIO wake needs no running clock.
    hal_gpio_init(BUTTON_SELECT_PIN, GPIO_INPUT, GPIO_PULL_NONE);

    low_power_dormant_until_gpio_pin_state(BUTTON_SELECT_PIN, /*edge*/ true, /*high*/ true, DORMANT_CLOCK_SOURCE_ROSC,
                                           NULL);

    // Woken: clear the phase so the next boot is normal (not another sleep) and
    // reboot for a pristine clock tree.
    watchdog_hw->scratch[SLEEP_SCRATCH_PHASE] = 0;
    watchdog_reboot(0, 0, 0);
    for (;;);
}

void sleep_manager_init(void) {
    request_queue = xQueueCreate(SLEEP_REQUEST_QUEUE_LEN, sizeof(SleepRequest));
    ack_sem = xSemaphoreCreateCounting(SLEEP_MAX_PARTICIPANTS, 0);
    PANIC_IF(request_queue == NULL || ack_sem == NULL, "sleep: init alloc failed");

    PANIC_IF(task_create(&SLEEP_MANAGER_TASK_CONFIG, sleep_manager_task, NULL) == NULL, "sleep task create failed");
}
