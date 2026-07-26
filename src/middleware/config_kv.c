#include "config_kv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "middleware/log.h"
#include "middleware/sys_fault.h"

static const char* TAG = "CONFIG_IO";

static char* config_kv_trim_left(char* s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

static void config_kv_trim_right(char* s) {
    size_t n = strlen(s);
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' || s[n - 1] == '\n')) {
        s[--n] = '\0';
    }
}

// All write helpers funnel through here. fat_io_write wants a non-const
// pointer; the cast is safe since the buffer is never modified.
static bool config_kv_emit(FIL* f, const char* str) {
    return fat_io_write(f, (char*)str, (uint32_t)strlen(str)) == FAT_IO_OK;
}

bool config_kv_parse(const char* path, ConfigKvLineFn fn, void* ctx) {
    ABORT_IF(fn == NULL);

    FIL fil;
    if (fat_io_open_read(&fil, path) != FAT_IO_OK) {
        LOG_WARN(TAG, "settings file not found");
        return false;
    }

    char line[CONFIG_KV_LINE_MAX];
    bool ok = true;

    for (;;) {
        FatIoResult r = fat_io_read_line(&fil, line, sizeof(line));

        if (r == FAT_IO_EOF) {
            break;
        }
        if (r == FAT_IO_ERROR) {
            ok = false;
            break;
        }
        if (r == FAT_IO_BUFFER_SIZE_ERROR) {
            // Line didn't fit: discard up to the next newline, otherwise the
            // leftover tail gets misparsed as a new line on the next round.
            FatIoResult d = fat_io_skip_to_newline(&fil);
            if (d == FAT_IO_ERROR) {
                ok = false;
                break;
            }
            if (d == FAT_IO_EOF) break;
            continue;
        }

        char* sep = strchr(line, ':');
        if (!sep) continue; /* no ':' -> blank/comment line, skip */
        *sep = '\0';

        char* key = config_kv_trim_left(line);
        config_kv_trim_right(key);
        char* val = config_kv_trim_left(sep + 1);
        config_kv_trim_right(val);

        if (*key == '\0') continue; /* no key -> skip */

        fn(key, val, ctx);
    }

    fat_io_close(&fil);
    return ok;
}

bool config_kv_write_header(FIL* f, const char* filetype, uint32_t version) {
    char b[CONFIG_KV_LINE_MAX];
    int n = snprintf(b, sizeof(b), "Filetype: %s\n", filetype);
    if (n < 0 || (size_t)n >= sizeof(b)) return false;
    if (!config_kv_emit(f, b)) return false;
    return config_kv_write_u32(f, "Version", version);
}

bool config_kv_write_u32(FIL* f, const char* key, uint32_t value) {
    char b[CONFIG_KV_LINE_MAX];
    int n = snprintf(b, sizeof(b), "%s: %lu\n", key, (unsigned long)value);
    if (n < 0 || (size_t)n >= sizeof(b)) return false; /* truncation guard */
    return config_kv_emit(f, b);
}

bool config_kv_write_i32(FIL* f, const char* key, int32_t value) {
    char b[CONFIG_KV_LINE_MAX];
    int n = snprintf(b, sizeof(b), "%s: %ld\n", key, (long)value);
    if (n < 0 || (size_t)n >= sizeof(b)) return false;
    return config_kv_emit(f, b);
}

bool config_kv_write_bool(FIL* f, const char* key, bool value) { return config_kv_write_u32(f, key, value ? 1u : 0u); }

bool config_kv_write_str(FIL* f, const char* key, const char* value) {
    char b[CONFIG_KV_LINE_MAX];
    int n = snprintf(b, sizeof(b), "%s: %s\n", key, value);
    if (n < 0 || (size_t)n >= sizeof(b)) return false;
    return config_kv_emit(f, b);
}

bool config_kv_save_atomic(const char* path, ConfigKvWriteFn writer, void* ctx) {
    ABORT_IF(writer == NULL);

    char tmp[CONFIG_KV_LINE_MAX];
    int n = snprintf(tmp, sizeof(tmp), "%s.tmp", path);
    if (n < 0 || (size_t)n >= sizeof(tmp)) return false;

    FIL fil;
    if (fat_io_open_write(&fil, tmp) != FAT_IO_OK) return false;

    bool ok = writer(&fil, ctx);

    // Close even on a failed write, otherwise the descriptor leaks.
    if (fat_io_close(&fil) != FAT_IO_OK) ok = false;

    if (!ok) {
        fat_io_remove_file(tmp);
        return false;
    }

    // FAT rename can fail if the destination exists, so remove it first.
    // A power loss here leaves path briefly missing; the window is tiny and
    // acceptable for config data (a fully crash-safe scheme would need a
    // two-file/journal approach).
    if (fat_io_check_file_exist(path) == FAT_IO_OK) {
        fat_io_remove_file(path);
    }

    if (fat_io_rename(tmp, path) != FAT_IO_OK) {
        fat_io_remove_file(tmp);
        return false;
    }
    return true;
}
