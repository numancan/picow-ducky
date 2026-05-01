#include "sd_card.h"

#include "hw_config.h"
#include "middleware/debug.h"

static bool sd_is_mounted = false;

FRESULT sd_card_mount() {
    sd_card_t* sd_card = sd_get_by_num(0);

    PANIC_IF("SD CARD not initiated!", sd_card->state.m_Status == STA_NOINIT);

    FRESULT fr = f_mount(&sd_card->state.fatfs, "", 1);

    DEBUG_PRINTF("sdm_mount: %s (%d)\n", FRESULT_str(fr), fr);

    if (fr == FR_OK) sd_is_mounted = true;
    return fr;
}

FRESULT sd_card_unmount() {
    FRESULT fr = f_unmount("");

    DEBUG_PRINTF("sdm_unmount: %s (%d)\n", FRESULT_str(fr), fr);

    if (fr == FR_OK) sd_is_mounted = false;
    return fr;
}

bool sd_card_is_mounted() { return sd_is_mounted; }

FRESULT sd_card_open_write(FIL* fil, const char* const filename) {
    FRESULT fr = f_open(fil, filename, FA_OPEN_APPEND | FA_WRITE | FA_CREATE_ALWAYS);

    DEBUG_PRINTF("f_open(%s): %s (%d)\n", filename, FRESULT_str(fr), fr);

    return fr;
}

FRESULT sd_card_open_read(FIL* fil, const char* const filename) {
    FRESULT fr = f_open(fil, filename, FA_READ);

    // TCHAR str[126];
    // fr = f_getcwd(str, 126); /* Get current directory path */
    // printf("%s\n", fr);

    DEBUG_PRINTF("sd_card_open: %s (%d)\n", FRESULT_str(fr), fr);

    return fr;
}

FRESULT sd_card_close(FIL* fil) {
    FRESULT fr = f_close(fil);

    DEBUG_PRINTF("f_close: %s (%d)\n", FRESULT_str(fr), fr);

    return fr;
}

FRESULT sd_card_write(FIL* fil, char* str, uint32_t size) {
    UINT written_byte_size;

    FRESULT fr = f_write(fil, str, size, &written_byte_size);

    // TODO: check size == writen
    if (fr != FR_OK && written_byte_size < 0) {
        DEBUG_PRINTF("ERROR: Could not write to file (%d)\r\n", written_byte_size);
        return fr;
    }

    DEBUG_PRINTF("SC written byte:%lu == %lu\n", written_byte_size, size);

    return fr;
}

uint32_t sd_card_list_dir(const char* path, char* file_list, uint32_t n, const char* delim) {
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
                DEBUG_PRINTF("   <DIR>   %s\n", fno.fname);
            } else { /* File */
                // DEBUG_PRINTF("%s\n", fno.fname);

                size_t fname_size = strlen(fno.fname);
                if (!(n > fname_size)) {
                    printf("SC: There is no space to write file list");
                    fr = FR_INVALID_PARAMETER;
                    break;
                }
                strncat(file_list, fno.fname, fname_size);
                strncat(file_list, delim, 1);

                nFile++;
                n -= fname_size + 1;  // +1 for delim
            }
        }
        DEBUG_PRINTF("sd_card_list_dir: %d file found!\n", nFile);
    } else {
        printf("Failed to open \"%s\". (%u)\n", path, fr);
    }

    f_closedir(&dir);

    return nFile;
}

uint32_t sd_card_count_dir(const char* path) {
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    uint32_t nFile = 0;

    fr = f_opendir(&dir, path);

    if (fr == FR_OK) {
        for (;;) {
            fr = f_readdir(&dir, &fno);
            if (fr != FR_OK || fno.fname[0] == 0) break;
            nFile++;
        }
        f_closedir(&dir);
    }

    return nFile;
}

FRESULT sd_card_read_until(FIL* fil, char* buffer, char delim, int16_t len) {
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

FRESULT sd_card_remove_file(const char* filename) {
    FRESULT fr = f_unlink(filename);

    DEBUG_PRINTF("sd_card_remove_file: %s (%d)\n", FRESULT_str(fr), fr);
    return fr;
}

FRESULT sd_card_check_file_exits(const char* fname) {
    FRESULT fr;
    FILINFO fno;
    DEBUG_PRINTF("Test for \"%s\"...\n", fname);

    fr = f_stat(fname, &fno);

    DEBUG_PRINTF("sd_card_check_file_exits: %s (%d)\n", FRESULT_str(fr), fr);
    return fr;
}

// FRESULT sd_card_list_dir(char *path) {
//     DIR temp_dir;
//     FILINFO fno;

//     FRESULT fr = f_opendir(&temp_dir, path);

//     // snprintf(fnames, n, "");

//     if (fr == FR_OK) {
//         while (f_readdir(&temp_dir, &fno) == FR_OK && fno.fname[0]) {  // TODO: ya bu readdir resulttu
//         kaydetmiyoruz?
//             // strncat(fnames, fno.fname, n);
//             printf(fno.fname);
//             size_t fname_size = strlen(fno.fname);
//             // n -= fname_size;
//             // memset(fno.fname, 0, fname_size);
//         }

//         fr = f_closedir(&temp_dir);
//     }

//     printf("sdm_read_dir: %s (%d)\n", FRESULT_str(fr), fr);

//     return fr;
// }