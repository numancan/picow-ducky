#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	uint8_t *pString; 		/* Points to application string to be sent */
	uint8_t keycodes[6];	/* Keycodes buffer to be sent */
	uint16_t counter;		/* Keeps the number of reports to be sent */
} kb_buffer_t;

typedef struct {
    char    *str; // TODO:
    uint8_t  keycode;
}str_conv_t;

enum {
	KB_STATUS_BUSY = 0,
	KB_STATUS_READY = 1
};

enum {
	INTERVAL_MS_MIN = 10,
	INTERVAL_MS_DEF = 20,
};

/**
 * Initialize board peripherals and USB stack.
 * 
 */
void kb_init();

/**
 * Change sending report interval. 
 * Default value:   20 	ms
 * Min value		:   10	ms
 * @param ms interval time
 */
static inline void kb_set_poll_ms(uint32_t ms);

/**
 * Handle keyboard report sending and main usb tasks.
 * Application must be called this function in main loop.
 */
void kb_task();
void usb_hid_task(void* params);

/**
 * Append all character to buffer to send by kb_task.
 * 
 * @param str Application supplied string which will be sent.
 */
void kb_send_string(uint8_t *str);

/**
 * Append keycodes to buffer to send by kb_task.
 * 
 * @param keycodes Array of keycodes. Max lenght is 6.
 */
void kb_send_keycodes(uint8_t keycodes[6]);

/**
 * TODO:gönderlicek item yoksa boştur
 * 
 */
bool kb_is_buffer_empty(); 

/**
 * TODO: Data to be written örnek gönderilcen item yoksa readydir
 * 
 */
static inline uint8_t kb_status () { return kb_is_buffer_empty() ? KB_STATUS_READY : KB_STATUS_BUSY; }

/**
 * Invoked when sent string/keycodes.
 * 
 */
void kb_sent_complete_cb();

/**
 * TODO:
 * 
 */
uint8_t kb_str_to_keycode(uint8_t *str);
uint8_t kb_ascii_to_keycode(uint8_t c);

// TODO: eksikleri tamamla
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


#endif