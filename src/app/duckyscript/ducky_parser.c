#include "ducky_parser.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ducky_config.h"

bool ducky_parse_line(char* line, DuckyLine* parsed_line) {
    if (!line || !parsed_line) return false;

    // Terminate line before new line (\r or \n)
    line[strcspn(line, "\r\n")] = '\0';

    if (strlen(line) > DUCKY_MAX_LINE_LEN) return false;

    memset(parsed_line, 0, sizeof(DuckyLine));

    char* saveptr;
    char* pTemp = strtok_r(line, " ", &saveptr);
    if (pTemp) {
        snprintf(parsed_line->command, sizeof(parsed_line->command), "%s", pTemp);
    } else {
        return false;
    }

    pTemp = strtok_r(NULL, "", &saveptr);
    if (pTemp) {
        snprintf(parsed_line->args, sizeof(parsed_line->args), "%s", pTemp);
    }

    return true;
}