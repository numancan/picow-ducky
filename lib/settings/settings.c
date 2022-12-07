#include <string.h>
#include <stdio.h>

#include "sd_memory/sd_memory.h"
#include "settings.h"

/** Parameter maximum length in settings file */
#define PAR_MAX_LEN 12

static bool flag_finish = false;

const char const *parameters[] = {
  "WSSID",
  "WPASS",
  "PNAME"
};

settings_t *const settings = &(settings_t) {
  .wifi_ssid = "SSID",
  .wifi_pass = "PASSWORD",
  .payload_name = "hello_world.txt"};


int16_t get_param()
{
  uint8_t count = count_of(parameters);

  char par[PAR_MAX_LEN] = "";
  flag_finish = sdm_read_until(par, '=', PAR_MAX_LEN);

  for (uint8_t i = 0; i < count; i++)
  {
    if (strcmp(par, parameters[i]) == 0)
    {
      return i;
    }
  } 

  return -1;
}

void set_parameter(char *param, uint8_t len)
{
  char buff[len];

  flag_finish = sdm_read_until(buff, '\n', len);
  snprintf(param, len, "%s", buff);
}

uint8_t settings_init(char *sett_path)
{
  if (sdm_open_read(sett_path) != FR_OK) return 1;

  bool is_finished = 0;
  
  while (!flag_finish)
  {
    switch (get_param())
    {
    case 0: /* WSSID */
      set_parameter(settings->wifi_ssid, WSSID_MAX_NAME_LEN);
      break;

    case 1: /* WPASS */
      set_parameter(settings->wifi_pass, WPASS_MAX_NAME_LEN);
      break;

    case 2: /* PNAME */
      set_parameter(settings->payload_name, FILE_MAX_NAME_LEN);
      break;
    
    default:
      break;
    }
  }

  printf("%s %s %s\n", S_WIFI_SSID, S_WIFI_PASS, S_PAYLOAD_NAME);
  sdm_close_file();

  flag_finish = false;

  return 0;
}
