#include "cgi.h"

#include "app/pubsub/pubsub.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwip/init.h"
#include "middleware/log.h"
#include "pico/stdlib.h"
#include "string.h"

static const char* TAG = "CGI";

static PubSub cgi_pubsub = {.subs_callbacks = NULL, .subscribe_count = 0};

static const char* cgi_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    CgiEvent cgi_event = {0, NULL};

    LOG_INFO(TAG, "CGI HANDLER: %s %s", pcParam[0], pcValue[0]);

    // TODO: can make without if else
    if (!strcmp(pcParam[0], "trigger")) {
        cgi_event.cgi_event_type = CGI_EVENT_TRIGGER;
        cgi_event.value = pcValue[0];

    } else if (strcmp(pcParam[0], "delete") == 0) {
        cgi_event.cgi_event_type = CGI_EVENT_DELETE;
        cgi_event.value = pcValue[0];

    } else if (strcmp(pcParam[0], "settings") == 0) {
        cgi_event.cgi_event_type = CGI_EVENT_SETTINGS;
    }

    if (cgi_event.cgi_event_type) pubsub_notify(&cgi_pubsub, &cgi_event);

    return "/index.shtml";
}

static tCGI cgi_handlers[] = {
    {"/", cgi_handler},
    {"/index.shtml", cgi_handler},
};

void cgi_event_subscribe(CgiCallback callback) { pubsub_subscribe(&cgi_pubsub, callback); }

void cgi_init() {
    // cgi_pubsub = pubsub_alloc();
    http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
}

void cgi_deinit() {}