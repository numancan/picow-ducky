#pragma once

#include <stdint.h>

/**
STAT pins are open-drain. External pull-up resistors are present on the board so
Hi-Z reads as HIGH and driven LOW reads as LOW. */
typedef enum {
    BATTERY_CHARGING,        /**< Charge in progress (all CC/CV phases)    */
    BATTERY_CHARGE_COMPLETE, /**< Charge complete (EOC)                     */
    BATTERY_SYSTEM_TEST,     /**< System test (LDO) mode                    */
    BATTERY_NO_CHARGE,       /**< Standby / Shutdown / Fault (indeterminate
                                  without PG pin)                           */
} BatteryChargeState;

void hal_battery_init(void);
BatteryChargeState hal_battery_get_charge_state(void);
const char* hal_battery_charge_state_str(BatteryChargeState state);

/* Battery voltage in millivolts (integer; no FPU on the M0+ core). */
uint16_t hal_battery_get_millivolts(void);
