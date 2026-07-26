#include "ssi.h"

#include <lwip/arch.h>
#include <string.h>

#include "duckyscript/web/ducky_web.h"
#include "lwip/apps/fs.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwipopts.h"
#include "middleware/log.h"
#include "net_manager/web/provisioning_web.h"

static const char* TAG = "ssi";

enum { SSI_TAG_SCANJS = 0, SSI_TAG_PSTATUS, SSI_TAG_PAYLOAD, SSI_TAG_SETTINGS, SSI_TAG_DSTATUS, SSI_TAG_COUNT };

/* Tag names are limited to LWIP_HTTPD_MAX_TAG_NAME_LEN (default 8) chars. */
static const char* ssi_tags[SSI_TAG_COUNT] = {
    "scan", "pstatus", "payload", "settings", "dstatus",
};

// lwIP per-connection SSI state hooks. The payload list streams a directory and needs state;
// the actual state lives in duckyscript (ducky_ssi_state_*), we just forward here.
void* fs_state_init(struct fs_file* file, const char* name) {
    (void)file;
    return ducky_ssi_state_init(name);
}

void fs_state_free(struct fs_file* file, void* state) {
    (void)file;
    ducky_ssi_state_free(state);
}

u16_t ssi_handler(int iIndex, char* pcInsert, int iInsertLen, u16_t current_tag_part, u16_t* next_tag_part,
                  void* connection_state) {
    u16_t printed;

    LOG_DEBUG(TAG, "ssi_handler: %s %d", ssi_tags[iIndex], iInsertLen);

    switch (iIndex) {
        case SSI_TAG_PAYLOAD:
            printed = ducky_ssi_payload(connection_state, pcInsert, iInsertLen, current_tag_part, next_tag_part);
            break;

        case SSI_TAG_SETTINGS: printed = ducky_ssi_settings(pcInsert, iInsertLen); break;

        case SSI_TAG_DSTATUS: printed = ducky_ssi_status(pcInsert, iInsertLen); break;

        case SSI_TAG_SCANJS:
            printed = provisioning_ssi_scanjs(pcInsert, iInsertLen, current_tag_part, next_tag_part);
            break;

        case SSI_TAG_PSTATUS: printed = provisioning_ssi_status(pcInsert, iInsertLen); break;

        default: /* unknown tag */ printed = 0; break;
    }

    return printed;
}

void ssi_init() {
    for (size_t i = 0; i < LWIP_ARRAYSIZE(ssi_tags); i++) {
        LWIP_ASSERT("tag too long for LWIP_HTTPD_MAX_TAG_NAME_LEN", strlen(ssi_tags[i]) <= LWIP_HTTPD_MAX_TAG_NAME_LEN);
    }

    http_set_ssi_handler(ssi_handler, ssi_tags, SSI_TAG_COUNT);
}
