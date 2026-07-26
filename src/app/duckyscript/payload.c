#include "payload.h"

#include "ducky_config.h"  // DUCKY_PAYLOAD_DIR, DUCKY_FILE_EXT, DUCKY_MAX_PAYLOAD_FNAME_LEN
#include "middleware/fat_io.h"
#include "middleware/log.h"
#include "middleware/sys_fault.h"

static const char* TAG = "payload";

void payload_list_init(PayloadList* list) {
    ABORT_IF(list == NULL);
    ABORT_IF(!fat_io_is_mounted());

    list->total_count = 0;
    list->window_start = 0;
    list->cached_count = 0;

    list->total_count = fat_io_count_files(DUCKY_PAYLOAD_DIR, DUCKY_FILE_EXT, DUCKY_MAX_PAYLOAD_FNAME_LEN);

    payload_list_load_window(list, 0);

    LOG_INFO(TAG, "payload_list_init: %lu payloads, cached %lu", list->total_count, list->cached_count);
}

uint32_t payload_list_count(const PayloadList* list) {
    ABORT_IF(list == NULL);
    return list->total_count;
}

bool payload_list_load_window(PayloadList* list, uint32_t window_start) {
    ABORT_IF(list == NULL);

    if (list->total_count != 0 && window_start >= list->total_count) return false;

    DIR dir;
    if (fat_io_open_dir(&dir, DUCKY_PAYLOAD_DIR) != FR_OK) return false;

    // Skip window_start valid entries by reading them into a throwaway buffer.
    char skip[DUCKY_MAX_PAYLOAD_FNAME_LEN];
    for (uint32_t i = 0; i < window_start;) {
        FatIoResult r = fat_io_read_dir(&dir, DUCKY_FILE_EXT, skip, sizeof(skip));
        if (r == FAT_IO_OK) {
            i++;
        } else if (r != FAT_IO_BUFFER_SIZE_ERROR) {
            fat_io_close_dir(&dir);
            return false;
        }
    }

    list->window_start = window_start;
    list->cached_count = 0;
    while (list->cached_count < PAYLOAD_CACHE_SIZE) {
        FatIoResult r =
            fat_io_read_dir(&dir, DUCKY_FILE_EXT, list->names[list->cached_count], DUCKY_MAX_PAYLOAD_FNAME_LEN);
        if (r == FAT_IO_OK) {
            list->cached_count++;
        } else if (r != FAT_IO_BUFFER_SIZE_ERROR) {
            break;
        }
    }

    fat_io_close_dir(&dir);
    return true;
}

const char* payload_list_get(PayloadList* list, uint32_t index) {
    ABORT_IF(list == NULL);

    if (index >= list->total_count) return NULL;

    bool in_window = index >= list->window_start && index < list->window_start + list->cached_count;
    if (!in_window) {
        uint32_t page_start = (index / PAYLOAD_CACHE_SIZE) * PAYLOAD_CACHE_SIZE;
        if (!payload_list_load_window(list, page_start)) return NULL;
    }

    return list->names[index - list->window_start];
}
