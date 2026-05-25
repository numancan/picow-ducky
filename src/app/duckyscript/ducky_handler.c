#include "ducky_handler.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "class/hid/hid.h"
#include "hid/usb_device.h"
#include "hid/usb_hid.h"
#include "middleware/debug.h"
#include "pico/stdlib.h"
#include "task.h"


static uint32_t default_delay_ms = 0;
static uint32_t default_delay_fuzz = 0;

static void default_delay(void) {
    uint32_t delay = default_delay_ms;
    if (default_delay_fuzz) delay += (uint32_t)(rand() % (default_delay_fuzz + 1));
    vTaskDelay(pdMS_TO_TICKS(delay));
}

static bool handle_key_combo(ducky_line_t* ducky_line) {
    uint16_t usage = usb_hid_report_str_to_consumer(ducky_line->command);
    if (usage) {
        usb_hid_report_consumer(usage);
        return true;
    }

    uint8_t modifier = usb_hid_report_str_to_mod(ducky_line->command);
    uint8_t special_key = 0;
    uint8_t keycodes[6] = {0};
    uint8_t ki = 0;

    if (!modifier) {
        special_key = usb_hid_report_str_to_special(ducky_line->command);
        keycodes[ki++] = special_key;
    }

    if (!modifier && !special_key) return false;

    char* token = strtok(ducky_line->args, " ");
    while (token && ki < 6) {
        uint8_t spec = usb_hid_report_str_to_special(token);
        if (spec) {
            keycodes[ki++] = spec;
        } else {
            for (size_t i = 0; i < strlen(token) && ki < 6; i++) {
                uint8_t kc = usb_hid_report_char_to_keycode(token[i], NULL);
                if (kc) keycodes[ki++] = kc;
            }
        }
        token = strtok(NULL, " ");
    }

    usb_hid_report_keys(modifier, keycodes);
    return true;
}

void ducky_handler_exec_line(ducky_line_t* ducky_line) {
    if (!ducky_line->command[0]) return;
    if (strcmp(ducky_line->command, "REM") == 0) return;

    if (strcmp(ducky_line->command, "DELAY") == 0) {
        uint32_t delay_ms = (uint32_t)atoi(ducky_line->args);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        return;

    } else if (strcmp(ducky_line->command, "DEFAULTDELAY") == 0) {
        default_delay_ms = (uint32_t)atoi(ducky_line->args);
        return;

    } else if (strcmp(ducky_line->command, "DEFAULTDELAYFUZZ") == 0) {
        default_delay_fuzz = (uint32_t)atoi(ducky_line->args);
        return;

    } else if (strcmp(ducky_line->command, "DEFAULTCHARDELAY") == 0) {
        uint32_t default_char_delay = (uint32_t)atoi(ducky_line->args);
        usb_hid_report_set_char_delay_ms(default_char_delay);
        return;

    } else if (strcmp(ducky_line->command, "DEFAULTCHARDELAYFUZZ") == 0) {
        uint32_t default_char_delay_fuzz = (uint32_t)atoi(ducky_line->args);
        usb_hid_report_set_char_delay_fuzz(default_char_delay_fuzz);
        return;

    } else if (strcmp(ducky_line->command, "STRING") == 0) {
        usb_hid_report_string(ducky_line->args);
        default_delay();

    } else if (strcmp(ducky_line->command, "LAYOUT") == 0) {
        if (!strcmp(ducky_line->args, "TR_Q"))
            hid_report_set_layout(HID_KEY_LAYOUT_TR_Q);
        else if (!strcmp(ducky_line->args, "US_Q"))
            hid_report_set_layout(HID_KEY_LAYOUT_US_Q);
        else
            printf("Unknown layout: %s\n", ducky_line->args);

        default_delay();

    } else if (strcmp(ducky_line->command, "MOUSE_MOVE") == 0) {
        int x = 0, y = 0;
        sscanf(ducky_line->args, "%d %d", &x, &y);
        usb_hid_report_mouse_move((int8_t)x, (int8_t)y);
        default_delay();

    } else if (strcmp(ducky_line->command, "MOUSE_CLICK") == 0) {
        uint8_t b = 0;
        if (!strcmp(ducky_line->args, "LEFT"))
            b = MOUSE_BUTTON_LEFT;
        else if (!strcmp(ducky_line->args, "RIGHT"))
            b = MOUSE_BUTTON_RIGHT;
        else if (!strcmp(ducky_line->args, "MIDDLE"))
            b = MOUSE_BUTTON_MIDDLE;
        if (b) usb_hid_report_mouse_click(b);
        default_delay();

    } else if (strcmp(ducky_line->command, "MOUSE_SCROLL") == 0) {
        int w = atoi(ducky_line->args);
        usb_hid_report_mouse_scroll((int8_t)w);
        default_delay();

    } else {
        bool handled = handle_key_combo(ducky_line);
        if (handled) default_delay();
    }
}