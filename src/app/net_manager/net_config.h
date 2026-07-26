#pragma once

#include <stdbool.h>
#include <stddef.h>

/* Persistent Wi-Fi credential store: a single (ssid, pass) pair kept as a
 * config file on the SD card (via config_io/fat_io). All functions block on
 * file access and must be called from task context only (they are used
 * solely by net_task). */

/* Copies the stored credentials into the given buffers.
 * Returns false if no valid record exists or a field does not fit. */
bool net_config_load_wifi(char* ssid, size_t ssid_size, char* pass, size_t pass_size);

/* Overwrites the stored credentials. Truncates fields that exceed the
 * 802.11 limits (32-char SSID, 63-char passphrase). */
void net_config_save_wifi(const char* ssid, const char* pass);

/* Invalidates the stored credentials. */
void net_config_erase_wifi(void);
