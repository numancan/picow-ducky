#include <string.h>

// #include "pico.h"
#include "app/settings/settings.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwipopts.h"
#include "middleware/fat_io.h"
#include "middleware/log.h"
#include "ssi.h"

static const char* TAG = "ssi";

// Be aware of LWIP_HTTPD_MAX_TAG_NAME_LEN
static const char* ssi_tags[] = {"payload", "wssid", "wpass", "pname", "pti", "weben"};

// TODO: Use SSI_MULTIPART because payload list will be more than
// LWIP_HTTPD_MAX_TAG_INSERT_LEN LWIP_HTTPD_MAX_TAG_INSERT_LEN max length of the tags
// defaults to be 8 chars LWIP_HTTPD_MAX_TAG_NAME_LEN

u16_t ssi_handler(int iIndex, char* pcInsert, int iInsertLen) {
    size_t printed;

    LOG_INFO(TAG, "ssi_handler: %d %d\n", iIndex, iInsertLen);

    switch (iIndex) {
        case 0: /* "payload" */

            // TODO: check max lenght
            fat_io_list_dir("/payloads", pcInsert, iInsertLen, "");
            // TODO: fix this no need recpy
            printed = snprintf(pcInsert, iInsertLen, "%s", pcInsert);
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
