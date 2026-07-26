#pragma once

#include "lwip/arch.h"
#include "lwip/err.h"

struct pbuf;

// Contract a feature implements to own a POST route. The web_server master holds the
// singleton lwIP httpd_post_* symbols and dispatches to the matching handler; a new
// feature registers its own PostHandler in the master's post_routes[] table.
typedef struct {
    // Opens the destination for `uri`. Return ERR_OK to accept the body (streamed to
    // receive), a non-OK err_t to reject it. Set *post_auto_wnd = 0 for manual flow control.
    err_t (*begin)(const char* uri, u8_t* post_auto_wnd);
    // Streams one pbuf chain to the open destination. Takes ownership of `p` (frees it).
    err_t (*receive)(struct pbuf* p);
    // Closes the destination; may override response_uri with a redirect.
    void (*finished)(char* response_uri, u16_t response_uri_len);
} PostHandler;
