#pragma once

#include <stdint.h>

#include "u8g2.h"

#define CANVAS_WIDTH 128
#define CANVAS_HEIGHT 32

typedef struct {
    u8g2_t* u8g2;
} Canvas;

typedef enum {
    CanvasFontPrimary,   /* bold/title */
    CanvasFontSecondary, /* normal/list */
} CanvasFont;

void canvas_init(Canvas* canvas, u8g2_t* u8g2);

/* Frame lifecycle (full-buffer mode): clear -> draw... -> send. */
void canvas_clear(Canvas* canvas);
void canvas_send(Canvas* canvas);

void canvas_set_font(Canvas* canvas, CanvasFont font);
/* 1 = pixels on (default), 0 = pixels off (for inverted text on a filled box) */
void canvas_set_draw_color(Canvas* canvas, uint8_t color);

void canvas_draw_str(Canvas* canvas, uint8_t x, uint8_t y, const char* str);
void canvas_draw_line(Canvas* canvas, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void canvas_draw_box(Canvas* canvas, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void canvas_draw_frame(Canvas* canvas, int x, int y, int w, int h);
void canvas_draw_disc(Canvas* canvas, uint8_t x, uint8_t y, uint8_t r);
uint8_t canvas_string_width(Canvas* canvas, const char* str);
/* Baseline ascent of the current font (canvas_draw_str's y is the baseline). */
uint8_t canvas_font_ascent(Canvas* canvas);
