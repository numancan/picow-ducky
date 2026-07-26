#include "canvas.h"

#include "middleware/sys_fault.h"

void canvas_init(Canvas* canvas, u8g2_t* u8g2) {
    ABORT_IF(canvas == NULL || u8g2 == NULL);

    canvas->u8g2 = u8g2;
    u8g2_SetDrawColor(u8g2, 1);
}

void canvas_clear(Canvas* canvas) {
    ABORT_IF(!canvas);
    u8g2_ClearBuffer(canvas->u8g2);
}

void canvas_send(Canvas* canvas) {
    ABORT_IF(!canvas);
    u8g2_SendBuffer(canvas->u8g2);
}

void canvas_set_font(Canvas* canvas, CanvasFont font) {
    ABORT_IF(!canvas);
    const uint8_t* u8g2_font = (font == CanvasFontPrimary) ? u8g2_font_profont12_tr : u8g2_font_profont11_tr;
    u8g2_SetFont(canvas->u8g2, u8g2_font);
}

void canvas_draw_str(Canvas* canvas, uint8_t x, uint8_t y, const char* str) {
    ABORT_IF(!canvas);
    u8g2_DrawStr(canvas->u8g2, x, y, str);
}

void canvas_draw_line(Canvas* canvas, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    ABORT_IF(!canvas);
    u8g2_DrawLine(canvas->u8g2, x0, y0, x1, y1);
}

void canvas_draw_box(Canvas* canvas, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    ABORT_IF(!canvas);
    u8g2_DrawBox(canvas->u8g2, x, y, w, h);
}

void canvas_draw_disc(Canvas* canvas, uint8_t x, uint8_t y, uint8_t r) {
    ABORT_IF(!canvas);
    u8g2_DrawDisc(canvas->u8g2, x, y, r, U8G2_DRAW_ALL);
}

void canvas_set_draw_color(Canvas* canvas, uint8_t color) {
    ABORT_IF(!canvas);
    u8g2_SetDrawColor(canvas->u8g2, color);
}

void canvas_draw_frame(Canvas* canvas, int x, int y, int w, int h) {
    ABORT_IF(!canvas);
    u8g2_DrawFrame(canvas->u8g2, x, y, w, h);
}

uint8_t canvas_string_width(Canvas* canvas, const char* str) {
    ABORT_IF(!canvas);
    return (uint8_t)u8g2_GetStrWidth(canvas->u8g2, str);
}

uint8_t canvas_font_ascent(Canvas* canvas) {
    ABORT_IF(!canvas);
    return (uint8_t)u8g2_GetAscent(canvas->u8g2);
}