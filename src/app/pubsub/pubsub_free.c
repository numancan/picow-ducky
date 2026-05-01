#include "stdlib.h"
#include "pubsub_free.h"
#include "middleware/debug.h"

PubSubFree* pubsub_free_alloc() {
    PubSubFree *pubsub = (PubSubFree*) malloc(sizeof(PubSubFree));
    pubsub->subs_queue = (QueueHandle_t*) malloc(sizeof(QueueHandle_t));
    pubsub->subscribe_count = 0;

    return pubsub;
}

void pubsub_free_dealloc(PubSubFree *pubsub) {
    free(pubsub->subs_queue);
    free(pubsub);
}

void pubsub_free_subscribe(PubSubFree *pubsub, QueueHandle_t queue)
{
    QueueHandle_t *tmp = (QueueHandle_t*) realloc(pubsub->subs_queue, sizeof(QueueHandle_t) * (pubsub->subscribe_count + 1));
    
    if (tmp == NULL) {
        DEBUG_PRINTF("pubsub_free_subscribe err: No memory!");
        free(pubsub->subs_queue);
    }

    pubsub->subs_queue = tmp;
    tmp = NULL;

    pubsub->subs_queue[pubsub->subscribe_count] = queue;
    pubsub->subscribe_count++;
}


void pubsub_free_notify(PubSubFree *pubsub, void *msg)
{
    for (size_t i = 0; i < pubsub->subscribe_count; i++)
    {
        xQueueSend(pubsub->subs_queue[i], msg, 0);
    }
}