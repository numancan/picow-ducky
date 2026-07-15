#pragma once

#include "hid_layout.h"
#include "tusb.h"

static const KeyEntry ascii_table_tr[128] = {
    {0, 0},                              /* 0x00 NUL */
    {0, 0},                              /* 0x01 */
    {0, 0},                              /* 0x02 */
    {0, 0},                              /* 0x03 */
    {0, 0},                              /* 0x04 */
    {0, 0},                              /* 0x05 */
    {0, 0},                              /* 0x06 */
    {0, 0},                              /* 0x07 */
    {0, 0x2A},                           /* 0x08 BS  -> Backspace */
    {0, 0x2B},                           /* 0x09 TAB */
    {0, 0x28},                           /* 0x0A LF  -> Enter */
    {0, 0},                              /* 0x0B */
    {0, 0},                              /* 0x0C */
    {0, 0x28},                           /* 0x0D CR  -> Enter */
    {0, 0},                              /* 0x0E */
    {0, 0},                              /* 0x0F */
    {0, 0},                              /* 0x10 */
    {0, 0},                              /* 0x11 */
    {0, 0},                              /* 0x12 */
    {0, 0},                              /* 0x13 */
    {0, 0},                              /* 0x14 */
    {0, 0},                              /* 0x15 */
    {0, 0},                              /* 0x16 */
    {0, 0},                              /* 0x17 */
    {0, 0},                              /* 0x18 */
    {0, 0},                              /* 0x19 */
    {0, 0},                              /* 0x1A */
    {0, 0x29},                           /* 0x1B ESC */
    {0, 0},                              /* 0x1C */
    {0, 0},                              /* 0x1D */
    {0, 0},                              /* 0x1E */
    {0, 0},                              /* 0x1F */
    {0, 0x2C},                           /* 0x20 Space */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1E}, /* 0x21 !  -> Shift+1 */
    {0, 0x35},                           /* 0x22 "  -> sol-üst tuş (bare) */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x20},  /* 0x23 #  -> AltGr+3 */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x21},  /* 0x24 $  -> AltGr+4 */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x22}, /* 0x25 %  -> Shift+5 */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x23}, /* 0x26 &  -> Shift+6 */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1F}, /* 0x27 '  -> Shift+2 */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x25}, /* 0x28 (  -> Shift+8 */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x26}, /* 0x29 )  -> Shift+9 */
    {0, 0x2D},                           /* 0x2A *  -> '*' tuşu (0'ın sağı, bare) */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x21}, /* 0x2B +  -> Shift+4 */
    {0, 0x31},                           /* 0x2C ,  -> virgül tuşu (bare) */
    {0, 0x2E},                           /* 0x2D -  -> '-' tuşu (bare) */
    {0, 0x38},                           /* 0x2E .  -> '.' tuşu (US '/' konumu, bare) */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x24}, /* 0x2F /  -> Shift+7 */
    {0, 0x27},                           /* 0x30 0 */
    {0, 0x1E},                           /* 0x31 1 */
    {0, 0x1F},                           /* 0x32 2 */
    {0, 0x20},                           /* 0x33 3 */
    {0, 0x21},                           /* 0x34 4 */
    {0, 0x22},                           /* 0x35 5 */
    {0, 0x23},                           /* 0x36 6 */
    {0, 0x24},                           /* 0x37 7 */
    {0, 0x25},                           /* 0x38 8 */
    {0, 0x26},                           /* 0x39 9 */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x38}, /* 0x3A :  -> Shift+'.' tuşu */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x31}, /* 0x3B ;  -> Shift+virgül tuşu */
    {0, 0x64}, /* 0x3C <  -> ISO ekstra tuş (sol Shift yanı). ANSI'de yoksa: {RIGHTALT,0x35} */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x27}, /* 0x3D =  -> Shift+0 */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x64}, /* 0x3E >  -> Shift+ISO tuş. ANSI'de yoksa: {RIGHTALT,0x1E} */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x2D}, /* 0x3F ?  -> Shift+'*' tuşu */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x14},  /* 0x40 @  -> AltGr+Q */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x04}, /* 0x41 A */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x05}, /* 0x42 B */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x06}, /* 0x43 C */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x07}, /* 0x44 D */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x08}, /* 0x45 E */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x09}, /* 0x46 F */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0A}, /* 0x47 G */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0B}, /* 0x48 H */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0C}, /* 0x49 I  -> Shift+noktasız-ı tuşu (ASCII büyük I) */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0D}, /* 0x4A J */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0E}, /* 0x4B K */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0F}, /* 0x4C L */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x10}, /* 0x4D M */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x11}, /* 0x4E N */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x12}, /* 0x4F O */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x13}, /* 0x50 P */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x14}, /* 0x51 Q */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x15}, /* 0x52 R */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x16}, /* 0x53 S */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x17}, /* 0x54 T */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x18}, /* 0x55 U */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x19}, /* 0x56 V */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1A}, /* 0x57 W */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1B}, /* 0x58 X */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1C}, /* 0x59 Y */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1D}, /* 0x5A Z */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x25},  /* 0x5B [  -> AltGr+8 */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x2D},  /* 0x5C \  -> AltGr+'*' tuşu */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x26},  /* 0x5D ]  -> AltGr+9 */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x20}, /* 0x5E ^  -> ÖLÜ TUŞ: Shift+3, ardından Space gönder */
    {KEYBOARD_MODIFIER_LEFTSHIFT, 0x2E}, /* 0x5F _  -> Shift+'-' tuşu */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x31},  /* 0x60 `  -> ÖLÜ TUŞ: AltGr+virgül tuşu, ardından Space gönder */
    {0, 0x04},                           /* 0x61 a */
    {0, 0x05},                           /* 0x62 b */
    {0, 0x06},                           /* 0x63 c */
    {0, 0x07},                           /* 0x64 d */
    {0, 0x08},                           /* 0x65 e */
    {0, 0x09},                           /* 0x66 f */
    {0, 0x0A},                           /* 0x67 g */
    {0, 0x0B},                           /* 0x68 h */
    {0, 0x34},                           /* 0x69 i  -> noktalı i tuşu (OEM_7). DİKKAT: 0x0C DEĞİL */
    {0, 0x0D},                           /* 0x6A j */
    {0, 0x0E},                           /* 0x6B k */
    {0, 0x0F},                           /* 0x6C l */
    {0, 0x10},                           /* 0x6D m */
    {0, 0x11},                           /* 0x6E n */
    {0, 0x12},                           /* 0x6F o */
    {0, 0x13},                           /* 0x70 p */
    {0, 0x14},                           /* 0x71 q */
    {0, 0x15},                           /* 0x72 r */
    {0, 0x16},                           /* 0x73 s */
    {0, 0x17},                           /* 0x74 t */
    {0, 0x18},                           /* 0x75 u */
    {0, 0x19},                           /* 0x76 v */
    {0, 0x1A},                           /* 0x77 w */
    {0, 0x1B},                           /* 0x78 x */
    {0, 0x1C},                           /* 0x79 y */
    {0, 0x1D},                           /* 0x7A z */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x24},  /* 0x7B {  -> AltGr+7 */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x2E},  /* 0x7C |  -> AltGr+'-' tuşu */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x27},  /* 0x7D }  -> AltGr+0 */
    {KEYBOARD_MODIFIER_RIGHTALT, 0x30},  /* 0x7E ~  -> ÖLÜ TUŞ: AltGr+ü tuşu, ardından Space gönder */
    {0, 0x4C}                            /* 0x7F DEL -> Delete */
};