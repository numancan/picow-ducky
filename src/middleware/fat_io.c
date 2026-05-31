#include "fat_io.h"

#include <stdlib.h>

#include "hw_config.h"
#include "log.h"

static bool fat_io_is_mounted_flag = false;
static const char* TAG = "fat_io";

FRESULT fat_io_mount() {
    sd_card_t* sd_card = sd_get_by_num(0);

    if (!sd_card->spi_if_p->spi->initialized) {
        LOG_ERROR(TAG, "SD CARD not initiated!\n");
        return FR_NOT_READY;
    }

    FRESULT fr = f_mount(&sd_card->state.fatfs, "", 1);

    LOG_INFO(TAG, "fat_io_mount: %s (%d)", FRESULT_str(fr), fr);

    if (fr == FR_OK) fat_io_is_mounted_flag = true;
    return fr;
}

FRESULT fat_io_unmount() {
    FRESULT fr = f_unmount("");

    LOG_INFO(TAG, "fat_io_unmount: %s (%d)", FRESULT_str(fr), fr);

    if (fr == FR_OK) fat_io_is_mounted_flag = false;
    return fr;
}

bool fat_io_is_mounted() { return fat_io_is_mounted_flag; }

FRESULT fat_io_open_write(FIL* fil, const char* const filename) {
    FRESULT fr = f_open(fil, filename, FA_OPEN_APPEND | FA_WRITE | FA_CREATE_ALWAYS);

    LOG_INFO(TAG, "f_open(%s): %s (%d)", filename, FRESULT_str(fr), fr);

    return fr;
}

FRESULT fat_io_open_read(FIL* fil, const char* const filename) {
    FRESULT fr = f_open(fil, filename, FA_READ);

    // TCHAR str[126];
    // fr = f_getcwd(str, 126); /* Get current directory path */
    // printf("%s\n", fr);

    LOG_INFO(TAG, "fat_io_open_read: %s (%d)", FRESULT_str(fr), fr);

    return fr;
}

FRESULT fat_io_close(FIL* fil) {
    FRESULT fr = f_close(fil);

    LOG_INFO(TAG, "f_close: %s (%d)", FRESULT_str(fr), fr);

    return fr;
}

FatIoReadResult fat_io_read_line(FIL* file, char* buf, size_t buf_size) {
    if (!f_gets(buf, buf_size, file)) {
        return f_eof(file) ? FAT_IO_READ_EOF : FAT_IO_READ_ERROR;
    }

    size_t len = strlen(buf);
    bool has_newline = (len > 0) && (buf[len - 1] == '\n' || buf[len - 1] == '\r');

    if (!has_newline && !f_eof(file)) {
        return FAT_IO_READ_LINE_TOO_LONG;
    }

    return FAT_IO_READ_OK;
}

FRESULT fat_io_write(FIL* fil, char* str, uint32_t size) {
    UINT written_byte_size;

    FRESULT fr = f_write(fil, str, size, &written_byte_size);

    // TODO: check size == writen
    if (fr != FR_OK && written_byte_size < 0) {
        LOG_ERROR(TAG, "ERROR: Could not write to file (%d)", written_byte_size);
        return fr;
    }

    LOG_DEBUG(TAG, "fat_io written byte:%lu == %lu", written_byte_size, size);

    return fr;
}

uint32_t fat_io_count_files(const char* path) {
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    uint32_t count = 0;

    fr = f_opendir(&dir, path);
    if (fr == FR_OK) {
        for (;;) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;
            if (!(fno.fattrib & AM_DIR)) count++;
        }
        f_closedir(&dir);
    } else {
    }

    return count;
}

// FIXME: Deprecated
uint32_t fat_io_list_dir(const char* path, char* file_list, uint32_t n, const char* delim) {
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    uint32_t nFile = 0;

    memset(file_list, '\0', n);

    fr = f_opendir(&dir, path);

    if (fr == FR_OK) {
        for (;;) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;

            if (fno.fattrib & AM_DIR) { /* Directory */
                LOG_DEBUG(TAG, "   <DIR>   %s\n", fno.fname);
            } else { /* File */
                size_t fname_size = strlen(fno.fname);
                if (!(n > fname_size)) {
                    LOG_ERROR(TAG, "fat_io: There is no space to write file list");
                    fr = FR_INVALID_PARAMETER;
                    break;
                }
                strncat(file_list, fno.fname, fname_size);
                strncat(file_list, delim, 1);

                nFile++;
                n -= fname_size + 1;  // +1 for delim
            }
        }
        LOG_INFO(TAG, "fat_io_list_dir: %d file found!", nFile);
    } else {
        LOG_ERROR(TAG, "Failed to open \"%s\". (%u)", path, fr);
    }

    f_closedir(&dir);

    return nFile;
}

uint32_t fat_io_alloc_file_names(const char* path, char** file_list, uint32_t max_count) {
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    uint32_t count = 0;

    fr = f_opendir(&dir, path);
    if (fr == FR_OK) {
        for (;;) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;

            if (!(fno.fattrib & AM_DIR)) {
                size_t name_len = strlen(fno.fname);
                file_list[count] = malloc(name_len + 1);

                if (file_list[count] == NULL) {
                    LOG_ERROR(TAG, "Failed to allocate memory for file name");
                    return 0;
                }

                strcpy(file_list[count], fno.fname);

                count++;

                if (count >= max_count) break;
            }
        }
        f_closedir(&dir);
    } else {
        LOG_ERROR(TAG, "Failed to open directory: %d", fr);
    }

    return count;
}

FRESULT fat_io_read_until(FIL* fil, char* buffer, char delim, int16_t len) {
    char s[2];
    UINT br;
    size_t i = 0;

    while (1) {
        f_read(fil, s, 1, &br);
        if (br != 1) return FR_INT_ERR;

        if (s[0] == delim) {
            buffer[i] = '\0';
            return FR_OK;
        }

        if (!(s[0] == '\n' || s[0] == '\r') && len-- > 0) buffer[i++] = s[0];
    }

    return FR_OK;
}

FRESULT fat_io_remove_file(const char* filename) {
    FRESULT fr = f_unlink(filename);

    LOG_INFO(TAG, "fat_io_remove_file: %s (%d)", FRESULT_str(fr), fr);
    return fr;
}

FRESULT fat_io_rename(const char* old_path, const char* new_path) {
    FRESULT fr = f_rename(old_path, new_path);

    LOG_INFO(TAG, "fat_io_rename: %s (%d)", FRESULT_str(fr), fr);
    return fr;
}

FRESULT fat_io_check_file_exits(const char* fname) {
    FRESULT fr;
    FILINFO fno;
    LOG_INFO(TAG, "Test for \"%s\"...\n", fname);

    fr = f_stat(fname, &fno);

    LOG_INFO(TAG, "fat_io_check_file_exits: %s (%d)", FRESULT_str(fr), fr);
    return fr;
}
