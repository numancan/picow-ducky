#pragma once

#define HID_LAYOUT_US_Q                                                 \
    {0, 0},                                  /* 0x00 NUL   */           \
        {0, 0},                              /* 0x01 SOH   */           \
        {0, 0},                              /* 0x02 STX   */           \
        {0, 0},                              /* 0x03 ETX   */           \
        {0, 0},                              /* 0x04 EOT   */           \
        {0, 0},                              /* 0x05 ENQ   */           \
        {0, 0},                              /* 0x06 ACK   */           \
        {0, 0},                              /* 0x07 BEL   */           \
        {0, 0x2A},                           /* 0x08 BS  → Backspace */ \
        {0, 0x2B},                           /* 0x09 HT  → Tab */       \
        {0, 0x28},                           /* 0x0A LF  → Enter*/      \
        {0, 0},                              /* 0x0B VT */              \
        {0, 0},                              /* 0x0C FF */              \
        {0, 0},                              /* 0x0D CR*/               \
        {0, 0},                              /* 0x0E SO*/               \
        {0, 0},                              /* 0x0F SI*/               \
        {0, 0},                              /* 0x10 DLE*/              \
        {0, 0},                              /* 0x11 DC1*/              \
        {0, 0},                              /* 0x12 DC2*/              \
        {0, 0},                              /* 0x13 DC3*/              \
        {0, 0},                              /* 0x14 DC4*/              \
        {0, 0},                              /* 0x15 NAK*/              \
        {0, 0},                              /* 0x16 SYN*/              \
        {0, 0},                              /* 0x17 ETB*/              \
        {0, 0},                              /* 0x18 CAN*/              \
        {0, 0},                              /* 0x19 EM*/               \
        {0, 0},                              /* 0x1A SUB*/              \
        {0, 0x29},                           /* 0x1B ESC → Escape*/     \
        {0, 0},                              /* 0x1C FS*/               \
        {0, 0},                              /* 0x1D GS*/               \
        {0, 0},                              /* 0x1E RS*/               \
        {0, 0},                              /* 0x1F US*/               \
        {0, 0x2C},                           /* 0x20 */                 \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1E}, /* 0x21 !*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x34}, /* 0x22 "*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x20}, /* 0x23 #*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x21}, /* 0x24 $*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x22}, /* 0x25 %*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x24}, /* 0x26 &*/                \
        {0, 0x34},                           /* 0x27 '*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x26}, /* 0x28 (*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x27}, /* 0x29 )*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x25}, /* 0x2A **/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x2E}, /* 0x2B +*/                \
        {0, 0x36},                           /* 0x2C ,*/                \
        {0, 0x2D},                           /* 0x2D -*/                \
        {0, 0x37},                           /* 0x2E .*/                \
        {0, 0x38},                           /* 0x2F /*/                \
        {0, 0x27},                           /* 0x30 0*/                \
        {0, 0x1E},                           /* 0x31 1*/                \
        {0, 0x1F},                           /* 0x32 2*/                \
        {0, 0x20},                           /* 0x33 3*/                \
        {0, 0x21},                           /* 0x34 4*/                \
        {0, 0x22},                           /* 0x35 5*/                \
        {0, 0x23},                           /* 0x36 6*/                \
        {0, 0x24},                           /* 0x37 7*/                \
        {0, 0x25},                           /* 0x38 8*/                \
        {0, 0x26},                           /* 0x39 9*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x33}, /* 0x3A :*/                \
        {0, 0x33},                           /* 0x3B ;*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x36}, /* 0x3C <*/                \
        {0, 0x2E},                           /* 0x3D =*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x37}, /* 0x3E >*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x38}, /* 0x3F ?*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1F}, /* 0x40 @*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x04}, /* 0x41 A*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x05}, /* 0x42 B*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x06}, /* 0x43 C*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x07}, /* 0x44 D*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x08}, /* 0x45 E*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x09}, /* 0x46 F*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0A}, /* 0x47 G*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0B}, /* 0x48 H*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0C}, /* 0x49 I*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0D}, /* 0x4A J*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0E}, /* 0x4B K*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x0F}, /* 0x4C L*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x10}, /* 0x4D M*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x11}, /* 0x4E N*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x12}, /* 0x4F O*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x13}, /* 0x50 P*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x14}, /* 0x51 Q*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x15}, /* 0x52 R*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x16}, /* 0x53 S*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x17}, /* 0x54 T*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x18}, /* 0x55 U*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x19}, /* 0x56 V*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1A}, /* 0x57 W*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1B}, /* 0x58 X*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1C}, /* 0x59 Y*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x1D}, /* 0x5A Z*/                \
        {0, 0x2F},                           /* 0x5B [*/                \
        {0, 0x31},                           /* 0x5C '\'*/              \
        {0, 0x30},                           /* 0x5D ]*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x23}, /* 0x5E ^*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x2D}, /* 0x5F _*/                \
        {0, 0x35},                           /* 0x60 `*/                \
        {0, 0x04},                           /* 0x61 a*/                \
        {0, 0x05},                           /* 0x62 b*/                \
        {0, 0x06},                           /* 0x63 c*/                \
        {0, 0x07},                           /* 0x64 d*/                \
        {0, 0x08},                           /* 0x65 e*/                \
        {0, 0x09},                           /* 0x66 f*/                \
        {0, 0x0A},                           /* 0x67 g*/                \
        {0, 0x0B},                           /* 0x68 h*/                \
        {0, 0x0C},                           /* 0x69 i*/                \
        {0, 0x0D},                           /* 0x6A j*/                \
        {0, 0x0E},                           /* 0x6B k*/                \
        {0, 0x0F},                           /* 0x6C l*/                \
        {0, 0x10},                           /* 0x6D m*/                \
        {0, 0x11},                           /* 0x6E n*/                \
        {0, 0x12},                           /* 0x6F o*/                \
        {0, 0x13},                           /* 0x70 p*/                \
        {0, 0x14},                           /* 0x71 q*/                \
        {0, 0x15},                           /* 0x72 r*/                \
        {0, 0x16},                           /* 0x73 s*/                \
        {0, 0x17},                           /* 0x74 t*/                \
        {0, 0x18},                           /* 0x75 u*/                \
        {0, 0x19},                           /* 0x76 v*/                \
        {0, 0x1A},                           /* 0x77 w*/                \
        {0, 0x1B},                           /* 0x78 x*/                \
        {0, 0x1C},                           /* 0x79 y*/                \
        {0, 0x1D},                           /* 0x7A z*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x2F}, /* 0x7B {*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x31}, /* 0x7C |*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x30}, /* 0x7D }*/                \
        {KEYBOARD_MODIFIER_LEFTSHIFT, 0x35}, /* 0x7E ~*/                \
        {0, 0},                              /* 0x7F DEL*/