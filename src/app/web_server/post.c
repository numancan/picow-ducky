#include "post.h"

#include <stdio.h>
#include <string.h>

#include "app/settings/settings.h"
#include "lwip/apps/httpd.h"
#include "middleware/fat_io.h"
#include "middleware/log.h"


static const char* TAG = "POST";

enum { INVALID_POST = -1, POST_UPLOAD = 0, POST_SETTINGS };

typedef struct {
    int8_t post_id;
    const char* uri;
} PostUriMapping;

const PostUriMapping post_uris[] = {
    {POST_UPLOAD, "/upload"},
    {POST_SETTINGS, "/settings"},
};

static void* current_connection = NULL;
static int8_t current_post_req_id = INVALID_POST;
static FIL current_file;

static bool check_file_ext(char* fname, const char* ext) {
    if (!(*fname) && strlen(fname) > 24) return 0;

    char* fn_cpy = strdup(fname);

    strtok(fn_cpy, ".");
    fn_cpy = strtok(NULL, "");

    return (strcmp(fn_cpy, ext) == 0);
}

static int8_t get_post_id(const char* uri) {
    for (size_t i = 0; i < count_of(post_uris); i++) {
        char* defined_uri = (char*)post_uris[i].uri;

        if (strncmp(uri, defined_uri, strlen(defined_uri)) == 0) {
            return post_uris[i].post_id;
        }
    }
    return INVALID_POST;
}

// Example uri upload?p=payload.txt
static char* extract_uri_param(const char* uri) {
    char* param = strchr(uri, '=') + 1;
    return param;
}

err_t httpd_post_begin(void* connection, const char* uri, const char* http_request, u16_t http_request_len,
                       int content_len, char* response_uri, u16_t response_uri_len, u8_t* post_auto_wnd) {
    LWIP_UNUSED_ARG(http_request);
    LWIP_UNUSED_ARG(http_request_len);
    LWIP_UNUSED_ARG(content_len);

    err_enum_t err = ERR_VAL;

    LOG_INFO(TAG, "URI: %s", uri);

    if (current_connection != connection) {
        current_connection = connection;

        current_post_req_id = get_post_id(uri);

        switch (current_post_req_id) {
            case POST_UPLOAD:

                char* param = extract_uri_param(uri);

                if (check_file_ext(param, "txt")) {
                    fat_io_change_dir("/payloads");
                    err = fat_io_open_write(&current_file, param);
                    fat_io_change_dir("..");
                }

                *post_auto_wnd = 0;
                break;

            case POST_SETTINGS:

                err = fat_io_open_write(&current_file, SETTINGS_FILE_NAME);

                *post_auto_wnd = 0;
                break;

            default: break;
        }
    }

    if (err != ERR_OK) current_post_req_id = INVALID_POST;
    return err != ERR_VAL ? ERR_OK : ERR_VAL;
}

err_t httpd_post_receive_data(void* connection, struct pbuf* p) {
    char buff[1400];
    err_t err = ERR_VAL;

    if (current_connection == connection &&
        (current_post_req_id == POST_UPLOAD || current_post_req_id == POST_SETTINGS)) {
        char* pt = (char*)pbuf_get_contiguous(p, buff, sizeof(buff), p->len, 0);
        FRESULT fr = fat_io_write(&current_file, pt, p->len);
        LOG_INFO(TAG, "fat_io_write: %s (%d)", FRESULT_str(fr), fr);

        pt[p->len] = '\0';

        LOG_DEBUG(TAG, "------POST DATA------");
        LOG_DEBUG(TAG, "%s", pt);
        LOG_DEBUG(TAG, "---------END---------");

        err = ERR_OK;
    }

    pbuf_free(p);
    return err != ERR_VAL ? ERR_OK : ERR_VAL;
}

void httpd_post_finished(void* connection, char* response_uri, u16_t response_uri_len) {
    if (current_connection == connection) {
        snprintf(response_uri, response_uri_len, "/index.shtml");

        if ((current_post_req_id == POST_UPLOAD || current_post_req_id == POST_SETTINGS)) fat_io_close(&current_file);

        if (current_post_req_id == POST_SETTINGS)
            // TODO: if ok
            snprintf(response_uri, response_uri_len, "/index.shtml?settings=ok");

        current_connection = NULL;
        current_post_req_id = INVALID_POST;
    }
}