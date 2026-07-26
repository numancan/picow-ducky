#pragma once

// Duckyscript's web handlers, invoked by the web_server master via its lwIP hooks.

#include "lwip/arch.h"
#include "web_server/post.h"

// Handle one CGI event parsed from the query string (e.g. name="trigger",
// value="payload.txt"). `value` may be NULL for value-less events.
void ducky_cgi_dispatch(const char* name, const char* value);

// Streams the payload directory as a JSON array body (multipart). `state` is the
// opaque per-connection state produced by ducky_ssi_state_init().
u16_t ducky_ssi_payload(void* state, char* pcInsert, int iInsertLen, u16_t current_tag_part, u16_t* next_tag_part);
// Single-shot JSON object of the live ducky settings.
u16_t ducky_ssi_settings(char* pcInsert, int iInsertLen);
// Single-shot JSON object of the current playback status.
u16_t ducky_ssi_status(char* pcInsert, int iInsertLen);

// Per-connection SSI state factory/destructor. Returns non-NULL only for the
// payload-list file; other SSI files are stateless (returns NULL).
void* ducky_ssi_state_init(const char* name);
void ducky_ssi_state_free(void* state);

// Each route is a PostHandler (see web_server/post.h) matched against the request URI.
extern const PostHandler ducky_upload_post_handler;    // body -> payload file (/upload?p=NAME)
extern const PostHandler ducky_settings_post_handler;  // body -> config staging file (/settings)
extern const PostHandler ducky_live_post_handler;      // body -> live scratch file (/live)
