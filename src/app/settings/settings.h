#pragma once

#include "stdbool.h"
#include "stdint.h"

#define SETTINGS_MAX_PARAM_ID_LEN (12)
#define SETTINGS_MAX_PARAM_VAL_LEN (64)
#define SETTINGS_FILE_PATH "/"
#define SETTINGS_FILE_NAME "settings.txt"

typedef enum {
    SETTINGS_ID_WEB_SERVER_ENABLED,
    SETTINGS_ID_WIFI_SSID,
    SETTINGS_ID_WIFI_PASS,
    SETTINGS_ID_PAYLOAD_TO_INJECT,
    SETTINGS_ID_PAYLOAD_NAME,
    SETTINGS_ID_COUNT
} SettingsID;

typedef enum { SETTINGS_TYPE_STRING, SETTINGS_TYPE_BOOL } SettingType;

typedef struct {
    SettingsID id;
    char tag[SETTINGS_MAX_PARAM_ID_LEN];
    SettingType type;
    union {
        char s[SETTINGS_MAX_PARAM_VAL_LEN];
        bool b;
    } val;
} SettingParam;

// typedef struct {
//     SettingParam wifi_ssid;
//     SettingParam wifi_pass;
//     SettingParam pti; /* Enable Plug to inject */
//     SettingParam payload_name; /* Payload of plug to inject */
// } Settings;

void settings_init(void);
uint8_t settings_set_to_sd(SettingParam* settings);
uint8_t settings_get_from_sd();
uint8_t settings_save(void);
void settings_print(bool show_pass);
SettingParam* settings_get_param_wID(SettingsID id);
bool settings_get_bool(SettingsID id);
void settings_set_bool(SettingsID id, bool val);
