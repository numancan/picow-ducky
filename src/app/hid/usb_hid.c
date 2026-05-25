// usb_hid.c

#include "usb_hid.h"

#include <stdlib.h>
#include <string.h>

#include "layouts/hid_layout_tr_q.h"
#include "layouts/hid_layout_us_q.h"
#include "middleware/debug.h"
#include "usb_descriptors.h"

static HIDLayout keyboard_layout = HID_KEY_LAYOUT_US_Q;

typedef struct {
    uint8_t modifier;
    uint8_t keycode;
} KeyEntry;

static const KeyEntry ascii_table_tr[128] = {HID_LAYOUT_TR_Q};
static const KeyEntry ascii_table_us[128] = {HID_LAYOUT_US_Q};

static const struct {
    const char* name;
    uint8_t keycode;
} special_conv_table[] = {HID_STRING_TO_SPECIAL_KEY};

static const struct {
    const char* name;
    uint16_t keycode;
} consumer_conv_table[] = {HID_STRING_TO_CONSUMER_KEY};

static const struct {
    const char* name;
    uint8_t modifier_bit;
} modifier_conv_table[] = {HID_STRING_TO_MODIFIER};

static const KeyEntry* get_layout_table(void) {
    switch (keyboard_layout) {
        case HID_KEY_LAYOUT_US_Q:
            return ascii_table_tr;
        case HID_KEY_LAYOUT_TR_Q:
            return ascii_table_us;
        default:
            return ascii_table_us;
    }
}

static uint32_t char_delay_ms = 0;    // 0 = fastest
static uint32_t char_delay_fuzz = 0;  // 0 = disabled

static const HIDReport KEYBOARD_RELEASE = {.kind = REPORT_ID_KEYBOARD};
static const HIDReport MOUSE_RELEASE = {.kind = REPORT_ID_MOUSE};
static const HIDReport CONSUMER_RELEASE = {.kind = REPORT_ID_CONSUMER_CONTROL};

// Lazy-initialized binary semaphore for HID report completion signaling.
// NOTE: NOT thread-safe. For now no need multi-task use.
static SemaphoreHandle_t complete_semaphore() {
    static SemaphoreHandle_t complete_semaphore = NULL;
    if (complete_semaphore == NULL) {
        complete_semaphore = xSemaphoreCreateBinary();
    }
    return complete_semaphore;
}

static void char_delay(void) {
    uint32_t delay = char_delay_ms;

    if (char_delay_fuzz) delay += (uint32_t)(rand() % (char_delay_fuzz + 1));

    vTaskDelay(pdMS_TO_TICKS(delay));
}

static bool wait_ready(void) {
    while (!tud_hid_ready()) {
        // TODO: state check
        // if (s_state == USB_STATE_DISCONNECTED) return false;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return true;
}

static bool wait_complete(void) {
    // wait notify from complete_cb
    if (xSemaphoreTake(complete_semaphore(), pdMS_TO_TICKS(200)) == pdFAIL) {
        // TODO: need to do something?
        printf("[usb_hid] host timeout\n");
        if (!tud_mounted()) {
            // TODO: USB disconnect goto cleanup
            printf("[usb_hid] disconnect\n");
            return false;
        }
        return false;
    }
    return true;
}

static void send_report(const HIDReport* r) {
    if (!wait_ready()) return;

    uint32_t now = xTaskGetTickCount();  // ms

    switch (r->kind) {
        case REPORT_ID_KEYBOARD:
            DEBUG_PRINTF("[%lu] KEYBOARD mod=0x%02X kc=%02X %02X %02X %02X %02X %02X\n", now, r->keyboard.modifier,
                         r->keyboard.keycodes[0], r->keyboard.keycodes[1], r->keyboard.keycodes[2],
                         r->keyboard.keycodes[3], r->keyboard.keycodes[4], r->keyboard.keycodes[5]);

            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, r->keyboard.modifier, (uint8_t*)r->keyboard.keycodes);
            break;

        case REPORT_ID_MOUSE:
            DEBUG_PRINTF("[%lu] MOUSE btn=0x%02X x=%d y=%d wheel=%d\n", now, r->mouse.buttons, r->mouse.x, r->mouse.y,
                         r->mouse.wheel);

            tud_hid_mouse_report(REPORT_ID_MOUSE, r->mouse.buttons, r->mouse.x, r->mouse.y, r->mouse.wheel, 0);
            break;

        case REPORT_ID_CONSUMER_CONTROL:
            DEBUG_PRINTF("[%lu] CONSUMER usage=0x%04X\n", now, r->consumer.usage);

            tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &r->consumer.usage, sizeof(r->consumer.usage));
            break;
    }

    wait_complete();
}

void hid_report_set_layout(HIDLayout layout) { keyboard_layout = layout; }
void usb_hid_report_set_char_delay_ms(uint32_t ms) { char_delay_ms = ms; }
void usb_hid_report_set_char_delay_fuzz(uint32_t fuzz_ms) { char_delay_fuzz = fuzz_ms; }

void usb_hid_report_keys(uint8_t modifier, const uint8_t keycodes[6]) {
    HIDReport press = {.kind = REPORT_ID_KEYBOARD, .keyboard = {.modifier = modifier}};

    memcpy(press.keyboard.keycodes, keycodes, 6);

    send_report(&press);
    send_report(&KEYBOARD_RELEASE);
}

void usb_hid_report_char(char c) {
    uint8_t modifier = 0;
    uint8_t keycode = usb_hid_report_char_to_keycode(c, &modifier);
    if (keycode == 0) return;

    HIDReport press = {.kind = REPORT_ID_KEYBOARD,
                       .keyboard = {.modifier = modifier, .keycodes = {keycode, 0, 0, 0, 0, 0}}};

    send_report(&press);
    send_report(&KEYBOARD_RELEASE);
    char_delay();
}

void usb_hid_report_string(const char* str) {
    while (*str) {
        usb_hid_report_char(*str++);
    }
}

void usb_hid_report_mouse_move(int8_t x, int8_t y) {
    HIDReport r = {.kind = REPORT_ID_MOUSE, .mouse = {.buttons = 0, .x = x, .y = y, .wheel = 0}};
    send_report(&r);
}

void usb_hid_report_mouse_click(uint8_t buttons) {
    HIDReport press = {.kind = REPORT_ID_MOUSE, .mouse = {.buttons = buttons, .x = 0, .y = 0, .wheel = 0}};
    send_report(&press);
    send_report(&MOUSE_RELEASE);
}

void usb_hid_report_mouse_scroll(int8_t wheel) {
    HIDReport r = {.kind = REPORT_ID_MOUSE, .mouse = {.buttons = 0, .x = 0, .y = 0, .wheel = wheel}};
    send_report(&r);
}

void usb_hid_report_consumer(uint16_t usage) {
    HIDReport press = {.kind = REPORT_ID_CONSUMER_CONTROL, .consumer = {.usage = usage}};
    send_report(&press);
    send_report(&CONSUMER_RELEASE);
}

uint8_t usb_hid_report_str_to_mod(const char* str) {
    for (size_t i = 0; i < MODIFIER_ARRAY_SIZE; i++) {
        if (strcmp(modifier_conv_table[i].name, str) == 0) {
            return modifier_conv_table[i].modifier_bit;
        }
    }
    return 0;
}

uint8_t usb_hid_report_str_to_special(const char* str) {
    for (size_t i = 0; i < SPECIAL_KEY_ARRAY_SIZE; i++) {
        if (strcmp(special_conv_table[i].name, str) == 0) {
            return special_conv_table[i].keycode;
        }
    }
    return 0;
}

uint16_t usb_hid_report_str_to_consumer(const char* str) {
    for (size_t i = 0; i < CONSUMER_KEY_ARRAY_SIZE; i++) {
        if (strcmp(consumer_conv_table[i].name, str) == 0) {
            return consumer_conv_table[i].keycode;
        }
    }
    return 0;
}

uint8_t usb_hid_report_char_to_keycode(char c, uint8_t* modifier) {
    uint8_t idx = (uint8_t)c;
    if (idx >= 128) return 0;

    const KeyEntry* e = &get_layout_table()[idx];
    if (modifier) *modifier |= e->modifier;
    return e->keycode;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when sent REPORT successfully to host
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void)instance;
    (void)len;

    xSemaphoreGive(complete_semaphore());
}

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer,
                               uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer,
                           uint16_t bufsize) {
    (void)instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD) {
            // bufsize should be (at least) 1
            if (bufsize < 1) return;

            uint8_t const kbd_leds = buffer[0];
            printf("Keyboard LEDs: %s %s %s\n", (kbd_leds & KEYBOARD_LED_CAPSLOCK) ? "CapsLock" : "",
                   (kbd_leds & KEYBOARD_LED_NUMLOCK) ? "NumLock" : "",
                   (kbd_leds & KEYBOARD_LED_SCROLLLOCK) ? "ScrollLock" : "");
        }
    }
}
