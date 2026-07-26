#pragma once

#include <stdbool.h>

#include "ducky_parser.h"
#include "ducky_settings.h"
#include "middleware/enum_gen.h"

// $EXPORT=ID,Command_Name
#define DUCKY_COMMAND_LIST(X)                       \
    X(REM, "REM")                                   \
    X(REPEAT, "REPEAT")                             \
    X(DELAY, "DELAY")                               \
    X(DEFAULTDELAY, "DEFAULTDELAY")                 \
    X(DEFAULTDELAYFUZZ, "DEFAULTDELAYFUZZ")         \
    X(DEFAULTCHARDELAY, "DEFAULTCHARDELAY")         \
    X(DEFAULTCHARDELAYFUZZ, "DEFAULTCHARDELAYFUZZ") \
    X(STRING, "STRING")                             \
    X(LAYOUT, "LAYOUT")                             \
    X(MOUSE_MOVE, "MOUSE_MOVE")                     \
    X(MOUSE_CLICK, "MOUSE_CLICK")                   \
    X(MOUSE_SCROLL, "MOUSE_SCROLL")

DECLARE_ENUM(DuckyCommandID, DUCKY_COMMAND_COUNT, DUCKY_COMMAND_LIST)

// Load the default delays and keyboard layout from settings before playback.
void ducky_handler_init(const DuckySettings* settings);

// Execute one parsed duckyscript line. Returns false when a HID send hit a
// terminal transport state and the payload should stop.
bool ducky_handler_exec_line(DuckyLine* ducky_line);
