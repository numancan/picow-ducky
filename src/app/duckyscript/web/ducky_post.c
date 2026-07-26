#include <stdio.h>
#include <string.h>

#include "ducky_web.h"
#include "duckyscript/ducky_config.h"
#include "lwip/pbuf.h"
#include "middleware/fat_io.h"
#include "middleware/log.h"

static const char* TAG = "POST";

// Both routes stream their body straight to this handle, opened in begin.
static FIL current_file;

// Example uri: upload?p=payload.txt
static const char* extract_uri_param(const char* uri) {
    const char* eq = strchr(uri, '=');
    return eq ? eq + 1 : NULL;
}

// Shared by both routes: stream one pbuf chain to the open file and free the chain.
static err_t post_write_body(struct pbuf* p) {
    err_t err = ERR_OK;

    for (struct pbuf* q = p; q != NULL; q = q->next) {
        FRESULT fr = fat_io_write(&current_file, (char*)q->payload, q->len);
        if (fr != FR_OK) {
            LOG_ERROR(TAG, "fat_io_write: %s (%d)", FRESULT_str(fr), fr);
            err = ERR_VAL;
            break;
        }
    }

    pbuf_free(p);
    return err;
}

static err_t upload_begin(const char* uri, u8_t* post_auto_wnd) {
    err_t err = ERR_VAL;

    LOG_INFO(TAG, "URI: %s", uri);

    const char* param = extract_uri_param(uri);
    if (param != NULL && strlen(param) <= DUCKY_MAX_PAYLOAD_FNAME_LEN && fat_io_has_ext(param, DUCKY_FILE_EXT)) {
        // TODO: use full path
        fat_io_change_dir(DUCKY_PAYLOAD_DIR);
        err = fat_io_open_write(&current_file, param);
        fat_io_change_dir("..");
    }

    *post_auto_wnd = 0;
    return err;
}

static void upload_finished(char* response_uri, u16_t response_uri_len) {
    (void)response_uri;
    (void)response_uri_len;
    fat_io_close(&current_file);
}

const PostHandler ducky_upload_post_handler = {
    .begin = upload_begin,
    .receive = post_write_body,
    .finished = upload_finished,
};

static err_t settings_begin(const char* uri, u8_t* post_auto_wnd) {
    (void)uri;
    (void)post_auto_wnd;

    LOG_INFO(TAG, "URI: %s", uri);

    // Stream the raw config body to a staging file; parsing/applying happens later in the CGI
    // handler fired from the finished redirect. Keeps this module schema-agnostic.
    return fat_io_open_write(&current_file, DUCKY_CONFIG_RX_PATH);
}

static void settings_finished(char* response_uri, u16_t response_uri_len) {
    fat_io_close(&current_file);
    // Hand off to the CGI event bus: the handler parses the staged file and applies it.
    snprintf(response_uri, response_uri_len, "/api/ducky/cmd.cgi?settings=1");
}

const PostHandler ducky_settings_post_handler = {
    .begin = settings_begin,
    .receive = post_write_body,
    .finished = settings_finished,
};

static err_t live_begin(const char* uri, u8_t* post_auto_wnd) {
    (void)uri;
    (void)post_auto_wnd;

    LOG_INFO(TAG, "URI: %s", uri);

    // Stream the live editor's script body to a scratch payload file; the actual playback is
    // fired from the finished redirect's CGI handler, keeping this module playback-agnostic.
    return fat_io_open_write(&current_file, DUCKY_LIVE_PATH);
}

static void live_finished(char* response_uri, u16_t response_uri_len) {
    fat_io_close(&current_file);
    // Hand off to the CGI event bus: the handler queues the scratch file for playback.
    snprintf(response_uri, response_uri_len, "/api/ducky/cmd.cgi?live=1");
}

const PostHandler ducky_live_post_handler = {
    .begin = live_begin,
    .receive = post_write_body,
    .finished = live_finished,
};
