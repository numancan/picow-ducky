#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ducky_config.h"  // DUCKY_MAX_PAYLOAD_FNAME_LEN

#define PAYLOAD_CACHE_SIZE 16

typedef struct {
    char names[PAYLOAD_CACHE_SIZE][DUCKY_MAX_PAYLOAD_FNAME_LEN];
    uint32_t total_count;   // total payloads in /payloads
    uint32_t window_start;  // absolute index of names[0] in the full list
    uint32_t cached_count;  // valid entries in names[] (<= PAYLOAD_CACHE_SIZE)
} PayloadList;

// Count the payloads in /payloads and load the first window into the cache.
void payload_list_init(PayloadList* list);

// Total number of payloads.
uint32_t payload_list_count(const PayloadList* list);

// Name at the given absolute index, paging the cache window as needed. NULL if out of range.
const char* payload_list_get(PayloadList* list, uint32_t index);

// Load the cache window starting at window_start. Returns false on I/O error or bad start.
bool payload_list_load_window(PayloadList* list, uint32_t window_start);
