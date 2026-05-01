#pragma once

#include "view.h"
#include <stdbool.h>

// Menu specific constants
#define DM_VISIBLE_ITEMS 2

// Helper macros to reduce boilerplate
#define MENU_ITEM_ACTION(label, cb)                                            \
  { .text = label, .type = ITEM_TYPE_ACTION, .action = cb }

#define MENU_ITEM_SUBMENU(label, view)                                         \
  { .text = label, .type = ITEM_TYPE_SUBMENU, .sub_view = view }

#define MENU_ITEM_BACK(label)                                                  \
  { .text = label, .type = ITEM_TYPE_BACK }

#define MENU_ITEM_INPUT(label, buf, len)                                       \
  {                                                                            \
    .text = label, .type = ITEM_TYPE_INPUT, .input_buffer = buf,               \
    .input_max_len = len                                                       \
  }

#define MENU_ITEM_TOGGLE(label, cb, st)                                        \
  { .text = label, .type = ITEM_TYPE_TOGGLE, .action = cb, .state = st }

typedef struct MenuItem MenuItem;
typedef struct Menu Menu;

// Menu item types
typedef enum {
  ITEM_TYPE_ACTION,  // Normal list item, executes callback on select
  ITEM_TYPE_SUBMENU, // Navigates to a sub_view
  ITEM_TYPE_BACK,    // Returns to parent view
  ITEM_TYPE_INPUT,   // Text input field
  ITEM_TYPE_TOGGLE   // Boolean switch (ON/OFF)
} MenuItemType;

// Function pointer for menu actions
typedef void (*MenuActionCallback)(void);

// Menu item structure
struct MenuItem {
  const char *text;          // Displayed text
  MenuItemType type;         // Type of the item
  MenuActionCallback action; // Function to execute (for ACTION)
  View *sub_view;            // Pointer to view (for SUBMENU)
  char *input_buffer;        // Target string buffer
  int input_max_len;         // Maximum string len allocated
  bool *state;               // Target boolean state (for TOGGLE)
};

// Menu context structure (Page State)
struct Menu {
  const char *title;
  MenuItem *items;
  int item_count;

  // Internal State vars
  int selected_index;
  int scroll_offset;
  bool is_input_mode;
  int input_cursor_pos;
  int input_char_index;
};

// Menu View Initializer
void menu_view_init(View *view, Menu *menu, View *parent);
