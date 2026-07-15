#include "elements.h"

#include "middleware/sys_fault.h"

#define ELEMENTS_LEFT_PADDING 2
#define ELEMENTS_LIST_LABEL_X 10  // row label x

void elements_draw_header(Canvas* canvas, const char* title) {
    ABORT_IF(canvas == NULL || title == NULL);

    canvas_set_font(canvas, CanvasFontPrimary);
    canvas_draw_str(canvas, ELEMENTS_LEFT_PADDING, canvas_font_ascent(canvas), title);
    canvas_draw_line(canvas, 0, ELEMENTS_HEADER_HEIGHT - 2, CANVAS_WIDTH, ELEMENTS_HEADER_HEIGHT - 2);
}

uint8_t elements_list_visible_rows(uint8_t height) { return (uint8_t)(height / ELEMENTS_LINE_HEIGHT); }

void elements_draw_list_row(Canvas* canvas, uint8_t y_top, uint16_t row, bool selected, const char* label) {
    ABORT_IF(canvas == NULL || label == NULL);

    canvas_set_font(canvas, CanvasFontSecondary);

    uint8_t y = (uint8_t)(y_top + row * ELEMENTS_LINE_HEIGHT);
    if (selected) {
        canvas_draw_str(canvas, ELEMENTS_LEFT_PADDING, y + canvas_font_ascent(canvas), ">");
    }
    canvas_draw_str(canvas, ELEMENTS_LIST_LABEL_X, y + canvas_font_ascent(canvas), label);
}

void elements_draw_list(Canvas* canvas, uint8_t y_top, uint8_t height, uint16_t selected, uint16_t count,
                        ElementsItemLabelCallback get_label, void* context) {
    ABORT_IF(canvas == NULL || get_label == NULL);

    if (count == 0) return;

    uint16_t visible = elements_list_visible_rows(height);
    uint16_t window = (selected < visible) ? 0 : (selected - visible + 1);

    for (uint16_t row = 0; row < visible; row++) {
        uint16_t idx = window + row;
        if (idx >= count) break;

        elements_draw_list_row(canvas, y_top, row, idx == selected, get_label(context, idx));
    }

    elements_scrollbar(canvas, y_top, window, visible, count);
}

#define ELEMENTS_SCROLLBAR_WIDTH 2
#define ELEMENTS_SCROLLBAR_MIN_THUMB 2

void elements_scrollbar(Canvas* canvas, uint8_t y_top, uint16_t pos, uint16_t visible, uint16_t total) {
    ABORT_IF(canvas == NULL);

    if (total <= visible) return;

    const uint8_t track_h = (uint8_t)(visible * ELEMENTS_LINE_HEIGHT);

    uint8_t thumb_h = (uint8_t)((uint32_t)visible * track_h / total);
    if (thumb_h < ELEMENTS_SCROLLBAR_MIN_THUMB) thumb_h = ELEMENTS_SCROLLBAR_MIN_THUMB;

    uint8_t thumb_y = (uint8_t)(y_top + (uint32_t)pos * track_h / total);

    canvas_draw_box(canvas, CANVAS_WIDTH - ELEMENTS_SCROLLBAR_WIDTH, thumb_y, ELEMENTS_SCROLLBAR_WIDTH, thumb_h);
}

void elements_draw_str_right(Canvas* canvas, uint8_t y, const char* str) {
    uint8_t w = canvas_string_width(canvas, str);
    uint8_t right = CANVAS_WIDTH - ELEMENTS_SCROLLBAR_WIDTH - 1;
    uint8_t x = (w >= right) ? 0 : (uint8_t)(right - w);
    canvas_draw_str(canvas, x, y + canvas_font_ascent(canvas), str);
}
