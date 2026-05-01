#pragma once


typedef enum { CGI_EVENT_TRIGGER = 1, CGI_EVENT_DELETE, CGI_EVENT_SETTINGS  } CgiEventType;

typedef struct {
    CgiEventType cgi_event_type;
    char *value;
} CgiEvent;

typedef void (*CgiCallback)(CgiEvent *event);

void cgi_init();
void cgi_event_subscribe(CgiCallback callback);