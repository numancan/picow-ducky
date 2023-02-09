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
  "PNAME",
  "PTI"
};

settings_t *const settings = &(settings_t) {
  .wifi_ssid = "SSID",
  .wifi_pass = "PASSWORD",
  .payload_name = "hello_world.txt",
  .pti = false
  };


int16_t   get_param()
{
  uint8_t count = count_of(parameters);

  char par[PAR_MAX_LEN] = "";
  flag_finish = sdm_read_until(par, '=', PAR_MAX_LEN);

  if(!par[0]) return -1;

  for (uint8_t i = 0; i < count; i++)
  {
    if (strcmp(par, parameters[i]) == 0)
    {
      return i;
    }
  } 

  return -1;
}

void set_parameter_str(char *param, uint8_t len)
{
  char buff[len];

  flag_finish = sdm_read_until(buff, '\n', len);
  snprintf(param, len, "%s", buff);
}

void set_parameter_bool(bool *param)
{
  char buff[6];

  flag_finish = sdm_read_until(buff, '\n', 6);
  
  *param = memcmp(buff, "true", 5) == 0 ? true : false;
}

uint8_t settings_init(char *sett_path)
{
  if (sdm_open_read(sett_path) != FR_OK) return 1;
  /** TODO: Eğer settings file boş ise return*/

  bool is_finished = 0;
  
  while (!flag_finish)
  {
    switch (get_param())
    {
    case 0: /* WSSID */
      set_parameter_str(settings->wifi_ssid, WSSID_MAX_NAME_LEN);
      break;

    case 1: /* WPASS */
      set_parameter_str(settings->wifi_pass, WPASS_MAX_NAME_LEN);
      break;

    case 2: /* PNAME */
      set_parameter_str(settings->payload_name, FILE_MAX_NAME_LEN);
      break;

    case 3: /* PTI */
      set_parameter_bool(&settings->pti);
      break;
    
    default:
      break;
    }
  }

  sdm_close_file();

  flag_finish = false;

  return 0;
}

void settings_print(bool show_pass)
{
  printf("#### SETTINGS ####\n");
  printf("Wifi SSID(WSSID)    : %s\n", S_WIFI_SSID);
  printf("Wifi PASS(WPASS)    : %s\n", show_pass ? S_WIFI_PASS : "******");
  printf("PAYLOAD NAME(PNAME) : %s\n", S_PAYLOAD_NAME);
  printf("PLUG TO INJECT(PTI) : %s\n", S_PTI ? "true" : "false");
  printf("##################\n");
}
