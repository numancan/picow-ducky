#pragma once

#include "ffconf.h"

#define DUCKY_CONFIG_DIR "/apps_data"
#define DUCKY_CONFIG_FNAME "ducky.cfg"
#define DUCKY_CONFIG_PATH DUCKY_CONFIG_DIR "/" DUCKY_CONFIG_FNAME

// Staging path for a settings body streamed in over HTTP before it is parsed and applied.
// Distinct from config_kv_save_atomic's "<path>.tmp" to avoid collisions.
#define DUCKY_CONFIG_RX_PATH DUCKY_CONFIG_DIR "/ducky.rx"
#define DUCKY_FILETYPE "Ducky Settings"
#define DUCKY_VERSION 1
#define DUCKY_PAYLOAD_DIR "/payloads"
#define DUCKY_FILE_EXT ".txt"

// Web "live" editor scratch file: the streamed script body is written here, then played once.
// Kept in /payloads but with a .tmp extension so the .txt-filtered payload listing skips it.
#define DUCKY_LIVE_FNAME "live.tmp"
#define DUCKY_LIVE_PATH DUCKY_PAYLOAD_DIR "/" DUCKY_LIVE_FNAME
#define DUCKY_MAX_PAYLOAD_FNAME_LEN FF_MAX_LFN
#define DUCKY_MAX_PAYLOAD_PATH_LEN (sizeof(DUCKY_PAYLOAD_DIR) + DUCKY_MAX_PAYLOAD_FNAME_LEN)
#define DUCKY_MAX_COMMAND_LEN 24
#define DUCKY_MAX_LINE_LEN 256

#define DUCKY_DELAY_MIN 0u
#define DUCKY_DELAY_MAX 2000u
#define DUCKY_DELAY_STEP 100u
#define DUCKY_DELAY_COUNT ((DUCKY_DELAY_MAX - DUCKY_DELAY_MIN) / DUCKY_DELAY_STEP + 1u)

// Auto-disarm the transport after this long armed with no playback.
#define DUCKY_ARM_TIMEOUT_MS (3u * 60u * 1000u)  // 3 minutes