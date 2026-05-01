#include "hal/hal.h"
#include "hw_config.h"

// static sd_card_t *sd_card = NULL;

static spi_t spi = {.hw_inst = SD_CARD_SPI_INST,
                    .sck_gpio = SD_CARD_SCK_PIN,
                    .mosi_gpio = SD_CARD_MOSI_PIN,
                    .miso_gpio = SD_CARD_MISO_PIN,
                    .set_drive_strength = true,
                    .mosi_gpio_drive_strength = GPIO_DRIVE_STRENGTH_4MA,
                    .sck_gpio_drive_strength = GPIO_DRIVE_STRENGTH_2MA,
                    .baud_rate = SD_CARD_BAUDRATE,
                    .spi_mode = 0};

static sd_spi_if_t spi_interface = {.spi = &spi, .ss_gpio = SD_CARD_CS_PIN};

static sd_card_t sd_card = {
    .type = SD_IF_SPI,
    .spi_if_p = &spi_interface,
    .use_card_detect = false,
    .card_detect_gpio = 99,
    .card_detected_true = 0,
    .card_detect_use_pull = false,
    .card_detect_pull_hi = false,
};

size_t sd_get_num() { return 1; }

sd_card_t *sd_get_by_num(size_t num) { return &sd_card; }
