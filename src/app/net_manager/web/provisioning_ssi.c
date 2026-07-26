#include "provisioning_web.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lwip/apps/httpd.h"
#include "net_manager/net_manager.h"
#include "net_manager/wifi_scan_list.h"
#include "ssi.h"

/* Scan snapshot held across multipart SSI calls of a single request. */
static WifiScanEntry snap[WIFI_SCAN_MAX_ENTRIES];
static size_t snap_count;

/* Makes an SSID safe as a JSON string:
 * ", \ and control characters are escaped. */
static size_t json_escape(const char* in, char* out, size_t out_size) {
    size_t o = 0;
    for (size_t i = 0; in[i] != '\0'; i++) {
        unsigned char c = (unsigned char)in[i];
        const char* rep = NULL;
        char ubuf[7];

        switch (c) {
            case '\"': rep = "\\\""; break;
            case '\\': rep = "\\\\"; break;
            default:
                if (c < 0x20) {
                    snprintf(ubuf, sizeof(ubuf), "\\u%04x", c);
                    rep = ubuf;
                }
                break;
        }

        if (rep != NULL) {
            size_t rl = strlen(rep);
            if (o + rl >= out_size) break;
            memcpy(&out[o], rep, rl);
            o += rl;
        } else {
            if (o + 1 >= out_size) break;
            out[o++] = (char)c;
        }
    }
    out[o] = '\0';
    return o;
}

/* scanjs: emits the scan list as a JSON array (multipart). */
int provisioning_ssi_scanjs(char* pcInsert, int iInsertLen, u16_t current_tag_part, u16_t* next_tag_part) {
    int len = 0;

    if (current_tag_part == 0) {
        snap_count = wifi_scan_list_get(snap, WIFI_SCAN_MAX_ENTRIES);

        if (snap_count == 0) {
            len = snprintf(pcInsert, iInsertLen, "[]");
            /* *next_tag_part not set => tag is finished */
        } else {
            len = snprintf(pcInsert, iInsertLen, "[");
            *next_tag_part = 1;
        }
    } else {
        size_t i = (size_t)current_tag_part - 1;

        if (i >= snap_count) {
            /* Defensive guard: close the JSON on an unexpected state */
            len = snprintf(pcInsert, iInsertLen, "]");
        } else {
            char esc[WIFI_SCAN_SSID_MAX_LEN * 6 + 1]; /* worst case \u00xx */
            json_escape(snap[i].ssid, esc, sizeof(esc));

            bool last = (i == snap_count - 1);

            len = snprintf(pcInsert, iInsertLen, "%s{\"ssid\":\"%s\",\"rssi\":%d,\"locked\":%s}%s", (i > 0) ? "," : "",
                           esc, (int)snap[i].rssi, snap[i].locked ? "true" : "false", last ? "]" : "");

            if (!last) {
                *next_tag_part = current_tag_part + 1;
            }
        }
    }
    return len;
}

/* status: emits the raw NetState value (not its label); the web UI resolves it via
 * NET_STATUS_LIST. */
int provisioning_ssi_status(char* pcInsert, int iInsertLen) {
    int n = snprintf(pcInsert, iInsertLen, "{\"state\":%d}", net_manager_get_state());

    if (n < 0 || n >= iInsertLen) return 0;
    return n;
}
