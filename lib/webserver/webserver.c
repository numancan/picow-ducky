#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "lwipopts.h"
#include "ssi.h"

#include "webserver.h"

#define WIFI_CONNECTION_TIMEOUT 30000

ws_err_t ws_init(const char *ssid, const char *pass)
{
  if (cyw43_arch_init()) return WS_ERR_WIFI_INIT;

  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
  sleep_ms(500);
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
  
  cyw43_arch_enable_sta_mode();

  // if (cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM)) return WS_ERR_WIFI_INIT;
  cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));

  if (cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, WIFI_CONNECTION_TIMEOUT)) {
      printf("Wifi connection failed!\n");
      return WS_ERR_WIFI_CONN;
  } else {
      printf("Wifi connected!\n");
      cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
      
      ip_addr_t *ip_addr = &cyw43_state.netif[CYW43_ITF_STA].ip_addr;
      printf("IP Address: %s\n", ip4addr_ntoa(ip_addr));
  }

  httpd_init();
  ssi_init();
  printf("HTTPD&SSI initialized!\n");

  return WS_ERR_OK;
}