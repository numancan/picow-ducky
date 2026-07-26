#include "cgi.h"

#include "duckyscript/web/ducky_web.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "net_manager/web/provisioning_web.h"

static const char* ducky_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    if (iNumParams < 1) return "/api/ducky/status.shtml";

    ducky_cgi_dispatch(pcParam[0], pcValue[0]);
    return "/api/ducky/status.shtml";
}

static const char* provisioning_handler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    if (iNumParams < 1) return "/api/portal/status.shtml";

    provisioning_cgi_dispatch(pcParam[0], pcValue[0]);
    return "/api/portal/status.shtml";
}

// Dedicated CGI endpoints, not page URLs: a plain page load must never trigger a handler.
static tCGI cgi_handlers[] = {
    {"/api/ducky/cmd.cgi", ducky_handler},
    {"/api/portal/cmd.cgi", provisioning_handler},
};

void cgi_init() { http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers)); }
