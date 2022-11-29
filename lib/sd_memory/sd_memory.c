#include <stdio.h>

#include "f_util.h"
#include "diskio.h"
#include "sd_memory.h"

/*------------------- HARDWARE CONFIGURATION OF SPI -------------------*/
void spi0_dma_isr();

// Hardware Configuration of SPI "objects"
static spi_t spi = {  // One for each SPI.
        .hw_inst = spi0,  // SPI component
        .miso_gpio = 0, // GPIO number (not pin number)
        .mosi_gpio = 3,
        .sck_gpio = 2,

        /* The choice of SD card matters! SanDisk runs at the highest speed. PNY
           can only mangage 5 MHz. Those are all I've tried. */
        //.baud_rate = 1000 * 1000,
        .baud_rate = 12500 * 1000,  // The limitation here is SPI slew rate.
        //.baud_rate = 25 * 1000 * 1000, // Actual frequency: 20833333. Has
        // worked for me with SanDisk.

        .dma_isr = spi0_dma_isr
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_card = {
        .pcName = "0:",           // Name used to mount device
        .spi = &spi,              // Pointer to the SPI driving this card
        .ss_gpio = 1,             // The SPI slave select GPIO for this SD card
        .use_card_detect = true,
        .card_detect_gpio = 22,   // Card detect
        .card_detected_true = 1,  // What the GPIO read returns when a card is
                                  // present. Use -1 if there is no card detect.
        .m_Status = STA_NOINIT
};

void spi0_dma_isr() { spi_irq_handler(&spi); }

size_t sd_get_num() { return 1; }
size_t spi_get_num() { return 1; }

sd_card_t *sd_get_by_num(size_t num) { return &sd_card; }
spi_t *spi_get_by_num(size_t num) { return &spi; }

/*--------------------------------------------------------------------*/

static sd_card_t *pSD = &sd_card;
FRESULT temp_fresult;
FIL temp_fil;

FIL *sdm_get_cur_fil() { return &temp_fil; }

FRESULT sdm_mount()
{
  temp_fresult = f_mount(&pSD->fatfs, pSD->pcName, 1);
  if (temp_fresult != FR_OK) printf("f_mount error: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);

  return temp_fresult;
}

FRESULT sdm_open_read(char* filename)
{
  
  temp_fresult = f_open(&temp_fil, filename, FA_READ);

  if (temp_fresult != FR_OK) {
      printf("ERROR: Could not open file (%d)\r\n", temp_fresult);
  }

  return temp_fresult;
}

FRESULT sdm_close_file()
{
  temp_fresult = f_close(&temp_fil);

  if (temp_fresult != FR_OK) {
    printf("f_close error: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);  
  }

  return temp_fresult;
}

FRESULT sdm_unmount()
{
  temp_fresult = f_mount(0, pSD->pcName, 0);
  if (temp_fresult != FR_OK) printf("f_umount error: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);

  return temp_fresult;
}