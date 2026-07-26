#include "post.h"

#include <stdio.h>
#include <string.h>

#include "duckyscript/web/ducky_web.h"
#include "lwip/apps/httpd.h"
#include "lwip/def.h"
#include "lwip/pbuf.h"
#include "net_manager/web/provisioning_web.h"

// lwIP resolves these three POST symbols at link time; there can be exactly one definition
// each in the binary. The master owns URI routing (post_routes[]) and per-connection tracking;
// the owning feature implements the PostHandler it registers. A new feature adds a row here and
// exports its handler — no other master change.

typedef struct {
    const char* uri;
    const PostHandler* handler;
} PostRoute;

static const PostRoute post_routes[] = {
    {"/upload", &ducky_upload_post_handler},
    {"/settings", &ducky_settings_post_handler},
    {"/live", &ducky_live_post_handler},
    {"/connect", &provisioning_connect_post_handler},
};

static void* current_connection = NULL;
static const PostHandler* current_handler = NULL;

static const PostHandler* get_post_handler(const char* uri) {
    for (size_t i = 0; i < LWIP_ARRAYSIZE(post_routes); i++) {
        const char* defined_uri = post_routes[i].uri;

        if (strncmp(uri, defined_uri, strlen(defined_uri)) == 0) {
            return post_routes[i].handler;
        }
    }
    return NULL;
}

err_t httpd_post_begin(void* connection, const char* uri, const char* http_request, u16_t http_request_len,
                       int content_len, char* response_uri, u16_t response_uri_len, u8_t* post_auto_wnd) {
    LWIP_UNUSED_ARG(http_request);
    LWIP_UNUSED_ARG(http_request_len);
    LWIP_UNUSED_ARG(content_len);
    LWIP_UNUSED_ARG(response_uri);
    LWIP_UNUSED_ARG(response_uri_len);

    if (current_connection == connection) return ERR_OK;

    current_connection = connection;
    current_handler = get_post_handler(uri);
    if (current_handler == NULL) return ERR_VAL;

    err_t err = current_handler->begin(uri, post_auto_wnd);
    if (err != ERR_OK) current_handler = NULL;

    return err;
}

err_t httpd_post_receive_data(void* connection, struct pbuf* p) {
    if (current_connection != connection || current_handler == NULL) {
        pbuf_free(p);
        return ERR_VAL;
    }

    return current_handler->receive(p);
}

void httpd_post_finished(void* connection, char* response_uri, u16_t response_uri_len) {
    if (current_connection != connection || current_handler == NULL) return;

    // Generic default response; the handler overrides it if its route needs a redirect.
    snprintf(response_uri, response_uri_len, "/");
    current_handler->finished(response_uri, response_uri_len);

    current_connection = NULL;
    current_handler = NULL;
}
