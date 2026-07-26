#include "fat_io.h"

#include <stdlib.h>

#include "ff.h"
#include "hw_config.h"
#include "middleware/log.h"
#include "sleep_manager.h"

static const char* TAG = "fat_io";

// Runs last in the sleep sequence (after every task has finished with the SD
// card), so unmounting here can't race an in-flight file operation.
static void fat_io_sleep_finalizer(void* context) {
    (void)context;
    fat_io_unmount();
}

FatIoResult fat_io_init() {
    sd_card_t* sd_card = sd_get_by_num(0);

    if (!sd_card->spi_if_p->spi->initialized) {
        LOG_ERROR(TAG, "SD CARD not initiated!\n");
        return FAT_IO_NOT_READY;
    }

    FRESULT fr = f_mount(&sd_card->state.fatfs, "", 1);

    LOG_INFO(TAG, "fat_io_init: %s (%d) %d", FRESULT_str(fr), fr, fat_io_is_mounted());

    if (fr != FR_OK) return (FatIoResult)fr;

    sleep_manager_register_finalizer(fat_io_sleep_finalizer, NULL);

    return FAT_IO_OK;
}

FatIoResult fat_io_unmount() {
    FRESULT fr = f_unmount("");

    LOG_INFO(TAG, "fat_io_unmount: %s (%d) %d", FRESULT_str(fr), fr, fat_io_is_mounted());

    return (FatIoResult)fr;
}

bool fat_io_is_mounted() { return sd_get_by_num(0)->state.fatfs.fs_type != 0; }

FatIoResult fat_io_open_write(FIL* fil, const char* const filename) {
    FRESULT fr = f_open(fil, filename, FA_OPEN_APPEND | FA_WRITE | FA_CREATE_ALWAYS);

    LOG_INFO(TAG, "f_open(%s): %s (%d)", filename, FRESULT_str(fr), fr);

    return (FatIoResult)fr;
}

FatIoResult fat_io_open_read(FIL* fil, const char* const filename) {
    FRESULT fr = f_open(fil, filename, FA_READ);

    LOG_INFO(TAG, "fat_io_open_read: %s (%d)", FRESULT_str(fr), fr);

    return (FatIoResult)fr;
}

FatIoResult fat_io_close(FIL* fil) {
    FRESULT fr = f_close(fil);

    LOG_INFO(TAG, "f_close: %s (%d)", FRESULT_str(fr), fr);

    return (FatIoResult)fr;
}

FatIoResult fat_io_read_line(FIL* file, char* buf, size_t buf_size) {
    if (!f_gets(buf, buf_size, file)) {
        return f_eof(file) ? FAT_IO_EOF : FAT_IO_ERROR;
    }

    size_t len = strlen(buf);
    bool has_newline = (len > 0) && (buf[len - 1] == '\n' || buf[len - 1] == '\r');

    if (!has_newline && !f_eof(file)) {
        return FAT_IO_BUFFER_SIZE_ERROR;
    }

    return FAT_IO_OK;
}

FatIoResult fat_io_write(FIL* fil, char* str, uint32_t size) {
    UINT written_byte_size;

    FRESULT fr = f_write(fil, str, size, &written_byte_size);

    if (fr != FR_OK || written_byte_size != size) {
        LOG_ERROR(TAG, "ERROR: Could not write to file (%d)", written_byte_size);
    }

    return (FatIoResult)fr;
}

uint32_t fat_io_count_files(const char* path, const char* ext, uint32_t max_fname_len) {
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    uint32_t count = 0;

    fr = f_opendir(&dir, path);
    if (fr == FR_OK) {
        for (;;) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;
            if (fno.fattrib & AM_DIR) continue;
            if (ext && !fat_io_has_ext(fno.fname, ext)) continue;
            if (max_fname_len && strlen(fno.fname) >= max_fname_len) continue;
            count++;
        }
        f_closedir(&dir);
    }

    return count;
}

bool fat_io_has_ext(const char* name, const char* ext) {
    const char* dot = strrchr(name, '.');
    if (!dot) return false;
    return strcasecmp(dot, ext) == 0;
}

FatIoResult fat_io_open_dir(DIR* dir, const char* path) {
    FRESULT fr = f_opendir(dir, path);

    LOG_INFO(TAG, "fat_io_open_dir(%s): %s (%d)", path, FRESULT_str(fr), fr);

    return (FatIoResult)fr;
}

FatIoResult fat_io_close_dir(DIR* dir) {
    FRESULT fr = f_closedir(dir);

    LOG_INFO(TAG, "fat_io_close_dir: %s (%d)", FRESULT_str(fr), fr);

    return (FatIoResult)fr;
}

FatIoResult fat_io_read_dir(DIR* dir, const char* ext, char* buf, size_t buf_size) {
    FILINFO fno;

    while (f_readdir(dir, &fno) == FR_OK) {
        if (fno.fname[0] == '\0') {
            return FAT_IO_EOF;
        }

        if ((fno.fattrib & AM_DIR) || (ext && !fat_io_has_ext(fno.fname, ext))) {
            LOG_DEBUG(TAG, "fat_io_read_dir: skip %s (dir or wrong ext, want %s)", fno.fname, ext ? ext : "(none)");
            continue;
        }
        if (strlen(fno.fname) >= buf_size) return FAT_IO_BUFFER_SIZE_ERROR;

        strcpy(buf, fno.fname);
        return FAT_IO_OK;
    }

    return FAT_IO_ERROR;
}

FatIoResult fat_io_read_until(FIL* fil, char* buffer, char delim, int16_t len) {
    char s[2];
    UINT br;
    size_t i = 0;

    while (1) {
        f_read(fil, s, 1, &br);
        if (br != 1) return FAT_IO_INT_ERR;

        if (s[0] == delim) {
            buffer[i] = '\0';
            return FAT_IO_OK;
        }

        if (!(s[0] == '\n' || s[0] == '\r') && len-- > 0) buffer[i++] = s[0];
    }
}

FatIoResult fat_io_remove_file(const char* filename) {
    FRESULT fr = f_unlink(filename);

    LOG_INFO(TAG, "fat_io_remove_file: %s (%d)", FRESULT_str(fr), fr);
    return (FatIoResult)fr;
}

FatIoResult fat_io_rename(const char* old_path, const char* new_path) {
    FRESULT fr = f_rename(old_path, new_path);

    LOG_INFO(TAG, "fat_io_rename: %s (%d)", FRESULT_str(fr), fr);
    return (FatIoResult)fr;
}

FatIoResult fat_io_check_file_exist(const char* fname) {
    FRESULT fr;
    FILINFO fno;

    fr = f_stat(fname, &fno);

    return (FatIoResult)fr;
}

FatIoResult fat_io_skip_to_newline(FIL* file) {
    char c;
    UINT br; /* bytes actually read by f_read */

    for (;;) {
        FRESULT fr = f_read(file, &c, 1, &br);

        if (fr != FR_OK) return FAT_IO_ERROR;
        if (br == 0) return FAT_IO_EOF; /* file ended before a newline */

        if (c == '\n') return FAT_IO_OK;
        if (c == '\r') {
            // Could be CRLF: peek the next byte and consume it only if it's
            // the LF; otherwise rewind so the next read doesn't miss it.
            FRESULT pr = f_read(file, &c, 1, &br);
            if (pr != FR_OK) return FAT_IO_ERROR;
            if (br == 0) return FAT_IO_OK; /* file ended right after the CR */
            if (c != '\n') {
                f_lseek(file, f_tell(file) - 1);
            }
            return FAT_IO_OK;
        }
    }
}
