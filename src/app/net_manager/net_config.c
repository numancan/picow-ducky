#include "net_config.h"

#include <string.h>

#include "config_kv.h"
#include "fat_io.h"
#include "middleware/log.h"
#include "middleware/sys_fault.h"

/* 802.11 limits (content length, excluding the null terminator):
 * 32 is the SSID limit; 63 is the WPA/WPA2 passphrase limit. */
#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 63

#define WIFI_CONFIG_DIR "/apps_data"
#define WIFI_CONFIG_FNAME "wifi.cfg"
#define WIFI_CONFIG_PATH WIFI_CONFIG_DIR "/" WIFI_CONFIG_FNAME
#define WIFI_FILETYPE "WiFi Config"
#define WIFI_VERSION 1

#define KEY_SSID "SSID"
#define KEY_PASS "Password"

static const char* TAG = "wifi_config";

typedef struct {
    char ssid[MAX_SSID_LEN + 1];
    char pass[MAX_PASSWORD_LEN + 1];
} WifiConfig;

/* Load context: buffers are zero-initialised so a bounded strncpy always
 * leaves them terminated. `valid` latches to false on any over-length field
 * (file-derived data cannot be trusted to fit). */
typedef struct {
    WifiConfig cfg;
    bool has_ssid;
    bool valid;
} WifiLoadCtx;

static void wifi_load_cb(const char* key, const char* val, void* ctx) {
    WifiLoadCtx* c = (WifiLoadCtx*)ctx;

    if (strcmp(key, KEY_SSID) == 0) {
        if (strlen(val) > MAX_SSID_LEN) {
            c->valid = false;
            return;
        }
        strncpy(c->cfg.ssid, val, sizeof c->cfg.ssid - 1);
        c->has_ssid = true;
    } else if (strcmp(key, KEY_PASS) == 0) {
        if (strlen(val) > MAX_PASSWORD_LEN) {
            c->valid = false;
            return;
        }
        strncpy(c->cfg.pass, val, sizeof c->cfg.pass - 1);
    }
}

static bool wifi_write(FIL* f, void* ctx) {
    const WifiConfig* cfg = (const WifiConfig*)ctx;
    bool ok = true;
    ok &= config_kv_write_header(f, WIFI_FILETYPE, WIFI_VERSION);
    ok &= config_kv_write_str(f, KEY_SSID, cfg->ssid);
    ok &= config_kv_write_str(f, KEY_PASS, cfg->pass);
    return ok;
}

bool net_config_load_wifi(char* ssid, size_t ssid_size, char* pass, size_t pass_size) {
    ABORT_IF(ssid == NULL || pass == NULL);

    if (fat_io_check_file_exist(WIFI_CONFIG_PATH) != FAT_IO_OK) return false;

    WifiLoadCtx c = {.valid = true};
    if (!config_kv_parse(WIFI_CONFIG_PATH, wifi_load_cb, &c)) return false;
    if (!c.valid || !c.has_ssid || c.cfg.ssid[0] == '\0') return false;

    size_t ssid_len = strlen(c.cfg.ssid);
    size_t pass_len = strlen(c.cfg.pass);
    if (ssid_len >= ssid_size || pass_len >= pass_size) return false;

    memcpy(ssid, c.cfg.ssid, ssid_len + 1);
    memcpy(pass, c.cfg.pass, pass_len + 1);
    return true;
}

void net_config_save_wifi(const char* ssid, const char* pass) {
    ABORT_IF(ssid == NULL || ssid[0] == '\0');

    /* Zero-init leaves field tails terminated after the bounded copies. */
    WifiConfig cfg = {0};
    strncpy(cfg.ssid, ssid, MAX_SSID_LEN);
    strncpy(cfg.pass, pass ? pass : "", MAX_PASSWORD_LEN);

    /* Ignore an EXIST result: we only need the directory to be present. */
    fat_io_mkdir(WIFI_CONFIG_DIR);

    if (!config_kv_save_atomic(WIFI_CONFIG_PATH, wifi_write, &cfg)) {
        LOG_ERROR(TAG, "failed to save wifi config");
    }
}

void net_config_erase_wifi(void) {
    FatIoResult r = fat_io_remove_file(WIFI_CONFIG_PATH);
    if (r != FAT_IO_OK && r != FAT_IO_NO_FILE && r != FAT_IO_NO_PATH) {
        LOG_WARN(TAG, "failed to erase wifi config");
    }
}
