#include "ducky_settings.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_kv.h"
#include "ducky_config.h"
#include "fat_io.h"
#include "middleware/log.h"

static const DuckySettings DUCKY_DEFAULT = {
    .char_delay = 0,
    .char_fuzz_delay = 0,
    .delay = 0,
    .fuzz_delay = 0,
    .kb_layout = 0,
    .transporter = 0,
    .transporter_always_on = true,
};

static const char* setting_key(DuckySettingsItem item) {
    switch (item) {
#define X(index, key, ...) \
    case index: return key;
        DUCKY_SETTINGS_LIST(X)
#undef X
        default: return NULL;
    }
}

const char* ducky_settings_label(DuckySettingsItem item) {
    switch (item) {
#define X(index, key, display_name, ...) \
    case index: return display_name;
        DUCKY_SETTINGS_LIST(X)
#undef X
        default: return "Unknown";
    }
}

static uint32_t delay_next(uint32_t v) {
    uint32_t idx = v / DUCKY_DELAY_STEP;
    idx = (idx + 1u) % DUCKY_DELAY_COUNT;
    return DUCKY_DELAY_MIN + idx * DUCKY_DELAY_STEP;
}

// Snap a raw (e.g. hand-edited) value onto the valid range and step grid.
static uint32_t delay_clamp(uint32_t v) {
    if (v <= DUCKY_DELAY_MIN) return DUCKY_DELAY_MIN;
    if (v >= DUCKY_DELAY_MAX) return DUCKY_DELAY_MAX;
    uint32_t idx = (v - DUCKY_DELAY_MIN + DUCKY_DELAY_STEP / 2u) / DUCKY_DELAY_STEP;  // round to nearest grid step
    return DUCKY_DELAY_MIN + idx * DUCKY_DELAY_STEP;
}

void ducky_settings_next(DuckySettings* s, DuckySettingsItem item) {
    switch (item) {
        case DUCKY_SETTINGS_ITEM_CHAR_DELAY: s->char_delay = delay_next(s->char_delay); break;
        case DUCKY_SETTINGS_ITEM_CHAR_FUZZ: s->char_fuzz_delay = delay_next(s->char_fuzz_delay); break;
        case DUCKY_SETTINGS_ITEM_DELAY: s->delay = delay_next(s->delay); break;
        case DUCKY_SETTINGS_ITEM_FUZZ_DELAY: s->fuzz_delay = delay_next(s->fuzz_delay); break;
        case DUCKY_SETTINGS_ITEM_KB_LAYOUT: s->kb_layout = (uint8_t)((s->kb_layout + 1u) % HID_KEY_LAYOUT_COUNT); break;
        case DUCKY_SETTINGS_ITEM_TRANSPORTER:
            s->transporter = (uint8_t)((s->transporter + 1u) % HID_TRANSPORT_COUNT);
            break;
        case DUCKY_SETTINGS_ITEM_TRANSPORTER_AO: s->transporter_always_on = !s->transporter_always_on; break;
        default: break;
    }
}

void ducky_settings_format(const DuckySettings* s, DuckySettingsItem item, char* out, size_t n) {
    switch (item) {
        case DUCKY_SETTINGS_ITEM_CHAR_DELAY: snprintf(out, n, "<%lu>", (unsigned long)s->char_delay); break;
        case DUCKY_SETTINGS_ITEM_CHAR_FUZZ: snprintf(out, n, "<%lu>", (unsigned long)s->char_fuzz_delay); break;
        case DUCKY_SETTINGS_ITEM_DELAY: snprintf(out, n, "<%lu>", (unsigned long)s->delay); break;
        case DUCKY_SETTINGS_ITEM_FUZZ_DELAY: snprintf(out, n, "<%lu>", (unsigned long)s->fuzz_delay); break;
        case DUCKY_SETTINGS_ITEM_KB_LAYOUT: {
            const char* nm = hid_layout_name((HIDLayoutID)s->kb_layout);
            snprintf(out, n, "<%s>", nm ? nm : "?");
            break;
        }
        case DUCKY_SETTINGS_ITEM_TRANSPORTER: {
            const char* nm = hid_transport_name(s->transporter);
            snprintf(out, n, "<%s>", nm ? nm : "?");
            break;
        }
        case DUCKY_SETTINGS_ITEM_TRANSPORTER_AO:
            snprintf(out, n, "<%s>", s->transporter_always_on ? "On" : "Off");
            break;
        default: snprintf(out, n, "<?>"); break;
    }
}

// Maps a parsed config key onto its struct field.
static void ducky_load_cb(const char* key, const char* val, void* ctx) {
    DuckySettings* s = (DuckySettings*)ctx;

    if (strcmp(key, setting_key(DUCKY_SETTINGS_ITEM_CHAR_DELAY)) == 0) {
        s->char_delay = (uint32_t)strtoul(val, NULL, 10);
        s->char_delay = delay_clamp(s->char_delay);
    } else if (strcmp(key, setting_key(DUCKY_SETTINGS_ITEM_CHAR_FUZZ)) == 0) {
        s->char_fuzz_delay = (uint32_t)strtoul(val, NULL, 10);
        s->char_fuzz_delay = delay_clamp(s->char_fuzz_delay);
    } else if (strcmp(key, setting_key(DUCKY_SETTINGS_ITEM_DELAY)) == 0) {
        s->delay = (uint32_t)strtoul(val, NULL, 10);
        s->delay = delay_clamp(s->delay);
    } else if (strcmp(key, setting_key(DUCKY_SETTINGS_ITEM_FUZZ_DELAY)) == 0) {
        s->fuzz_delay = (uint32_t)strtoul(val, NULL, 10);
        s->fuzz_delay = delay_clamp(s->fuzz_delay);
    } else if (strcmp(key, setting_key(DUCKY_SETTINGS_ITEM_KB_LAYOUT)) == 0) {
        // Out-of-range value: keep the pre-seeded value instead of overwriting it.
        uint32_t v = strtoul(val, NULL, 10);
        if (v < HID_KEY_LAYOUT_COUNT) s->kb_layout = (uint8_t)v;
    } else if (strcmp(key, setting_key(DUCKY_SETTINGS_ITEM_TRANSPORTER)) == 0) {
        uint32_t v = strtoul(val, NULL, 10);
        if (v < HID_TRANSPORT_COUNT) s->transporter = (uint8_t)v;
    } else if (strcmp(key, setting_key(DUCKY_SETTINGS_ITEM_TRANSPORTER_AO)) == 0) {
        s->transporter_always_on = (strtoul(val, NULL, 10) != 0);
    }
}

static bool ducky_write(FIL* f, void* ctx) {
    const DuckySettings* s = (const DuckySettings*)ctx;
    bool ok = true;
    ok &= config_kv_write_header(f, DUCKY_FILETYPE, DUCKY_VERSION);
    ok &= config_kv_write_u32(f, setting_key(DUCKY_SETTINGS_ITEM_CHAR_DELAY), s->char_delay);
    ok &= config_kv_write_u32(f, setting_key(DUCKY_SETTINGS_ITEM_CHAR_FUZZ), s->char_fuzz_delay);
    ok &= config_kv_write_u32(f, setting_key(DUCKY_SETTINGS_ITEM_DELAY), s->delay);
    ok &= config_kv_write_u32(f, setting_key(DUCKY_SETTINGS_ITEM_FUZZ_DELAY), s->fuzz_delay);
    ok &= config_kv_write_u32(f, setting_key(DUCKY_SETTINGS_ITEM_KB_LAYOUT), s->kb_layout);
    ok &= config_kv_write_u32(f, setting_key(DUCKY_SETTINGS_ITEM_TRANSPORTER), s->transporter);
    ok &= config_kv_write_bool(f, setting_key(DUCKY_SETTINGS_ITEM_TRANSPORTER_AO), s->transporter_always_on);
    return ok;
}

bool ducky_settings_load(DuckySettings* out) {
    *out = DUCKY_DEFAULT;

    if (!fat_io_is_mounted()) {
        LOG_ERROR("DUCKY_SETTINGS", "FATFS not mounted!");
        // default is loaded from the begining
        return true;
    }

    FatIoResult res = fat_io_check_file_exist(DUCKY_CONFIG_PATH);

    if (res == FAT_IO_NO_PATH) {
        fat_io_mkdir(DUCKY_CONFIG_DIR);
    }
    if (res == FAT_IO_NO_FILE || res == FAT_IO_NO_PATH) {
        return ducky_settings_save(out);
    }

    if (!config_kv_parse(DUCKY_CONFIG_PATH, ducky_load_cb, out)) {
        // Corrupt/unreadable file: fall back to defaults per the header contract.
        *out = DUCKY_DEFAULT;
        return false;
    }
    return true;
}

bool ducky_settings_save(const DuckySettings* in) {
    // Cast away const to fit the atomic-save callback signature; ducky_write does not mutate it.
    return config_kv_save_atomic(DUCKY_CONFIG_PATH, ducky_write, (void*)in);
}

bool ducky_settings_parse_file(const char* path, DuckySettings* io) { return config_kv_parse(path, ducky_load_cb, io); }