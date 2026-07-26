#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ducky_config.h"

typedef struct {
    char command[DUCKY_MAX_COMMAND_LEN];
    char args[DUCKY_MAX_LINE_LEN];
} DuckyLine;

// Split a raw line into command + args in place. Returns false on an empty or
// oversized line.
bool ducky_parse_line(char* line, DuckyLine* parsed_line);
