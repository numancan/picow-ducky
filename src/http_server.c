#include <string.h>

#include "bsp/board.h"

#include "pico/cyw43_arch.h"
#include "lwip/apps/httpd.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwipopts.h"

#if !LWIP_HTTPD_FILE_STATE
#error LWIP_HTTPD_EXAMPLE_SSI_SIMPLE_CGI_INTEGRATION needs LWIP_HTTPD_FILE_STATE
#endif
#if !LWIP_HTTPD_CGI_SSI
#error LWIP_HTTPD_EXAMPLE_SSI_SIMPLE_CGI_INTEGRATION needs LWIP_HTTPD_CGI_SSI
#endif

#define MAX_CGI_LEN   16

const char * ssi_example_tags[] = {
  "HellWorl",
  "counter",
  "MultPart",
  "CgiParam"
};

u16_t ssi_example_ssi_handler(
#if LWIP_HTTPD_SSI_RAW
                             const char* ssi_tag_name,
#else /* LWIP_HTTPD_SSI_RAW */
                             int iIndex,
#endif /* LWIP_HTTPD_SSI_RAW */
                             char *pcInsert, int iInsertLen
#if LWIP_HTTPD_SSI_MULTIPART
                             , u16_t current_tag_part, u16_t *next_tag_part
#endif /* LWIP_HTTPD_SSI_MULTIPART */
#if defined(LWIP_HTTPD_FILE_STATE) && LWIP_HTTPD_FILE_STATE
                             , void *connection_state
#endif /* LWIP_HTTPD_FILE_STATE */
                             )
{
  size_t printed;
#if LWIP_HTTPD_SSI_RAW
  /* a real application could use if(!strcmp) blocks here, but we want to keep
     the differences between configurations small, so translate string to index here */
  int iIndex;
  for (iIndex = 0; iIndex < LWIP_ARRAYSIZE(ssi_example_tags); iIndex++) {
    if(!strcmp(ssi_tag_name, ssi_example_tags[iIndex])) {
      break;
    }
  }
#endif
#if defined(LWIP_HTTPD_FILE_STATE) && LWIP_HTTPD_FILE_STATE
  LWIP_UNUSED_ARG(connection_state);
#endif

  switch (iIndex) {
  case 0: /* "HelloWorld" */
    printed = snprintf(pcInsert, iInsertLen, "Hello World!");
    break;
  case 1: /* "counter" */
    {
      static int counter;
      counter++;
      printed = snprintf(pcInsert, iInsertLen, "%d", counter);
    }
    break;
  case 2: /* "MultPart" */
#if LWIP_HTTPD_SSI_MULTIPART
    switch (current_tag_part) {
    case 0:
      printed = snprintf(pcInsert, iInsertLen, "part0");
      *next_tag_part = 1;
      break;
    case 1:
      printed = snprintf(pcInsert, iInsertLen, "part1");
      *next_tag_part = 2;
      break;
    case 2:
      printed = snprintf(pcInsert, iInsertLen, "part2");
      break;
    default:
      printed = snprintf(pcInsert, iInsertLen, "unhandled part: %d", (int)current_tag_part);
      break;
    }
#else
    printed = snprintf(pcInsert, iInsertLen, "LWIP_HTTPD_SSI_MULTIPART disabled");
#endif
    break;
  case 3:
    if (connection_state) {
      char *params = (char *)connection_state;
      if (*params) {
        printed = snprintf(pcInsert, iInsertLen, "%s", (char *)params);
      } else {
        printed = snprintf(pcInsert, iInsertLen, "none");
      }
    } else {
       printed = snprintf(pcInsert, iInsertLen, "NULL");
    }
    break;
  default: /* unknown tag */
    printed = 0;
    break;
  }
  LWIP_ASSERT("sane length", printed <= 0xFFFF);
  return (u16_t)printed;
}

void ssi_ex_init(void)
{
  int i;
  for (i = 0; i < LWIP_ARRAYSIZE(ssi_example_tags); i++) {
    LWIP_ASSERT("tag too long for LWIP_HTTPD_MAX_TAG_NAME_LEN",
      strlen(ssi_example_tags[i]) <= LWIP_HTTPD_MAX_TAG_NAME_LEN);
  }

  http_set_ssi_handler(ssi_example_ssi_handler,
#if LWIP_HTTPD_SSI_RAW
    NULL, 0
#else
    ssi_example_tags, LWIP_ARRAYSIZE(ssi_example_tags)
#endif
    );
}

void *fs_state_init(struct fs_file *file, const char *name)
{
  char *ret;
  LWIP_UNUSED_ARG(file);
  LWIP_UNUSED_ARG(name);
  ret = (char *)mem_malloc(MAX_CGI_LEN);
  if (ret) {
    *ret = 0;
  }
  return ret;
}

void fs_state_free(struct fs_file *file, void *state)
{
  LWIP_UNUSED_ARG(file);
  if (state != NULL) {
    mem_free(state);
  }
}

void httpd_cgi_handler(struct fs_file *file, const char* uri, int iNumParams,
                              char **pcParam, char **pcValue
#if defined(LWIP_HTTPD_FILE_STATE) && LWIP_HTTPD_FILE_STATE
                                     , void *connection_state
#endif /* LWIP_HTTPD_FILE_STATE */
                                     )
{
  LWIP_UNUSED_ARG(file);
  LWIP_UNUSED_ARG(uri);
  if (connection_state != NULL) {
    char *start = (char *)connection_state;
    char *end = start + MAX_CGI_LEN;
    int i;
    memset(start, 0, MAX_CGI_LEN);
    /* print a string of the arguments: */
    for (i = 0; i < iNumParams; i++) {
      size_t len;
      len = end - start;
      if (len) {
        size_t inlen = strlen(pcParam[i]);
        size_t copylen = LWIP_MIN(inlen, len);
        memcpy(start, pcParam[i], copylen);
        start += copylen;
        len -= copylen;
      }
      if (len) {
        *start = '=';
        start++;
        len--;
      }
      if (len) {
        size_t inlen = strlen(pcValue[i]);
        size_t copylen = LWIP_MIN(inlen, len);
        memcpy(start, pcValue[i], copylen);
        start += copylen;
        len -= copylen;
      }
      if (len) {
        *start = ';';
        len--;
      }
      /* ensure NULL termination */
      end--;
      *end = 0;
    }
  }
}

enum {
  WIFI_NOT_CONNECTED = 0,
  WIFI_CONNECTED = 1,
  WIFI_CONNECTING = 250,
  WIFI_CONNECTION_FAILED = 750
};

uint32_t wifi_status = WIFI_NOT_CONNECTED;

void wifi_status_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  if (!wifi_status) return;
  if (wifi_status == WIFI_CONNECTED)

  switch (wifi_status)
  {
    case WIFI_NOT_CONNECTED:
    return;
    break;

    case WIFI_CONNECTED:
      if (!cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN))
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      return;
    break;
  
  default:
    // Blink every interval ms
    if ( board_millis() - start_ms < wifi_status) return; // not enough time
    start_ms += wifi_status;

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
    
    led_state = 1 - led_state; // toggle
    break;
  }

}

int connect_wifi()
{
  if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
  } 

    cyw43_arch_enable_sta_mode();
    
    // this seems to be the best be can do using the predefined `cyw43_pm_value` macro:
    // cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM);
    // however it doesn't use the `CYW43_NO_POWERSAVE_MODE` value, so we do this instead:
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    // "Luna_Misafir", "Luna1991"
    // "RIFKI", "9RpGe_8VbNtzY65k@"
    wifi_status = WIFI_CONNECTING;
    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms("Luna_Misafir", "Luna1991", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        wifi_status = WIFI_CONNECTION_FAILED;
        return 1;
    } else {
        wifi_status = WIFI_CONNECTED;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        printf("Connected.\n");
        printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), 80);

        extern cyw43_t cyw43_state;
      } 

  return 0;
}

int http_server_init()
{
  int connection_status = connect_wifi();
  httpd_init();
  printf("Http server initialized.\n");
  
  ssi_ex_init();

  return connection_status;
}
