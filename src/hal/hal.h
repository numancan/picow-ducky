#pragma once

#include "hardware/i2c.h"
#include "hal_gpio.h"

extern const GpioI2C gpio_ssd1306;
extern const GpioSpi gpio_sd_card;
extern const GpioInput gpio_inputs[];
extern const uint8_t gpio_input_count;

typedef enum { INPUT_KEY_SELECT, INPUT_KEY_DOWN } InputKey;

#define BUTTON_DOWN_PIN         (11)
#define BUTTON_SELECT_PIN       (18)

#define STATUS_LED_PIN          (12)

#define SSD1306_I2C_INST        (i2c0)
#define SSD1306_SDA_PIN         (8)
#define SSD1306_SCL_PIN         (9)
#define SSD1306_I2C_CLK_HZ      (400 * 1000)
#define SSD1306_I2C_ADDR        (_u(0x3C))

#define SD_CARD_SPI_INST        (spi0)
#define SD_CARD_MOSI_PIN        (4)
#define SD_CARD_MISO_PIN        (7)
#define SD_CARD_SCK_PIN         (6)
#define SD_CARD_CS_PIN          (5)
#define SD_CARD_BAUDRATE        (31250000U) // 125 * 1000 * 1000 / 4

#define BATT_SENSE_PIN          (26)

#define MCP_STAT1_PIN           (3)
#define MCP_STAT2_PIN           (2)

void hal_init();