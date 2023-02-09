#include <stdio.h>
#include <string.h>

#include "sd_memory.h"
#include "f_util.h"
#include "diskio.h"
#include "ffconf.h"

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
static FRESULT temp_fresult;
static FIL temp_fil;
static DIR temp_dir;
static FILINFO temp_finfo;

FIL *sdm_get_cur_fil() { return &temp_fil; }

FRESULT sdm_mount()
{
  temp_fresult = f_mount(&pSD->fatfs, pSD->pcName, 1);
  
  printf("sdm_mount: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);

  return temp_fresult;
}

FRESULT sdm_open_read(char* fname)
{
  temp_fresult = f_open(&temp_fil, fname, FA_READ);
      FRESULT fr;
    TCHAR str[126];

    fr = f_getcwd(str, 126);  /* Get current directory path */
  printf("%s\n", fr);

  printf("sdm_open_read: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);

  return temp_fresult;
}

FRESULT sdm_open_write(char* fname)
{
  temp_fresult = f_open(&temp_fil, fname, FA_WRITE | FA_CREATE_ALWAYS);

  printf("sdm_open_write: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);

  return temp_fresult;
}

FRESULT sdm_close_file()
{
  temp_fresult = f_close(&temp_fil);
  
  printf("sdm_close_file: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);

  memset(&temp_fil, 0, sizeof(temp_fil));
  return temp_fresult;
}

FRESULT sdm_unmount()
{
  temp_fresult = f_mount(0, pSD->pcName, 0);
 
  printf("sdm_unmount: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);

  return temp_fresult;
}

/** TODO: bu max file name 256 çok ya bunu override etmek lazım ve heap kullanıyor */
FRESULT sdm_read_dir(TCHAR *path, uint8_t *fnames, size_t n)
{
  temp_fresult = f_opendir(&temp_dir, path);

  snprintf(fnames, n, "");

  if (temp_fresult == FR_OK) {
    while(f_readdir(&temp_dir, &temp_finfo) == FR_OK && temp_finfo.fname[0] && n > 0) { //TODO: ya bu readdir resulttu kaydetmiyoruz?
      strncat(fnames, temp_finfo.fname, n);

      size_t fname_size = strlen(temp_finfo.fname);
      n -= fname_size;
      memset(temp_finfo.fname, 0, fname_size);
    }

    temp_fresult = f_closedir(&temp_dir);
  }

  printf("sdm_read_dir: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);
  
  return temp_fresult;
}

// TODO: bu fonksiyon hoş değil kontrol yok belki strtok ile yapılabilir bilemedim reis
FRESULT sdm_read_until(char *buffer, char delim, int16_t len)
{
  char s[2];
  UINT br;
  size_t i = 0;
  
  while (1)
  {
    f_read(&temp_fil, s, 1, &br);
    if (br != 1) return FR_INT_ERR;

    if (s[0] == delim) { buffer[i] = '\0'; return FR_OK;}

    if (!(s[0] == '\n' || s[0] == '\r') && len-- > 0) buffer[i++] = s[0];
  }

  return FR_OK;
}

FRESULT sdm_remove_payload_file(TCHAR *filename)
{
  if (!sdm_check_file_ext(filename, "txt")) return FR_INVALID_PARAMETER;
  
  temp_fresult = f_unlink(filename);

  printf("sdm_remove_payload_file: %s (%d)\n", FRESULT_str(temp_fresult), temp_fresult);
  return temp_fresult;
}

bool sdm_check_file_ext(TCHAR *fname, const TCHAR *ext)
{
  if (!(*fname) && strlen(fname) > 24 ) return 0;
  
  char *fn_cpy = strdup(fname);

  strtok(fn_cpy, ".");
  fn_cpy = strtok(NULL, "");
  
  return (strcmp(fn_cpy, ext) == 0);
}

/** TODO: BUNE reis*/
FRESULT sdm_write(char *str, UINT size)
{
  UINT written_byte_size;

  temp_fresult = f_write(&temp_fil, str, size, &written_byte_size);

  if (temp_fresult != FR_OK && written_byte_size < 0) {
      printf("ERROR: Could not write to file (%d)\r\n", written_byte_size);
      return temp_fresult;
  }

  printf("SDM written byte:%lu\n", written_byte_size);
  return FR_OK;
}