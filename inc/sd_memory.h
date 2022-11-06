#ifndef _SD_MEMORY_H_
#define _SD_MEMORY_H_

#include "hw_config.h"

FIL tempFil;

void sdm_init(void);
void sdm_gets(TCHAR*);
FRESULT sdm_mount();
FRESULT sdm_unmount();
FRESULT sdm_open_read(char *filename);
FRESULT sdm_close_file();
#endif