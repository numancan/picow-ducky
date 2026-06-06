#include "cgi.h"

#include "app/duckyscript/ducky.h"
#include "app/settings/settings.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwip/init.h"
#include "middleware/log.h"
#include "pico/stdlib.h"
#include "string.h"

static const char* TAG = "CGI";

typedef enum { CGI_EVENT_NONE = 0, CGI_EVENT_TRIGGER, CGI_EVENT_DELETE, CGI_EVENT_SETTINGS } CgiEventType;

static CgiEventType cgi_event_type_from_str(const char* str) {
    if (strcmp(str, "trigger") == 0) return CGI_EVENT_TRIGGER;
    if (strcmp(str, "delete") == 0) return CGI_EVENT_DELETE;
    if (strcmp(str, "settings") == 0) return CGI_EVENT_SETTINGS;
    return CGI_EVENT_NONE;
}

static const char* cgi_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    if (iNumParams < 1) return "/index.shtml";

    LOG_INFO(TAG, "CGI HANDLER: %s %s", pcParam[0], pcValue[0]);

    // TODO: lwIP TCP thread blocking here not good idea
    switch (cgi_event_type_from_str(pcParam[0])) {
        case CGI_EVENT_TRIGGER: ducky_play_script(pcValue[0]); break;
        case CGI_EVENT_DELETE: break;  // TODO: payload delete
        case CGI_EVENT_SETTINGS: settings_load_from_file(); break;
        case CGI_EVENT_NONE: break;
    }

    return "/index.shtml";
}

static tCGI cgi_handlers[] = {
    {"/", cgi_handler},
    {"/index.shtml", cgi_handler},
};

void cgi_init() { http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers)); }
