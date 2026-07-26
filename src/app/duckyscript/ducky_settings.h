#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ducky_config.h"
#include "middleware/config_kv.h"
#include "middleware/enum_gen.h"

// HID headers provide the enum counts and name functions used by the value policy.
#include "hid/hid_transport.h"
#include "hid/layouts/hid_layout.h"

typedef struct DuckySettings {
    uint32_t char_delay;
    uint32_t char_fuzz_delay;
    uint32_t delay;
    uint32_t fuzz_delay;
    uint8_t kb_layout;
    uint8_t transporter;
    bool transporter_always_on;
} DuckySettings;

// $EXPORT=INDEX,KEY,DISPLAY_NAME,MAX_VALUE,TYPE
#define DUCKY_SETTINGS_LIST(X)                                                                  \
    X(DUCKY_SETTINGS_ITEM_CHAR_DELAY, "char_delay", "Char Delay", DUCKY_DELAY_MAX, int)         \
    X(DUCKY_SETTINGS_ITEM_CHAR_FUZZ, "char_fuzz_delay", "Char Fuzz", DUCKY_DELAY_MAX, int)      \
    X(DUCKY_SETTINGS_ITEM_DELAY, "delay", "Delay", DUCKY_DELAY_MAX, int)                        \
    X(DUCKY_SETTINGS_ITEM_FUZZ_DELAY, "fuzz_delay", "Fuzz Delay", DUCKY_DELAY_MAX, int)         \
    X(DUCKY_SETTINGS_ITEM_KB_LAYOUT, "kb_layout", "KB Layout", HID_KEY_LAYOUT_COUNT, select)    \
    X(DUCKY_SETTINGS_ITEM_TRANSPORTER, "transporter", "Transport", HID_TRANSPORT_COUNT, select) \
    X(DUCKY_SETTINGS_ITEM_TRANSPORTER_AO, "transporter_always_on", "Always On", 2, bool)

DECLARE_ENUM(DuckySettingsItem, DUCKY_SETTINGS_ITEM_COUNT, DUCKY_SETTINGS_LIST)

// Load settings from SD into *out, falling back to defaults when unavailable.
bool ducky_settings_load(DuckySettings* out);

// Atomically persist *in to SD. Returns false on I/O failure.
bool ducky_settings_save(const DuckySettings* in);

// Parse a config file at `path`, applying recognized keys onto *io. The caller pre-seeds
// *io (e.g. from the current settings) so keys absent from the file keep their value.
// Parsed values are clamped to their valid range.
bool ducky_settings_parse_file(const char* path, DuckySettings* io);

// Display label for a settings item.
const char* ducky_settings_label(DuckySettingsItem item);

// Advance the item's value to the next step (wraps around).
void ducky_settings_next(DuckySettings* s, DuckySettingsItem item);

// Format the item's value into out as "<...>" (e.g. "<2000>", "<US-Q>", "<On>").
void ducky_settings_format(const DuckySettings* s, DuckySettingsItem item, char* out, size_t n);