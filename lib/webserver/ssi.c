#include <string.h>

#include "pico/platform.h"
#include "lwip/def.h"
#include "lwip/apps/httpd.h"
#include "lwipopts.h"
#include "sd_memory.h"
#include "settings/settings.h"

#include "ssi.h"

// max length of the tags defaults to be 8 chars
// LWIP_HTTPD_MAX_TAG_NAME_LEN
const char * __not_in_flash("httpd") ssi_example_tags[] = {
  "payloads",
  "wssid",
  "wpass",
  "pname"
};

u16_t __time_critical_func(ssi_handler)(int iIndex, char *pcInsert, int iInsertLen) {
  size_t printed;
  
  switch (iIndex) {
  case 0: /* "payloads" */

    sdm_read_dir("/payloads", pcInsert, iInsertLen);

    // printed = strlen(pcInsert);
    printed = snprintf(pcInsert, iInsertLen, "%s", pcInsert);
    break;

  case 1: /* "wssid" */
    {
      printed = snprintf(pcInsert, iInsertLen, "<input type='text' id='wssid' name='WSSID' value='%s'>", S_WIFI_SSID);
      printf("insert len:%d\n", iInsertLen);
    }
    break;
  
  case 2: /* "wpass" */
    {
      printed = snprintf(pcInsert, iInsertLen, "<input type='text' id='wpass' name='WPASS' value='%s'>", S_WIFI_PASS);
      printf("insert len:%d\n", iInsertLen);
    }
    break;

  case 3: /* "pname" */
    {
      printed = snprintf(pcInsert, iInsertLen, "<input type='text' id='pname' name='PNAME' value='%s'>", S_PAYLOAD_NAME);
      printf("insert len:%d\n", iInsertLen);
    }
    break;

  default: /* unknown tag */
    printed = 0;
    break;
  }

  LWIP_ASSERT("sane length", printed <= 0xFFFF);
  return (u16_t)printed;
}

void ssi_init() {
  size_t i;
  for (i = 0; i < LWIP_ARRAYSIZE(ssi_example_tags); i++) {
    LWIP_ASSERT("tag too long for LWIP_HTTPD_MAX_TAG_NAME_LEN",
      strlen(ssi_example_tags[i]) <= LWIP_HTTPD_MAX_TAG_NAME_LEN);
  }

  http_set_ssi_handler(ssi_handler,
    ssi_example_tags, LWIP_ARRAYSIZE(ssi_example_tags)
  );
}
