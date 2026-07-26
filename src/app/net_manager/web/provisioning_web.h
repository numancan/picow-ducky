#pragma once

// Provisioning captive-portal web handlers; web_server owns the singleton lwIP
// hooks and forwards the actual work to the functions declared here (sibling of
// duckyscript's ducky_web.h).

#include "lwip/arch.h"
#include "web_server/post.h"

// Handle one CGI event parsed from the query string (e.g. name="forget").
void provisioning_cgi_dispatch(const char* name, const char* value);

// Streams the Wi-Fi scan list as a JSON array body (multipart).
int provisioning_ssi_scanjs(char* pcInsert, int iInsertLen, u16_t current_tag_part, u16_t* next_tag_part);
// Single-shot JSON object of the current connection state (raw NetState value).
int provisioning_ssi_status(char* pcInsert, int iInsertLen);

// POST route (see web_server/post.h) the master dispatches to after matching the
// request URI against its post_routes[] table.
extern const PostHandler provisioning_connect_post_handler;  // body -> Wi-Fi credentials (/connect)
