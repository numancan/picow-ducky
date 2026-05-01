#pragma once

#include "u8g2.h"

// Common UI constants for 128x32 OLED
#define UI_HEADER_HEIGHT 12
#define UI_LINE_HEIGHT 10

// Draws a standard header with a title and a separator line
void ui_draw_header(u8g2_t *u8g2, const char *title);
