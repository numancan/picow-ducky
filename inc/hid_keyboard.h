#ifndef _HID_KEYBOARD_H_
#define _HID_KEYBOARD_H_

#define HID_STRING_TO_KEYCODE               \
  { "GUI",          HID_KEY_GUI_LEFT },     \
  { "ALT",          HID_KEY_ALT_LEFT },     \
  { "CAPSLOCK",     HID_KEY_CAPS_LOCK },    \
  { "ENTER",        HID_KEY_ENTER },        \
  { "DELETE",       HID_KEY_DELETE },       \
  { "HOME",         HID_KEY_HOME },         \
  { "INSERT",       HID_KEY_INSERT },       \
  { "PAGEUP",       HID_KEY_PAGE_UP },      \
  { "PAGEDOWN",     HID_KEY_PAGE_DOWN },    \
  { "WINDOWS",      HID_KEY_GUI_LEFT },     \
  { "UPARROW",      HID_KEY_ARROW_UP },     \
  { "DOWNARROW",    HID_KEY_ARROW_DOWN },   \
  { "LEFTARROW",    HID_KEY_ARROW_LEFT },   \
  { "RIGHTARROW",   HID_KEY_ARROW_RIGHT },  \
  { "TAB",          HID_KEY_TAB },          \
  { "SHIFT",        HID_KEY_SHIFT_LEFT},    \
  { "SPACE",        HID_KEY_SPACE },        \
  { "PRINTSCREEN",  HID_KEY_PRINT_SCREEN }, \
  { "ESC",          HID_KEY_ESCAPE },       \
  { "ESCAPE",       HID_KEY_ESCAPE },       \
  { "PAUSE",        HID_KEY_PAUSE },        \
  { "HOME",         HID_KEY_HOME },         \
  { "F1",           HID_KEY_F1 },           \
  { "F2",           HID_KEY_F2 },           \
  { "F3",           HID_KEY_F3 },           \
  { "F4",           HID_KEY_F4 },           \
  { "F5",           HID_KEY_F5 },           \
  { "F6",           HID_KEY_F6 },           \
  { "F7",           HID_KEY_F7 },           \
  { "F8",           HID_KEY_F8 },           \
  { "F9",           HID_KEY_F9 },           \
  { "F10",          HID_KEY_F10 },          \
  { "F11",          HID_KEY_F11 },          \
  { "F12",          HID_KEY_F12 },          \
  { "MENU",         HID_KEY_MENU },         \
  { "APP",          HID_KEY_APPLICATION },  \
  { "SCROLLLOCK",   HID_KEY_SCROLL_LOCK }   \

#define HID_ASCII_TO_KEYCODE \
{0, 0},  /*NUL Null char*/ \
{0, 0},  /*SOH Start of Heading*/ \
{0, 0},  /*STX Start of Text*/ \
{0, 0},  /*ETX End of Text*/ \
{0, 0},  /*EOT End of Transmission*/ \
{0, 0},  /*ENQ Enquiry*/ \
{0, 0},  /*ACK Acknowledgement*/ \
{0, 0},  /*BEL Bell*/ \
{0, 0},  /*BS 	Back Space*/ \
{0, HID_KEY_TAB},  /*HT 	Horizontal Tab*/ \
{0, HID_KEY_ENTER},  /*LF 	Line Feed*/ \
{0, 0},  /*VT 	Vertical Tab*/ \
{0, 0},  /*FF 	Form Feed*/ \
{0, 0},  /*CR 	Carriage Return*/ \
{0, 0},  /*SO 	Shift Out / X-On*/ \
{0, 0},  /*SI 	Shift In / X-Off*/ \
{0, 0},  /*DLE Data Line Escape*/ \
{0, 0},  /*DC1 Device Control 1 (oft.XON)*/ \
{0, 0},  /*DC2 Device Control 2*/ \
{0, 0},  /*DC3 Device Control 3 (oft.XOFF)*/ \
{0, 0},  /*DC4 Device Control 4*/ \
{0, 0},  /*NAK Negative Acknowledgement*/ \
{0, 0},  /*SYN Synchronous Idle*/ \
{0, 0},  /*ETB End of Transmit Block*/ \
{0, 0},  /*CAN Cancel*/ \
{0, 0},  /*EM 	End of Medium*/ \
{0, 0},  /*SUB Substitute*/ \
{0, 0},  /*ESC Escape*/ \
{0, 0},  /*FS 	File Separator*/ \
{0, 0},  /*GS 	Group Separator*/ \
{0, 0},  /*RS 	Record Separator*/ \
{0, 0},  /*US 	Unit Separator*/ \
{0, HID_KEY_SPACE},  /*SPACE Space*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_1},  /*! Exclamation mark*/ \
{0, HID_KEY_GRAVE},  /*“ Double quotes (or speech marks)*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_3},  /*# Number*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_4},  /*$ Dollar*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_5},  /*% Percent*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_6},  /*& Ampersand*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_2},  /*‘ Single quote*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_8},  /*( Open parenthesis (or open bracket)*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_9},  /*) Close parenthesis (orclose bracket)*/ \
{0, HID_KEY_MINUS},  /** Asterisk*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_4},  /*+ Plus*/ \
{0, HID_KEY_BACKSLASH},  /*, Comma*/ \
{0, HID_KEY_EQUAL},  /*- Hyphen*/ \
{0, HID_KEY_SLASH},  /*. Period, dot or full stop*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_7},  /*/ Slash or divide*/ \
{0, HID_KEY_0},  /*0 Zero*/ \
{0, HID_KEY_1},  /*1 One*/ \
{0, HID_KEY_2},  /*2 Two*/ \
{0, HID_KEY_3},  /*3 Three*/ \
{0, HID_KEY_4},  /*4 Four*/ \
{0, HID_KEY_5},  /*5 Five*/ \
{0, HID_KEY_6},  /*6 Six*/ \
{0, HID_KEY_7},  /*7 Seven*/ \
{0, HID_KEY_8},  /*8 Eight*/ \
{0, HID_KEY_9},  /*9 Nine*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_SLASH},  /*: Colon*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_BACKSLASH},  /*; Semicolon*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_GRAVE},  /*< Less than (or open angled bracket)*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_0},  /*= Equals*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_1},  /*> Greater than (or closeangled bracket)*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_MINUS},  /*? Question mark*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_Q},  /*@ At symbol*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_A},  /*A Uppercase A*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_B},  /*B Uppercase B*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_C},  /*C Uppercase C*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_D},  /*D Uppercase D*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_E},  /*E Uppercase E*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_F},  /*F Uppercase F*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_G},  /*G Uppercase G*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_H},  /*H Uppercase H*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_I},  /*I Uppercase I*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_J},  /*J Uppercase J*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_K},  /*K Uppercase K*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_L},  /*L Uppercase L*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_M},  /*M Uppercase M*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_N},  /*N Uppercase N*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_O},  /*O Uppercase O*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_P},  /*P Uppercase P*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_Q},  /*Q Uppercase Q*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_R},  /*R Uppercase R*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_S},  /*S Uppercase S*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_T},  /*T Uppercase T*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_U},  /*U Uppercase U*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_V},  /*V Uppercase V*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_W},  /*W Uppercase W*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_X},  /*X Uppercase X*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_Y},  /*Y Uppercase Y*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_Z},  /*Z Uppercase Z*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_8},  /*[ Opening bracket*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_MINUS},  /*\ Backslash*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_9},  /*] Closing bracket*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_3},  /*^ Caret - circumflex*/ \
{KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_EQUAL},  /*_ Underscore*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_BACKSLASH},  /*` Grave accent*/ \
{0, HID_KEY_A},  /*a Lowercase a*/ \
{0, HID_KEY_B},  /*b Lowercase b*/ \
{0, HID_KEY_C},  /*c Lowercase c*/ \
{0, HID_KEY_D},  /*d Lowercase d*/ \
{0, HID_KEY_E},  /*e Lowercase e*/ \
{0, HID_KEY_F},  /*f Lowercase f*/ \
{0, HID_KEY_G},  /*g Lowercase g*/ \
{0, HID_KEY_H},  /*h Lowercase h*/ \
{0, HID_KEY_APOSTROPHE},  /*i Lowercase i*/ \
{0, HID_KEY_J},  /*j Lowercase j*/ \
{0, HID_KEY_K},  /*k Lowercase k*/ \
{0, HID_KEY_L},  /*l Lowercase l*/ \
{0, HID_KEY_M},  /*m Lowercase m*/ \
{0, HID_KEY_N},  /*n Lowercase n*/ \
{0, HID_KEY_O},  /*o Lowercase o*/ \
{0, HID_KEY_P},  /*p Lowercase p*/ \
{0, HID_KEY_Q},  /*q Lowercase q*/ \
{0, HID_KEY_R},  /*r Lowercase r*/ \
{0, HID_KEY_S},  /*s Lowercase s*/ \
{0, HID_KEY_T},  /*t Lowercase t*/ \
{0, HID_KEY_U},  /*u Lowercase u*/ \
{0, HID_KEY_V},  /*v Lowercase v*/ \
{0, HID_KEY_W},  /*w Lowercase w*/ \
{0, HID_KEY_X},  /*x Lowercase x*/ \
{0, HID_KEY_Y},  /*y Lowercase y*/ \
{0, HID_KEY_Z},  /*z Lowercase z*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_7},  /*{ Opening brace*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_EQUAL},  /*| Vertical bar*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_0},  /*} Closing brace*/ \
{KEYBOARD_MODIFIER_RIGHTALT, HID_KEY_BRACKET_RIGHT},  /*~ Equivalency sign - tilde*/ \
{0, HID_KEY_BACKSPACE}  /*DEL Delete*/ \

typedef struct {
  uint8_t  *pKeys;
  uint8_t   modifier;
  uint8_t   keycodes[6];
  int16_t   counter;
  uint8_t   last_payload[128];
  uint8_t   repeat_count;
} key_buffer_t;

typedef struct {
    char    *str;
    uint8_t  keycode;
}str_conv_t;

typedef enum  {
  HID_TASK_NOT_READY = 0,
  HID_TASK_BUSY = 1,
  HID_TASK_NOT_BUSY = 2
}HID_STATUS;

enum  {
  BLINK_DISABLED = 0,
  BLINK_NOT_MOUNTED = 250,  // device not mounted
  BLINK_MOUNTED = 1000,     // device mounted
  BLINK_SUSPENDED = 2500,   // device is suspended
};

void hid_init(void);
HID_STATUS hid_task();

// void handle_ducky(uint8_t*, bool);

// void _keyboard_report_chr(uint8_t);
// void append_buff_string(uint8_t*, uint8_t);
// uint8_t _keycode_from_string(char*);
#endif