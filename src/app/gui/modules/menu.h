#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "../view.h"

typedef void (*MenuItemCallback)(void* context);

typedef struct {
    const char* label;
    MenuItemCallback callback;
    void* callback_context;
} MenuItem;

typedef MenuItem (*MenuGetItemCallback)(void* context, uint16_t index);
typedef uint16_t (*MenuGetCountCallback)(void* context);

typedef enum { MENU_TYPE_FIXED, MENU_TYPE_DYNAMIC } MenuType;

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

void menu_init_fixed(Menu* menu, const char* title, MenuItem* buf, uint16_t capacity);
void menu_init_dynamic(Menu* menu, const char* title, MenuGetItemCallback get_item, MenuGetCountCallback get_count,
                       void* context);

void menu_add_item(Menu* menu, const char* label, MenuItemCallback callback, void* callback_context);

/* The view to register with the ViewManager. */
View* menu_get_view(Menu* menu);
