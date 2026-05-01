#include "settings_view.h"

#include "gui/menu_view.h"
#include "settings.h"

static bool pti_state = false;
static bool webserver_state = false;
static bool settings_changed = false;

static void on_setting_changed(void) { settings_changed = true; }

static void settings_pti_toggle_action(void) {
    settings_set_bool(SETTINGS_ID_PAYLOAD_TO_INJECT, pti_state);
    on_setting_changed();
}

static void settings_webserver_toggle_action(void) {
    settings_set_bool(SETTINGS_ID_WEB_SERVER_ENABLED, webserver_state);
    on_setting_changed();
}

static MenuItem settings_items[] = {
    MENU_ITEM_TOGGLE("Web Server", settings_webserver_toggle_action, &webserver_state),
    {.text = "SSID",
     .type = ITEM_TYPE_INPUT,
     .action = on_setting_changed,
     .input_max_len = SETTINGS_MAX_PARAM_VAL_LEN},
    {.text = "Password",
     .type = ITEM_TYPE_INPUT,
     .action = on_setting_changed,
     .input_max_len = SETTINGS_MAX_PARAM_VAL_LEN},
    MENU_ITEM_TOGGLE("PTI", settings_pti_toggle_action, &pti_state),
    {.text = "Payload",
     .type = ITEM_TYPE_INPUT,
     .action = on_setting_changed,
     .input_max_len = SETTINGS_MAX_PARAM_VAL_LEN},
    MENU_ITEM_BACK("< Back"),
};

static Menu settings_menu = {
    .title = "Settings",
    .items = settings_items,
    .item_count = 6,
};

void settings_view_on_exit(View* view) {
    if (settings_changed) {
        settings_save();
        settings_changed = false;
    }
}

void settings_view_init(View* view, View* parent) {
    // Link input buffers directly to settings param values
    SettingParam* ssid = settings_get_param_wID(SETTINGS_ID_WIFI_SSID);
    SettingParam* pass = settings_get_param_wID(SETTINGS_ID_WIFI_PASS);
    SettingParam* pname = settings_get_param_wID(SETTINGS_ID_PAYLOAD_NAME);

    // Point input_buffer directly to settings value arrays
    settings_items[1].input_buffer = ssid->val.s;
    settings_items[2].input_buffer = pass->val.s;
    settings_items[4].input_buffer = pname->val.s;

    // Init state from current setting
    pti_state = settings_get_bool(SETTINGS_ID_PAYLOAD_TO_INJECT);
    webserver_state = settings_get_bool(SETTINGS_ID_WEB_SERVER_ENABLED);

    settings_changed = false;

    menu_view_init(view, &settings_menu, parent);
    view->on_exit = settings_view_on_exit;
}
