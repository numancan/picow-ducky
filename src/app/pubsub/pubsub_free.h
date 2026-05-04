#pragma once

#include "FreeRTOS.h"
#include "queue.h"

typedef void (*PubSubCallback)(const void*);

#include "semphr.h"

// Publisher class
typedef struct {
    volatile uint8_t subscribe_count;
    QueueHandle_t* subs_queue;
    SemaphoreHandle_t mutex;
} PubSubFree;

PubSubFree* pubsub_free_alloc();
void pubsub_free_dealloc(PubSubFree*);
void pubsub_free_subscribe(PubSubFree*, QueueHandle_t);
void pubsub_free_unsubscribe(PubSubFree*, QueueHandle_t);
void pubsub_free_notify(PubSubFree*, void*);