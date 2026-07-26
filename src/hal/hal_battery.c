#include "hal_battery.h"

#include <stdint.h>
#include <stdio.h>

#include "hal.h"
#include "hal_gpio.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

#define BATT_DIVIDER_R_TOP_KOHM (215U)
#define BATT_DIVIDER_R_BOT_KOHM (619U)
#define BATT_ADC_VREF_MV (3300U)
#define BATT_ADC_FULL_SCALE (1U << 12) /* 12-bit ADC */

void hal_battery_init(void) {
    hal_gpio_init(MCP_STAT1_PIN, GPIO_INPUT, GPIO_PULL_UP);
    hal_gpio_init(MCP_STAT2_PIN, GPIO_INPUT, GPIO_PULL_UP);

    hal_gpio_init_adc(BATT_SENSE_PIN);
}

/* MCP73833 truth table (STAT pins are open-drain, pulled HIGH externally):
 *   STAT1=LOW,  STAT2=HIGH  ->  Charge in Progress
 *   STAT1=HIGH, STAT2=LOW   ->  Charge Complete (EOC)
 *   STAT1=LOW,  STAT2=LOW   ->  System Test (LDO) mode
 *   STAT1=HIGH, STAT2=HIGH  ->  No charge (Standby / Shutdown / Fault) */
BatteryChargeState hal_battery_get_charge_state(void) {
    const bool stat1 = hal_gpio_read(MCP_STAT1_PIN); /* true = HIGH, false = LOW */
    const bool stat2 = hal_gpio_read(MCP_STAT2_PIN);

    if (!stat1 && stat2) {
        return BATTERY_CHARGING;
    } else if (stat1 && !stat2) {
        return BATTERY_CHARGE_COMPLETE;
    } else if (!stat1 && !stat2) {
        return BATTERY_SYSTEM_TEST;
    } else {
        return BATTERY_NO_CHARGE;
    }
}

const char* hal_battery_charge_state_str(BatteryChargeState state) {
    switch (state) {
        case BATTERY_CHARGING: return "Charging";
        case BATTERY_CHARGE_COMPLETE: return "Charge Complete";
        case BATTERY_SYSTEM_TEST: return "System Test";
        case BATTERY_NO_CHARGE: return "No Charge";
        default: return "Unknown";
    }
}

uint16_t hal_battery_get_millivolts(void) {
    /* Select ADC channel */
    adc_select_input(BATT_SENSE_PIN - 26);

    const uint16_t raw = adc_read();

    /* Vbat = Vadc * (Rtop + Rbot) / Rbot, in millivolts. Integer math with a
     * 64-bit numerator (raw * 3300 * 834 overflows 32 bits) and round-to-nearest. */
    const uint32_t divider_sum = BATT_DIVIDER_R_TOP_KOHM + BATT_DIVIDER_R_BOT_KOHM;
    const uint32_t denominator = BATT_DIVIDER_R_BOT_KOHM * BATT_ADC_FULL_SCALE;
    const uint64_t numerator = (uint64_t)raw * BATT_ADC_VREF_MV * divider_sum;

    return (uint16_t)((numerator + denominator / 2) / denominator);
}