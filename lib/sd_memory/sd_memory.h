#ifndef _SD_MEMORY_H_
#define _SD_MEMORY_H_

// TODO: TATSIZ
#include "hw_config.h"

#define sdm_gets(buf) f_gets(buf, sizeof(buf), sdm_get_cur_fil())

FIL*    sdm_get_cur_fil();
FRESULT sdm_mount();
FRESULT sdm_unmount();
FRESULT sdm_open_read(char *filename);
FRESULT sdm_close_file();

#endif