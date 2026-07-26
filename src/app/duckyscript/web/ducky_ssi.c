#include <lwip/arch.h>
#include <stdio.h>
#include <string.h>

#include "ducky_web.h"
#include "duckyscript/ducky.h"
#include "duckyscript/ducky_config.h"
#include "hid/hid_transport.h"
#include "lwip/mem.h"
#include "middleware/fat_io.h"
#include "middleware/log.h"

static const char* TAG = "ssi";

// The only SSI file that needs per-connection streaming state (see ducky_ssi_state_init).
#define SSI_PAYLOADS_FILE "payloads.shtml"

typedef struct {
    DIR dir;
    bool is_dir_open;
    bool is_done;
    bool first;  // JSON array: emit a leading comma before every element except the first
    // Holds the current JSON token (",\"name\"") while it is streamed across insert parts.
    char pending[DUCKY_MAX_PAYLOAD_FNAME_LEN + 4];
} PayloadSsiState;

static bool has_suffix(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return false;
    return strcmp(str + (str_len - suffix_len), suffix) == 0;
}

void* ducky_ssi_state_init(const char* name) {
    // Only the payload-list endpoint streams a directory; other SSI files are stateless.
    if (name == NULL || !has_suffix(name, SSI_PAYLOADS_FILE)) return NULL;

    PayloadSsiState* st = (PayloadSsiState*)mem_malloc(sizeof(PayloadSsiState));
    if (st == NULL) {
        LOG_ERROR(TAG, "state_init: out of memory for %s", name);
        return NULL;
    }

    st->pending[0] = '\0';
    st->first = true;

    if (fat_io_open_dir(&st->dir, DUCKY_PAYLOAD_DIR) == FAT_IO_OK) {
        st->is_dir_open = true;
        st->is_done = false;
    } else {
        LOG_ERROR(TAG, "opendir %s failed", DUCKY_PAYLOAD_DIR);
        st->is_dir_open = false;
        st->is_done = true;
    }

    return st;
}

void ducky_ssi_state_free(void* state) {
    if (state == NULL) return;

    PayloadSsiState* st = (PayloadSsiState*)state;
    if (st->is_dir_open) {
        fat_io_close_dir(&st->dir);
    }
    mem_free(st);
}

// Streams the payload directory as the body of a JSON array. The enclosing brackets live in
// the .shtml file (`[<!--#payload-->]`); this emits the comma-separated quoted names in between.
u16_t ducky_ssi_payload(void* state, char* pcInsert, int iInsertLen, u16_t current_tag_part, u16_t* next_tag_part) {
    PayloadSsiState* st = (PayloadSsiState*)state;
    if (st == NULL) {
        LOG_WARN(TAG, "payload handler called with null state");
        return 0;
    }

    int used = 0;

    while (used < iInsertLen && !st->is_done) {
        if (st->pending[0] == '\0') {
            char name[DUCKY_MAX_PAYLOAD_FNAME_LEN + 1];
            FatIoResult res = fat_io_read_dir(&st->dir, DUCKY_FILE_EXT, name, sizeof(name));

            if (res != FAT_IO_OK) {
                if (res == FAT_IO_BUFFER_SIZE_ERROR) {
                    LOG_WARN(TAG, "payload name too long, skipped");
                    continue;
                }
                if (res != FAT_IO_EOF) {
                    LOG_ERROR(TAG, "fat_io_read_dir failed (%d), SD/FS error?", res);
                }
                st->is_done = true;
                break;
            }

            // Bake the leading comma in now using the current `first` flag; `first` is only
            // cleared once the token is fully copied, so a carried-over token keeps its prefix.
            snprintf(st->pending, sizeof(st->pending), "%s\"%s\"", st->first ? "" : ",", name);
        }

        int len = (int)strlen(st->pending);

        /* Token longer than a whole insert buffer: drop it to avoid an infinite multipart loop.
         * Leave `first` untouched so the array stays valid JSON. */
        if (used == 0 && len > iInsertLen) {
            LOG_WARN(TAG, "payload token does not fit insert buffer, skipped");
            st->pending[0] = '\0';
            continue;
        }

        if (used + len > iInsertLen) break;

        memcpy(pcInsert + used, st->pending, len);
        used += len;
        st->pending[0] = '\0';
        st->first = false;
    }

    if (st->pending[0] != '\0' || !st->is_done) {
        *next_tag_part = current_tag_part + 1;
    }

    return (u16_t)used;
}

// Single-shot JSON object of the live ducky settings (mirrors the cfg keys parsed by ducky_load_cb).
u16_t ducky_ssi_settings(char* pcInsert, int iInsertLen) {
    const DuckySettings* s = ducky_get_settings();
    int n = snprintf(pcInsert, iInsertLen,
                     "{\"char_delay\":%lu,\"char_fuzz_delay\":%lu,\"delay\":%lu,\"fuzz_delay\":%lu,"
                     "\"kb_layout\":%u,\"transporter\":%u,\"transporter_always_on\":%s}",
                     (unsigned long)s->char_delay, (unsigned long)s->char_fuzz_delay, (unsigned long)s->delay,
                     (unsigned long)s->fuzz_delay, (unsigned)s->kb_layout, (unsigned)s->transporter,
                     s->transporter_always_on ? "true" : "false");

    if (n < 0 || n >= iInsertLen) {
        LOG_WARN(TAG, "settings json does not fit insert buffer (need %d, have %d)", n, iInsertLen);
        return 0;
    }
    return (u16_t)n;
}

// Single-shot JSON object of the current playback + transport status. `status`/`transport` are
// numeric enum ids the web UI maps via EXPORTS.DUCKY_STATUS_LIST / EXPORTS.HID_STATUS_LIST;
// `transport_name` is the configured transport's display name (shown while arming). The browser
// polls `transport` to gate a trigger on readiness before playback starts.
u16_t ducky_ssi_status(char* pcInsert, int iInsertLen) {
    const DuckySettings* s = ducky_get_settings();
    const char* transport_name = hid_transport_name(s->transporter);
    int n = snprintf(pcInsert, iInsertLen, "{\"status\":%d,\"transport\":%d,\"transport_name\":\"%s\"}",
                     (int)ducky_get_status(), (int)hid_transport_status(), transport_name ? transport_name : "?");

    if (n < 0 || n >= iInsertLen) {
        LOG_WARN(TAG, "status json does not fit insert buffer");
        return 0;
    }
    return (u16_t)n;
}
