#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "config.h"

typedef struct
{
  char      wifi_ssid[WSSID_MAX_NAME_LEN];
  char      wifi_pass[WPASS_MAX_NAME_LEN];
  char      payload_name[FILE_MAX_NAME_LEN]; /** */
  bool      pti;
} settings_t;
extern settings_t *const settings;

#define S_WIFI_SSID (settings->wifi_ssid)
#define S_WIFI_PASS (settings->wifi_pass)
#define S_PAYLOAD_NAME (settings->payload_name)
#define S_PTI (settings->pti)

/**
 * @brief Initialize settings with reading settings.txt from SD Card.
 * 
 * @param sett_path Path of settings.txt on SD Card
 * 
 * @return TODO: return 0 is OK!
 */
uint8_t settings_init(char *sett_path);

/**
 * @brief 
 * 
 */
void settings_print(bool show_pass);


#endif