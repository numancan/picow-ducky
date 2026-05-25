#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MAX_CHAR_PER_LINE 256

typedef struct {
    char command[24];
    char args[MAX_CHAR_PER_LINE];
} ducky_line_t;

bool ducky_parser_parse_line(char* line, ducky_line_t* parsed_line);
