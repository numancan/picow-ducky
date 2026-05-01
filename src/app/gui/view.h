#pragma once

#include "middleware/input.h"
#include "u8g2.h"

// Forward declarations
typedef struct View View;
typedef struct ViewManager ViewManager;

// View structure definition
struct View {
    void (*draw)(View* view, u8g2_t* u8g2);
    void (*on_enter)(View* view);
    void (*on_exit)(View* view);
    void (*on_input)(View* view, InputEvent* event, ViewManager* view_manager);
    void* context;  // View-specific state
    View* parent;
};
