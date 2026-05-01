#include "ui_common.h"

void ui_draw_header(u8g2_t *u8g2, const char *title) {
    if (!u8g2 || !title) return;

    u8g2_SetFont(u8g2, u8g2_font_profont12_tr);
    u8g2_DrawStr(u8g2, 2, 9, title);
    u8g2_DrawLine(u8g2, 0, 11, 128, 11);
}
