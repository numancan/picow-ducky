#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fat_io.h"

#define CONFIG_KV_LINE_MAX 96

// Called for every parsed "key: value" line.
typedef void (*ConfigKvLineFn)(const char* key, const char* val, void* ctx);

// Writes the file body via config_kv_write_* calls on f; returns false to abort the save.
typedef bool (*ConfigKvWriteFn)(FIL* f, void* ctx);

// Reads path line by line, invoking fn(key, val, ctx) for each "key: value" line found.
bool config_kv_parse(const char* path, ConfigKvLineFn fn, void* ctx);

// Writes the standard "Filetype:"/"Version:" header lines.
bool config_kv_write_header(FIL* f, const char* filetype, uint32_t version);

// Writes a single "key: value" line for the given type.
bool config_kv_write_u32(FIL* f, const char* key, uint32_t value);
bool config_kv_write_i32(FIL* f, const char* key, int32_t value);
bool config_kv_write_bool(FIL* f, const char* key, bool value);
bool config_kv_write_str(FIL* f, const char* key, const char* value);

// Writes path via a temp file + rename so a failed or interrupted write never
// leaves a corrupt file at path.
bool config_kv_save_atomic(const char* path, ConfigKvWriteFn writer, void* ctx);