#include "settings.h"

#include <stdio.h>
#include <string.h>

#include "middleware/debug.h"
#include "middleware/sd_card.h"
#include "pico/stdlib.h"

#define SETTINGS_FILE_PATH "/"
#define SETTINGS_FILE_NAME "settings.txt"

static SettingParam settings[] = {
    {.id = SETTINGS_ID_WEB_SERVER_ENABLED, .tag = "WEBEN", .type = SETTINGS_TYPE_BOOL, .val.b = true},
    {.id = SETTINGS_ID_WIFI_SSID, .tag = "WSSID", .type = SETTINGS_TYPE_STRING, .val.s = "SSID"},
    {.id = SETTINGS_ID_WIFI_PASS, .tag = "WPASS", .type = SETTINGS_TYPE_STRING, .val.s = "PASS"},
    {.id = SETTINGS_ID_PAYLOAD_TO_INJECT, .tag = "PTI", .type = SETTINGS_TYPE_BOOL, .val.b = false},
    {.id = SETTINGS_ID_PAYLOAD_NAME, .tag = "PNAME", .type = SETTINGS_TYPE_STRING, .val.s = "hello_world.txt"}};

static SettingParam* settings_get_param_wtag(const char* tag) {
    for (size_t i = 0; i < SETTINGS_ID_COUNT; i++) {
        char* defined_tag = settings[i].tag;

        if (strncmp(tag, defined_tag, strlen(defined_tag)) == 0) {
            return &settings[i];
        }
    }

    return NULL;
}

SettingParam* settings_get_param_wID(SettingsID id) {
    for (size_t i = 0; i < SETTINGS_ID_COUNT; i++) {
        SettingsID defined_id = settings[i].id;

        if (defined_id == id) {
            return &settings[i];
        }
    }

    return NULL;
}

bool settings_get_bool(SettingsID id) {
    SettingParam* param = settings_get_param_wID(id);
    if (param && param->type == SETTINGS_TYPE_BOOL) {
        return param->val.b;
    }
    return false;
}

void settings_set_bool(SettingsID id, bool val) {
    SettingParam* param = settings_get_param_wID(id);
    if (param && param->type == SETTINGS_TYPE_BOOL) {
        param->val.b = val;
    }
}

static void settings_set_param_value_wtag(const char* tag, const char* value) {
    SettingParam* param = settings_get_param_wtag(tag);

    if (param == NULL) return;

    if (param->type == SETTINGS_TYPE_STRING) {
        memset(param->val.s, 0, SETTINGS_MAX_PARAM_VAL_LEN);
        strncpy(param->val.s, value, SETTINGS_MAX_PARAM_VAL_LEN - 1);
    } else if (param->type == SETTINGS_TYPE_BOOL) {
        param->val.b = (value[0] == '1');
    }
}

// FIX: Bu fonksiyon çalıştığında static settings değiştirilmiyor sanki?
uint8_t settings_set_to_sd(SettingParam* settings) {
    FIL file;
    FRESULT fr;

    char line_buff[SETTINGS_MAX_PARAM_ID_LEN + SETTINGS_MAX_PARAM_VAL_LEN + 2] = {0};

    fr = sd_card_open_write(&file, SETTINGS_FILE_NAME);

    for (size_t i = 0; i < SETTINGS_ID_COUNT; i++) {
        const char* val_str;
        char bool_tmp[2] = {0};

        if (settings[i].type == SETTINGS_TYPE_STRING) {
            val_str = settings[i].val.s;
        } else {
            bool_tmp[0] = settings[i].val.b ? '1' : '0';
            val_str = bool_tmp;
        }

        int32_t written = snprintf(line_buff, count_of(line_buff), "%s=%s\n", settings[i].tag, val_str);
        fr = sd_card_write(&file, line_buff, written);

        if (fr != FR_OK) {
            printf("Settings didn't wrote!");
            return FR_DENIED;
        }
    }

    sd_card_close(&file);

    return fr;
}

uint8_t settings_get_from_sd() {
    FIL file;
    FRESULT fr;

    fr = sd_card_open_read(&file, SETTINGS_FILE_NAME);
    DEBUG_PRINTF("settings_get_from_sd: %s (%d)\n", FRESULT_str(fr), fr);

    // TODO: get ducky script max len from confing file
    char buff[255] = {0};
    // fr = sd_card_read_line(&file, buff, count_of(buff));
    // DEBUG_PRINTF("settings_get_from_sd: %s (%d)\n", FRESULT_str(fr), fr);
    while (sd_card_read_line(&file, buff, count_of(buff)) != NULL) {
        char* param_tag = strtok(buff, "=");

        if (param_tag != NULL) {
            char* param_value = strtok(NULL, "\r\n");
            if (param_value != NULL) {
                // printf("%s=%s\n", param_tag, param_value);
                settings_set_param_value_wtag(param_tag, param_value);
            }
        }
    }
    fr = sd_card_close(&file);

    // settings_print(1);

    return 0;
}

void settings_init() {
    PANIC_IF("SD Card not mounted!", sd_card_is_mounted());

    if (sd_card_check_file_exits(SETTINGS_FILE_NAME) != FR_OK) {
        settings_set_to_sd(settings);
    } else {
        settings_get_from_sd();
    }
}

uint8_t settings_save(void) { return settings_set_to_sd(settings); }

void settings_print(bool show_pass) {
    printf("#### SETTINGS ####\n");
    printf("Wifi SSID(WSSID)    : %s\n", settings_get_param_wID(SETTINGS_ID_WIFI_SSID)->val.s);
    printf("Wifi PASS(WPASS)    : %s\n", show_pass ? settings_get_param_wID(SETTINGS_ID_WIFI_PASS)->val.s : "****");
    printf("PAYLOAD NAME(PNAME) : %s\n", settings_get_param_wID(SETTINGS_ID_PAYLOAD_NAME)->val.s);
    printf("PLUG TO INJECT(PTI) : %s\n", settings_get_bool(SETTINGS_ID_PAYLOAD_TO_INJECT) ? "true" : "false");
    printf("WEB SERVER ENABLED(WEBEN) : %s\n", settings_get_bool(SETTINGS_ID_WEB_SERVER_ENABLED) ? "true" : "false");
    printf("##################\n");
}
