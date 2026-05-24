#pragma once

#include <stddef.h>

#include "f_util.h"
#include "stdbool.h"

#define sd_card_change_dir(dir) f_chdir(dir)

typedef enum {
    SD_READ_OK = 0,
    SD_READ_EOF,
    SD_READ_LINE_TOO_LONG,
    SD_READ_ERROR,
} sd_read_result_t;

FRESULT sd_card_mount();
bool sd_card_is_mounted();
FRESULT sd_card_unmount();
FRESULT sd_card_open_write(FIL* fil, const char* const filename);
FRESULT sd_card_open_read(FIL* fil, const char* const filename);
FRESULT sd_card_close(FIL* fil);
sd_read_result_t sd_card_read_line(FIL* file, char* buf, size_t buf_size);
FRESULT sd_card_write(FIL* fil, char* str, uint32_t size);
uint32_t sd_card_list_dir(const char* path, char* file_list, uint32_t n, const char* delim);
uint32_t sd_card_count_dir(const char* path);
FRESULT sd_card_read_until(FIL* fil, char* buffer, char delim, int16_t len);
FRESULT sd_card_remove_file(const char* filename);
FRESULT sd_card_check_file_exits(const char* fname);