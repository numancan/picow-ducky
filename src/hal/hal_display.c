#include "hal_display.h"

#include <stdio.h>
#include <stdlib.h>

#include "hal.h"
#include "pico/stdlib.h"

uint8_t u8g2_gpio_and_delay_cb(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    switch (msg) {
        case U8X8_MSG_DELAY_NANO: asm("nop"); break;
        case U8X8_MSG_DELAY_100NANO: asm("nop"); break;
        case U8X8_MSG_DELAY_10MICRO: sleep_us(10); break;
        case U8X8_MSG_DELAY_MILLI: sleep_ms(arg_int); break;
        case U8X8_MSG_DELAY_I2C: sleep_us(5); break;
        default: u8x8_SetGPIOResult(u8x8, 1); break;
    }
    return 1;
}

static uint8_t u8g2_hw_i2c_cb(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t buffer[32];
    static uint8_t buf_idx;
    uint8_t* data;

    switch (msg) {
        case U8X8_MSG_BYTE_SEND:
            data = (uint8_t*)arg_ptr;
            while (arg_int > 0) {
                buffer[buf_idx++] = *data;
                data++;
                arg_int--;
            }
            break;
        case U8X8_MSG_BYTE_INIT:
            /* add your custom code to init i2c subsystem */
            break;
        case U8X8_MSG_BYTE_SET_DC: break;
        case U8X8_MSG_BYTE_START_TRANSFER: buf_idx = 0; break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            i2c_write_blocking(gpio_ssd1306.i2c_ins, (SSD1306_I2C_ADDR & 0xFE), buffer, buf_idx, false);
            break;
        default: return 0;
    }
    return 1;
}

void hal_display_init(u8g2_t* u8g2) {
    u8g2_Setup_ssd1306_i2c_128x32_univision_f(u8g2, U8G2_R0, u8g2_hw_i2c_cb, u8g2_gpio_and_delay_cb);
    u8g2_InitDisplay(u8g2);
}

void hal_display_on(u8g2_t* u8g2) { u8g2_SetPowerSave(u8g2, 0); }
void hal_display_off(u8g2_t* u8g2) { u8g2_SetPowerSave(u8g2, 1); }