#pragma once

#include <float.h>
#include <stdint.h>

/**
 * @brief MCP73833 charge cycle state decoded from STAT1 and STAT2 pins.
 *
 * NOTE: PG pin is not connected on this hardware. Therefore the following
 * states cannot be distinguished from each other and are all reported as
 * BATTERY_NO_CHARGE:
 *   - Shutdown  (VDD < VUVLO, STAT1=Hi-Z, STAT2=Hi-Z)
 *   - Standby   (VBAT >= VREG+100mV, STAT1=Hi-Z, STAT2=Hi-Z)
 *   - Temperature Fault (STAT1=Hi-Z, STAT2=Hi-Z)
 *   - Timer Fault       (STAT1=Hi-Z, STAT2=Hi-Z)
 *
 * MCP73833 STAT pin truth table (from datasheet):
 *   STAT1=L,     STAT2=Hi-Z  ->  Charging
 *   STAT1=Hi-Z,  STAT2=L     ->  Charge Complete (EOC)
 *   STAT1=L,     STAT2=L     ->  System Test Mode (LDO)
 *   STAT1=Hi-Z,  STAT2=Hi-Z  ->  No charge (Standby / Shutdown / Fault)
 *
 * STAT pins are open-drain. External pull-up resistors are present on the
 * picow-ducky board so Hi-Z reads as HIGH and driven LOW reads as LOW.
 */
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
float hal_battery_get_voltage(void);
