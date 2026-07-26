#pragma once

#include "ducky_settings.h"
#include "gui/view.h"
#include "gui/view_manager.h"

typedef struct DuckySettingsView {
    View view;  // embedded directly; the view context points back to this struct
    DuckySettings* settings;
    uint8_t selected;
    bool dirty;  // set when a value changes; persisted on exit
} DuckySettingsView;

// Wire up the settings view and register it with the view manager.
void ducky_settings_view_init(DuckySettingsView* ducky_settings_view, ViewManager* vm, DuckySettings* settings);