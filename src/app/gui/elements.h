#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "canvas.h"

// header band height (matches the primary font height)
#define ELEMENTS_HEADER_HEIGHT 12
// list row pitch
#define ELEMENTS_LINE_HEIGHT 10

typedef const char* (*ElementsItemLabelCallback)(void* context, uint16_t index);

// Number of list rows that fit in a content area of the given height.
uint8_t elements_list_visible_rows(uint8_t height);

// Title bar with a separator line under it.
void elements_draw_header(Canvas* canvas, const char* title);

// Draws a generic list with scrolling, a cursor, and a scrollbar within the
// content area [y_top, y_top + height).
void elements_draw_list(Canvas* canvas, uint8_t y_top, uint8_t height, uint16_t selected, uint16_t count,
                        ElementsItemLabelCallback get_label, void* context);

// Draws a single list row (0-based, measured from y_top): selection bullet + label.
void elements_draw_list_row(Canvas* canvas, uint8_t y_top, uint16_t row, bool selected, const char* label);

// Vertical scrollbar on the right edge of the content area starting at y_top.
// pos = top visible row, visible = rows on screen, total = total row count.
void elements_scrollbar(Canvas* canvas, uint8_t y_top, uint16_t pos, uint16_t visible, uint16_t total);
// Right-aligned string drawn on the given list row (0-based, measured from y_top).
void elements_draw_str_right(Canvas* canvas, uint8_t y, const char* str);