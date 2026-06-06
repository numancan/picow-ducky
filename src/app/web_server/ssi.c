#include <lwip/arch.h>
#include <string.h>

// #include "pico.h"
#include "app/duckyscript/ducky_config.h"
#include "app/settings/settings.h"
#include "lwip/apps/fs.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwipopts.h"
#include "middleware/fat_io.h"
#include "middleware/log.h"
#include "ssi.h"

static const char* TAG = "ssi";

// Be careful tag name length, it's limited by LWIP_HTTPD_MAX_TAG_NAME_LEN = 8
static const char* ssi_tags[] = {"payload", "wssid", "wpass", "pname", "pti", "weben"};

typedef struct {
    DIR dir;
    bool is_dir_open;
    bool is_done;
    char pending_fname[DUCKY_MAX_PAYLOAD_FNAME_LEN + 1];
} PayloadSsiState;

static bool has_suffix(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return false;
    return strcmp(str + (str_len - suffix_len), suffix) == 0;
}

void* fs_state_init(struct fs_file* file, const char* name) {
    (void)file;

    if (name == NULL || !has_suffix(name, ".shtml")) return NULL;

    PayloadSsiState* st = (PayloadSsiState*)mem_malloc(sizeof(PayloadSsiState));
    if (st == NULL) {
        LOG_ERROR(TAG, "fs_state_init: out of memory for %s", name);
        return NULL;
    }

    st->pending_fname[0] = '\0';

    if (fat_io_open_dir(&st->dir, DUCKY_PAYLOAD_DIR) == FR_OK) {
        st->is_dir_open = true;
        st->is_done = false;
    } else {
        LOG_ERROR(TAG, "opendir %s failed", DUCKY_PAYLOAD_DIR);
        st->is_dir_open = false;
        st->is_done = true;
    }

    return st;
}

void fs_state_free(struct fs_file* file, void* state) {
    (void)file;

    if (state == NULL) return;

    PayloadSsiState* st = (PayloadSsiState*)state;
    if (st->is_dir_open) {
        fat_io_close_dir(&st->dir);
    }
    mem_free(st);
}

static u16_t payload_handler(PayloadSsiState* st, char* pcInsert, int iInsertLen, u16_t current_tag_part,
                             u16_t* next_tag_part) {
    if (st == NULL) {
        LOG_WARN(TAG, "payload handler called with null state");
        return 0;
    }

    if (st->is_done) return 0;

    int used = 0;

    while (used < iInsertLen) {
        if (st->pending_fname[0] == '\0') {
            FatIoResult res = fat_io_read_dir(&st->dir, DUCKY_FILE_EXT, st->pending_fname, sizeof(st->pending_fname));

            if (res != FAT_IO_OK) {
                if (res == FAT_IO_BUFFER_SIZE_ERROR) {
                    LOG_WARN(TAG, "payload name too long, skipped");
                    st->pending_fname[0] = '\0';
                    continue;
                }
                if (res != FAT_IO_EOF) {
                    LOG_ERROR(TAG, "fat_io_read_dir failed (%d), SD/FS error?", res);
                }
                st->is_done = true;
                break;
            }
        }

        int len = (int)strlen(st->pending_fname);
        int need = len;

        /* Can only happen if sizeof(pending_fname) > LWIP_HTTPD_MAX_TAG_INSERT_LEN:
         * the name fits no part, so drop it to avoid an infinite multipart loop. */
        if (used == 0 && need > iInsertLen) {
            LOG_WARN(TAG, "payload name does not fit insert buffer, skipped");
            st->pending_fname[0] = '\0';
            continue;
        }

        if (used + need > iInsertLen) break;

        memcpy(pcInsert + used, st->pending_fname, len);
        used += len;

        st->pending_fname[0] = '\0';
    }

    if (st->pending_fname[0] != '\0' || !st->is_done) {
        *next_tag_part = current_tag_part + 1;
    }

    return (u16_t)used;
}

u16_t ssi_handler(int iIndex, char* pcInsert, int iInsertLen, u16_t current_tag_part, u16_t* next_tag_part,
                  void* connection_state) {
    u16_t printed;

    LOG_DEBUG(TAG, "ssi_handler: %s %d", ssi_tags[iIndex], iInsertLen);

    switch (iIndex) {
        case 0: /*  "payload" */
            printed = payload_handler((PayloadSsiState*)connection_state, pcInsert, iInsertLen, current_tag_part,
                                      next_tag_part);
            break;

        case 1: /* "wssid" */
            printed = snprintf(pcInsert, iInsertLen, "<input type='text' id='wssid' name='WSSID' value='%s'>",
                               settings_get_string(SETTINGS_ID_WIFI_SSID));
            break;

        case 2: /* "wpass" */
            printed = snprintf(pcInsert, iInsertLen, "<input type='text' id='wpass' name='WPASS' value='%s'>",
                               settings_get_string(SETTINGS_ID_WIFI_PASS));
            break;

        case 3: /* "pname" */
            printed = snprintf(pcInsert, iInsertLen, "<input type='text' id='pname' name='PNAME' value='%s'>",
                               settings_get_string(SETTINGS_ID_PAYLOAD_NAME));
            break;

        case 4: /* "pti" */
        {
            bool pti_enabled = settings_get_bool(SETTINGS_ID_PAYLOAD_TO_INJECT);
            printed = snprintf(pcInsert, iInsertLen, "<input type='hidden' name='PTI' id='ptiH' value='%s'>",
                               pti_enabled ? "1" : "0");
        } break;

        case 5: /* "weben" */
        {
            bool weben_enabled = settings_get_bool(SETTINGS_ID_WEB_SERVER_ENABLED);
            printed = snprintf(pcInsert, iInsertLen, "<input type='hidden' name='WEBEN' id='webenH' value='%s'>",
                               weben_enabled ? "1" : "0");
        } break;

        default: /* unknown tag */ printed = 0; break;
    }

    LWIP_ASSERT("sane length", printed <= 0xFFFF);
    return (u16_t)printed;
}

void ssi_init() {
    for (size_t i = 0; i < LWIP_ARRAYSIZE(ssi_tags); i++) {
        LWIP_ASSERT("tag too long for LWIP_HTTPD_MAX_TAG_NAME_LEN", strlen(ssi_tags[i]) <= LWIP_HTTPD_MAX_TAG_NAME_LEN);
    }

    http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));
}
