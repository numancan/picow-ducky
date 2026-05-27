#pragma once

#include <stddef.h>

#include "f_util.h"
#include "stdbool.h"

#define fat_io_change_dir(dir) f_chdir(dir)

typedef enum {
    FAT_IO_READ_OK = 0,
    FAT_IO_READ_EOF,
    FAT_IO_READ_LINE_TOO_LONG,
    FAT_IO_READ_ERROR,
} FatIoReadResult;

FRESULT fat_io_mount();
bool fat_io_is_mounted();
FRESULT fat_io_unmount();
FRESULT fat_io_open_write(FIL* fil, const char* const filename);
FRESULT fat_io_open_read(FIL* fil, const char* const filename);
FRESULT fat_io_close(FIL* fil);
FatIoReadResult fat_io_read_line(FIL* file, char* buf, size_t buf_size);
FRESULT fat_io_write(FIL* fil, char* str, uint32_t size);
uint32_t fat_io_list_dir(const char* path, char* file_list, uint32_t n, const char* delim);
uint32_t fat_io_count_dir(const char* path);
FRESULT fat_io_read_until(FIL* fil, char* buffer, char delim, int16_t len);
FRESULT fat_io_remove_file(const char* filename);
FRESULT fat_io_check_file_exits(const char* fname);
