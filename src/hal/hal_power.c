#include "hal_power.h"

#include <hardware/regs/addressmap.h>
#include <hardware/regs/m0plus.h>
#include <hardware/resets.h>
#include <pico/runtime_init.h>
#include <stdio.h>

#include "hal.h"
#include "hardware/clocks.h"
#include "hardware/xosc.h"
#include "pico/sleep.h"
#include "pico/stdlib.h"

static void sleep_callback(uint gpio, uint32_t events) {
    // This is just to wake up the processor from sleep_run_from_xosc
}

void hal_power_deep_sleep(void) {
    printf("Entering sleep mode... Press BOTH SELECT and DOWN to wake.\n");
    uart_default_tx_wait_blocking();

    printf("Switching to XOSC\n");
    uart_default_tx_wait_blocking();

    // Set the crystal oscillator as the dormant clock source, UART will be reconfigured from here
    // This is necessary before sending the pico into dormancy
    sleep_run_from_xosc();

    printf("Going dormant until GPIO %d goes edge high\n", BUTTON_SELECT_PIN);
    uart_default_tx_wait_blocking();

    // Go to sleep until we see a high edge on GPIO 10
    sleep_goto_dormant_until_edge_high(BUTTON_SELECT_PIN);

    // Re-enabling clock sources and generators.
    sleep_power_up();

    // Restore clocks and peripherals
    clocks_init();
    stdio_init_all();

    printf("Welcome back!\n");
}

void hal_power_reboot(void) {
    // reset USB controller
    reset_block(RESETS_WDSEL_USBCTRL_BITS);
    // TODO: add other perpherials here if you use them

    // reset the CPU
    volatile uint32_t* AIRCR_register = (volatile uint32_t*)(PPB_BASE + M0PLUS_AIRCR_OFFSET);
    // From datasheet:
    // 31:16 VECTKEY: On writes, write 0x05FA to VECTKEY, otherwise the write is ignored.
    // 15 ENDIANESS: 0 = Little-endian.
    // 14:3 Reserved
    // 2 SYSRESETREQ: Writing 1 to this bit causes the SYSRESETREQ signal to the outer system to be asserted to request
    // a reset. 1 VECTCLRACTIVE: not relevant here
    *AIRCR_register = (0x05FA << M0PLUS_AIRCR_VECTKEY_LSB) | M0PLUS_AIRCR_SYSRESETREQ_BITS;
}
