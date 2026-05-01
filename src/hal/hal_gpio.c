#include "hal_gpio.h"

#include "pico/stdlib.h"

void hal_gpio_init(const uint8_t pin, const GpioDir dir, const GpioPull pull) {
    gpio_init(pin);
    gpio_set_dir(pin, dir);

    switch (pull) {
        case GPIO_PULL_DOWN:
            gpio_pull_down(pin);
            break;

        case GPIO_PULL_UP:
            gpio_pull_up(pin);
            break;

        default:
            break;
    }
}

void hal_gpio_init_I2C(const GpioI2C* gpio_i2c, const uint32_t baudHz) {
    i2c_init(gpio_i2c->i2c_ins, baudHz);

    gpio_set_function(gpio_i2c->scl, GPIO_FUNC_I2C);
    gpio_set_function(gpio_i2c->sda, GPIO_FUNC_I2C);

    gpio_pull_up(gpio_i2c->scl);
    gpio_pull_up(gpio_i2c->sda);
}

void hal_gpio_init_adc(const uint8_t pin) {
    adc_gpio_init(pin);
    adc_select_input(pin - 26);
}

// void hal_gpio_set_irq_callback(uint8_t gpio, uint32_t irq_level,
//                                gpio_irq_callback_t callback) {
//   gpio_set_irq_enabled_with_callback(gpio, irq_level, true, callback);
// }