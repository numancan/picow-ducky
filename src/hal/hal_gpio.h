#pragma once

#include <stdint.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"


typedef enum { GPIO_PULL_NONE, GPIO_PULL_UP, GPIO_PULL_DOWN } GpioPull;
typedef enum { GPIO_INPUT, GPIO_OUTPUT } GpioDir;

typedef struct {
    i2c_inst_t* i2c_ins;
    uint8_t sda;
    uint8_t scl;
    bool no_gpio_pull_up;
} GpioI2C;

typedef struct {
    spi_inst_t* spi_ins;
    uint8_t mosi;
    uint8_t miso;
    uint8_t sck;
    uint8_t cs;
    uint32_t baudrate;
} GpioSpi;

typedef enum { GPIO_LOW, GPIO_HIGH } GpioState;

typedef struct {
    const uint8_t pin;
    const uint8_t default_state;
    const uint8_t key;
} GpioInput;

static inline bool hal_gpio_read(uint8_t pin) { return gpio_get(pin); }
static inline void hal_gpio_write(uint8_t pin, bool value) { gpio_put(pin, value); }
static inline void hal_gpio_toggle(uint8_t pin) { gpio_put(pin, !gpio_get(pin)); }

void hal_gpio_init(const uint8_t, const GpioDir, const GpioPull);
void hal_gpio_init_I2C(const GpioI2C*, const uint32_t);
void hal_gpio_init_adc(const uint8_t);
// void hal_gpio_set_irq_callback(uint8_t, uint32_t, gpio_irq_callback_t);