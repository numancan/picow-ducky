#include "sd_memory.h"
#include "f_util.h"

static sd_card_t *pSD;
FRESULT tempFR;


void sdm_init()
{
  // time_init(); ??
  pSD = sd_get_by_num(0);
}

void sdm_gets(TCHAR *buf)
{
    f_gets(buf, sizeof(buf), &tempFil);
}

FRESULT sdm_mount()
{
  tempFR = f_mount(&pSD->fatfs, pSD->pcName, 1);
  if (tempFR != FR_OK) panic("f_mount error: %s (%d)\n", FRESULT_str(tempFR), tempFR);

  return tempFR;
}

FRESULT sdm_open_read(char* filename)
{
  
  tempFR = f_open(&tempFil, filename, FA_READ);

  if (tempFR != FR_OK) {
      panic("ERROR: Could not open file (%d)\r\n", tempFR);
      while (true);
  }

  return tempFR;
}

FRESULT sdm_close_file()
{
  tempFR = f_close(&tempFil);

  if (tempFR != FR_OK) {
    panic("f_close error: %s (%d)\n", FRESULT_str(tempFR), tempFR);  
  }

  return tempFR;
}

FRESULT sdm_unmount()
{
  tempFR = f_mount(0, pSD->pcName, 0);
  if (tempFR != FR_OK) panic("f_umount error: %s (%d)\n", FRESULT_str(tempFR), tempFR);

  return tempFR;
}
