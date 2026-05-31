#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ducky_config.h"

typedef struct {
    char command[DUCKY_MAX_COMMAND_LEN];
    char args[DUCKY_MAX_LINE_LEN];
} ducky_line_t;

bool ducky_parser_parse_line(char* line, ducky_line_t* parsed_line);
