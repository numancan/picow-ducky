#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../view.h"

typedef enum { MENU_TYPE_FIXED, MENU_TYPE_DYNAMIC } MenuType;

typedef void (*MenuItemCallback)(void* context);

typedef struct {
    const char* label;
    MenuItemCallback callback;
    void* callback_context;
} MenuItem;

typedef MenuItem (*MenuGetItemCallback)(void* context, uint16_t index);
typedef uint16_t (*MenuGetCountCallback)(void* context);

typedef struct {
    const char* title;
    uint16_t selected;
    MenuType type;
    View view;

    union {
        struct {
            MenuItem* items;
            uint16_t capacity;
            uint16_t count;
        } fixed;

        struct {
            MenuGetItemCallback get_item;
            MenuGetCountCallback get_count;
            void* context;
        } dynamic;
    } model;
} Menu;

/* Initializes a menu backed by a fixed, caller-owned item array. */
void menu_init_fixed(Menu* menu, const char* title, MenuItem* buffer, uint16_t capacity);
/* Initializes a menu backed by dynamically-fetched items. */
void menu_init_dynamic(Menu* menu, const char* title, MenuGetItemCallback get_item, MenuGetCountCallback get_count,
                       void* context);

/* Appends an item to a fixed menu. */
void menu_add_item(Menu* menu, const char* label, MenuItemCallback callback, void* callback_context);

/* The view to register with the ViewManager. */
View* menu_get_view(Menu* menu);
