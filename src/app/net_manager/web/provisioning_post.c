#include "provisioning_web.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lwip/pbuf.h"
#include "middleware/log.h"
#include "net_manager/net_manager.h"

static const char* TAG = "POST";

static bool post_creds_ok; /* body parsed successfully */
static char ssid[NET_SSID_MAX + 1];
static char password[NET_PASS_MAX + 1];

/* Decodes application/x-www-form-urlencoded text in place:
 * '+' -> ' ', "%XX" -> byte. Trailing spaces are trimmed. */
static void url_decode(char* dst, const char* src, size_t dst_size) {
    size_t di = 0;
    while (*src && di + 1 < dst_size) {
        char c = *src;
        if (c == '+') {
            dst[di++] = ' ';
            src++;
        } else if (c == '%' && isxdigit((unsigned char)src[1]) && isxdigit((unsigned char)src[2])) {
            int hi = src[1] <= '9' ? src[1] - '0' : (src[1] | 0x20) - 'a' + 10;
            int lo = src[2] <= '9' ? src[2] - '0' : (src[2] | 0x20) - 'a' + 10;
            dst[di++] = (char)((hi << 4) | lo);
            src += 3;
        } else {
            dst[di++] = c;
            src++;
        }
    }
    while (di > 0 && dst[di - 1] == ' ') di--;
    dst[di] = '\0';
}

/* Extracts the raw (url-encoded) value of a form field from the POST body
 * (pbuf). Writes it null-terminated into value_buf; returns NULL if the
 * field is missing or does not fit. */
static char* post_param_value(struct pbuf* p, const char* param_name, char* value_buf, size_t value_buf_len) {
    size_t param_len = strlen(param_name);
    u16_t param_pos = pbuf_memfind(p, param_name, param_len, 0);
    if (param_pos == 0xFFFF) {
        return NULL;
    }
    u16_t value_pos = param_pos + param_len;
    u16_t amp_pos = pbuf_memfind(p, "&", 1, value_pos);
    u16_t value_len = (amp_pos != 0xFFFF) ? (amp_pos - value_pos) : (p->tot_len - value_pos);
    if (value_len == 0 || value_len >= value_buf_len) {
        return NULL;
    }
    char* result = (char*)pbuf_get_contiguous(p, value_buf, value_buf_len, value_len, value_pos);
    if (result) {
        result[value_len] = '\0';
    }
    return result;
}

static err_t connect_begin(const char* uri, u8_t* post_auto_wnd) {
    (void)uri;
    post_creds_ok = false;
    // The small two-field form fits in a single pbuf; let httpd advance the window.
    *post_auto_wnd = 1;
    return ERR_OK;
}

static err_t connect_receive(struct pbuf* p) {
    err_t ret = ERR_VAL;

    char ssid_encoded[NET_SSID_MAX + 1];
    char password_encoded[NET_PASS_MAX + 1];
    char* ssid_raw = post_param_value(p, "ssid=", ssid_encoded, sizeof(ssid_encoded));
    char* password_raw = post_param_value(p, "password=", password_encoded, sizeof(password_encoded));

    if (ssid_raw != NULL) {
        url_decode(ssid, ssid_raw, sizeof(ssid));
        // Password may be empty/missing (open network) -> empty string.
        url_decode(password, password_raw ? password_raw : "", sizeof(password));
        post_creds_ok = true;
        ret = ERR_OK;
    } else {
        LOG_ERROR(TAG, "POST /connect: ssid missing");
    }

    pbuf_free(p);
    return ret;
}

static void connect_finished(char* response_uri, u16_t response_uri_len) {
    snprintf(response_uri, response_uri_len, "/index.html");
    if (post_creds_ok) {
        net_manager_submit_credentials(ssid, password);
    }
}

const PostHandler provisioning_connect_post_handler = {
    .begin = connect_begin,
    .receive = connect_receive,
    .finished = connect_finished,
};
