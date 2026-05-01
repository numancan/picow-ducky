#pragma once

#include "f_util.h"
#include "stdbool.h"

#define sd_card_change_dir(dir) f_chdir(dir)
#define sd_card_read_line(file, buff, len) f_gets(buff, len, file)

FRESULT sd_card_mount();
bool sd_card_is_mounted();
FRESULT sd_card_unmount();
FRESULT sd_card_open_write(FIL *fil, const char *const filename);
FRESULT sd_card_open_read(FIL *fil, const char *const filename);
FRESULT sd_card_close(FIL *fil);
FRESULT sd_card_write(FIL *fil, char *str, uint32_t size);
uint32_t sd_card_list_dir(const char *path, char *file_list, uint32_t n, const char* delim);
uint32_t sd_card_count_dir(const char *path);
FRESULT sd_card_read_until(FIL *fil, char *buffer, char delim, int16_t len);
FRESULT sd_card_remove_file(const char *filename);
FRESULT sd_card_check_file_exits(const char *fname);