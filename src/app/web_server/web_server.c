#include "web_server.h"

#include "cgi.h"
#include "lwip/apps/httpd.h"
#include "middleware/sys_fault.h"
#include "ssi.h"

static bool httpd_started = false;

// lock when use
void web_server_init() {
    // cyw43_arch_deinit not clear lwip_init so we only init once. (sdk 2.3.0)
    if (!httpd_started) {
        httpd_init();
        ssi_init();
        cgi_init();
        httpd_started = true;
    }
}
