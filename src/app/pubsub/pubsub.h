#pragma once

#include "stdint.h"

typedef void (*PubSubCallback)(void* msg);

// Publisher class
typedef struct {
    volatile uint8_t subscribe_count;
    PubSubCallback* subs_callbacks;
} PubSub;

PubSub* pubsub_alloc();
void pubsub_dealloc(PubSub*);
void pubsub_subscribe(PubSub* pubsub, void* callback);
void pubsub_unsubscribe(PubSub *pubsub, void *callback);
void pubsub_notify(PubSub*, void*);