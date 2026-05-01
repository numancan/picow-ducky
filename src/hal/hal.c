#include "hal.h"

#include <stdio.h>

#include "hal_battery.h"
#include "hal_gpio.h"
#include "hal_sd.h"

const GpioI2C gpio_ssd1306 = {.i2c_ins = SSD1306_I2C_INST, .sda = SSD1306_SDA_PIN, .scl = SSD1306_SCL_PIN};

const GpioSpi gpio_sd_card = {.spi_ins = SD_CARD_SPI_INST,
                              .mosi = SD_CARD_MOSI_PIN,
                              .miso = SD_CARD_MISO_PIN,
                              .sck = SD_CARD_SCK_PIN,
                              .cs = SD_CARD_CS_PIN,
                              .baudrate = SD_CARD_BAUDRATE};

const GpioInput gpio_inputs[] = {{.pin = BUTTON_SELECT_PIN, .default_state = GPIO_LOW, .key = INPUT_KEY_SELECT},
                                 {.pin = BUTTON_DOWN_PIN, .default_state = GPIO_LOW, .key = INPUT_KEY_DOWN}};

const uint8_t gpio_input_count = count_of(gpio_inputs);

void hal_init() {
    // Initialization of inputs
    for (size_t i = 0; i < gpio_input_count; i++) {
        hal_gpio_init(gpio_inputs[i].pin, GPIO_INPUT, GPIO_PULL_NONE);
    }

    hal_sd_card_init();
    hal_gpio_init_I2C(&gpio_ssd1306, 400 * 1000);
    hal_battery_init();
    
    hal_gpio_init(STATUS_LED_PIN, GPIO_OUTPUT, GPIO_PULL_NONE);
    hal_gpio_write(STATUS_LED_PIN, false);
}