#pragma once

#include "FreeRTOS.h"
#include "queue.h"

typedef void (*PubSubCallback)(const void*);

// Publisher class
typedef struct {
    volatile uint8_t subscribe_count;
    QueueHandle_t* subs_queue;
} PubSubFree;

PubSubFree* pubsub_free_alloc();
void pubsub_free_dealloc(PubSubFree*);
void pubsub_free_subscribe(PubSubFree*, QueueHandle_t);
void pubsub_free_notify(PubSubFree*, void*);