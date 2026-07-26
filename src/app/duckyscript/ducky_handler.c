#include "ducky_handler.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "ducky_settings.h"
#include "hid/hid_report.h"
#include "hid/layouts/hid_layout.h"
#include "middleware/log.h"
#include "pico/stdlib.h"
#include "task.h"

#define TAG "ducky_handler"

static uint32_t default_char_delay = 0;
static uint32_t default_char_delay_fuzz = 0;
static uint32_t default_delay = 0;
static uint32_t default_delay_fuzz = 0;

static uint32_t get_delay_ms(uint32_t delay, uint32_t fuzz) {
    if (fuzz) delay += (uint32_t)(rand() % (fuzz + 1));
    return delay;
}

static bool handle_key_combo(DuckyLine* ducky_line, HidStatus* status) {
    uint16_t usage = hid_report_str_to_consumer(ducky_line->command);
    if (usage) {
        *status = hid_report_consumer(usage);
        return true;
    }

    uint8_t modifier = hid_report_str_to_mod(ducky_line->command);
    uint8_t special_key = 0;
    uint8_t keycodes[6] = {0};
    uint8_t ki = 0;

    if (!modifier) {
        special_key = hid_report_str_to_special(ducky_line->command);
        keycodes[ki++] = special_key;
    }

    if (!modifier && !special_key) return false;

    char* saveptr;
    char* token = strtok_r(ducky_line->args, " ", &saveptr);
    while (token && ki < 6) {
        uint8_t mod = hid_report_str_to_mod(token);
        if (mod) {
            modifier |= mod;
        } else {
            uint8_t spec = hid_report_str_to_special(token);
            if (spec) {
                keycodes[ki++] = spec;
            } else {
                size_t len = strlen(token);
                for (size_t i = 0; i < len && ki < 6; i++) {
                    uint8_t kc = hid_report_char_to_keycode(token[i], NULL);
                    if (kc) keycodes[ki++] = kc;
                }
            }
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    *status = hid_report_keys(modifier, keycodes);
    return true;
}

static DuckyCommandID get_ducky_command_id(const char* command_str) {
#define X(id, str) \
    if (strcmp(command_str, str) == 0) return id;
    DUCKY_COMMAND_LIST(X)
#undef X

    return DUCKY_COMMAND_COUNT;  // no match: handled as a key combo
}

void ducky_handler_init(const DuckySettings* settings) {
    default_delay = settings->delay;
    default_delay_fuzz = settings->fuzz_delay;
    default_char_delay = settings->char_delay;
    default_char_delay_fuzz = settings->char_fuzz_delay;

    hid_layout_set(settings->kb_layout);
}

// TODO: consider using strcasecmp for commands
bool ducky_handler_exec_line(DuckyLine* ducky_line) {
    if (!ducky_line->command[0]) return true;

    DuckyCommandID cmd_id = get_ducky_command_id(ducky_line->command);

    switch (cmd_id) {
        case REM: return true;

        case DELAY: {
            uint32_t delay_ms = (uint32_t)atoi(ducky_line->args);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
            return true;
        }
        case DEFAULTDELAY: {
            default_delay = (uint32_t)atoi(ducky_line->args);
            return true;
        }
        case DEFAULTDELAYFUZZ: {
            default_delay_fuzz = (uint32_t)atoi(ducky_line->args);
            return true;
        }
        case DEFAULTCHARDELAY: {
            default_char_delay = (uint32_t)atoi(ducky_line->args);
            return true;
        }
        case DEFAULTCHARDELAYFUZZ: {
            default_char_delay_fuzz = (uint32_t)atoi(ducky_line->args);
            return true;
        }
        case STRING: {
            uint32_t char_delay = get_delay_ms(default_char_delay, default_char_delay_fuzz);
            HidStatus st = hid_report_string(ducky_line->args, char_delay);
            vTaskDelay(pdMS_TO_TICKS(get_delay_ms(default_delay, default_delay_fuzz)));
            return st == HID_STATUS_OK;
        }
        case LAYOUT: {
            if (!hid_layout_set_by_name(ducky_line->args)) {
                LOG_ERROR(TAG, "Unknown layout: %s\n", ducky_line->args);
            }
            vTaskDelay(pdMS_TO_TICKS(get_delay_ms(default_delay, default_delay_fuzz)));
            return true;
        }
        case MOUSE_MOVE: {
            int x = 0, y = 0;
            sscanf(ducky_line->args, "%d %d", &x, &y);
            HidStatus st = hid_report_mouse_move((int8_t)x, (int8_t)y);
            vTaskDelay(pdMS_TO_TICKS(get_delay_ms(default_delay, default_delay_fuzz)));
            return st == HID_STATUS_OK;
        }
        case MOUSE_CLICK: {
            uint8_t b = 0;
            if (!strcmp(ducky_line->args, "LEFT"))
                b = MOUSE_BUTTON_LEFT;
            else if (!strcmp(ducky_line->args, "RIGHT"))
                b = MOUSE_BUTTON_RIGHT;
            else if (!strcmp(ducky_line->args, "MIDDLE"))
                b = MOUSE_BUTTON_MIDDLE;

            HidStatus st = HID_STATUS_OK;
            if (b) st = hid_report_mouse_click(b);
            vTaskDelay(pdMS_TO_TICKS(get_delay_ms(default_delay, default_delay_fuzz)));
            return st == HID_STATUS_OK;
        }
        case MOUSE_SCROLL: {
            int w = atoi(ducky_line->args);
            HidStatus st = hid_report_mouse_scroll((int8_t)w);
            vTaskDelay(pdMS_TO_TICKS(get_delay_ms(default_delay, default_delay_fuzz)));
            return st == HID_STATUS_OK;
        }
        case DUCKY_COMMAND_COUNT:
        default: {
            // not a known command: try a key combo
            HidStatus st = HID_STATUS_OK;
            bool handled = handle_key_combo(ducky_line, &st);
            if (handled) {
                vTaskDelay(pdMS_TO_TICKS(get_delay_ms(default_delay, default_delay_fuzz)));
            }
            return st == HID_STATUS_OK;
        }
    }
}