#include "hal_display.h"

#include <stdio.h>
#include <stdlib.h>

#include "hal.h"
#include "pico/stdlib.h"

#define SSD1306_I2C_CLK_HZ 400 * 1000
#define SSD1306_I2C_ADDR _u(0x3C)

// TODO: check for freertos
uint8_t u8g2_gpio_and_delay_cb(u8x8_t* u8x8, uint8_t msg, uint8_t arg_int, void* arg_ptr) {
    switch (msg) {
        case U8X8_MSG_DELAY_NANO:
            asm("nop");
            break;
        case U8X8_MSG_DELAY_100NANO:
            // sleep_us(1000 * 100 * arg_int);
            asm("nop");
            break;
        case U8X8_MSG_DELAY_10MICRO:
            sleep_us(10);
            break;
        case U8X8_MSG_DELAY_MILLI:
            sleep_ms(arg_int);
            // vTaskDelay(pdMS_TO_TICKS(arg_int));
            break;
        case U8X8_MSG_DELAY_I2C:
            sleep_us(5);
            break;  // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
        case U8X8_MSG_GPIO_MENU_SELECT:
            u8x8_SetGPIOResult(u8x8, !gpio_get(BUTTON_SELECT_PIN));
            // u8x8_SetGPIOResult(u8x8, 1);
            break;
        case U8X8_MSG_GPIO_MENU_NEXT:
            u8x8_SetGPIOResult(u8x8, !gpio_get(BUTTON_DOWN_PIN));
            // u8x8_SetGPIOResult(u8x8, 1);
            break;
        case U8X8_MSG_GPIO_MENU_PREV:
            // u8x8_SetGPIOResult(u8x8, !gpio_get(9));
            u8x8_SetGPIOResult(u8x8, 1);
            break;
        case U8X8_MSG_GPIO_MENU_HOME:
            u8x8_SetGPIOResult(u8x8, 1);
            // u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
            break;
        default:
            u8x8_SetGPIOResult(u8x8, 1);  // default return value
            break;
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
        case U8X8_MSG_BYTE_SET_DC:
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            buf_idx = 0;
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            i2c_write_blocking(gpio_ssd1306.i2c_ins, (SSD1306_I2C_ADDR & 0xFE), buffer, buf_idx, false);
            break;
        default:
            return 0;
    }
    return 1;
}

void hal_display_init(u8g2_t* u8g2) {
    // hal_gpio_init_I2C(&gpio_ssd1306, SSD1306_I2C_CLK_HZ);

    u8g2_Setup_ssd1306_i2c_128x32_univision_f(u8g2, U8G2_R0, u8g2_hw_i2c_cb, u8g2_gpio_and_delay_cb);
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);
}