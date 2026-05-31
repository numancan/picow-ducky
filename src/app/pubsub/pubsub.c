#include "pubsub.h"

#include "middleware/log.h"
#include "stdlib.h"

PubSub* pubsub_alloc() {
    PubSub* pubsub = (PubSub*)malloc(sizeof(PubSub));
    pubsub->subs_callbacks = NULL;
    pubsub->subscribe_count = 0;

    return pubsub;
}

void pubsub_dealloc(PubSub* pubsub) {
    if (pubsub == NULL) return;

    free(pubsub->subs_callbacks);
    free(pubsub);
}

void pubsub_subscribe(PubSub* pubsub, void* callback) {
    if (pubsub == NULL || callback == NULL) {
        printf("pubsub_subscribe err: Invalid arguments (pubsub or callback is NULL)!\n");
        return;
    }

    PubSubCallback* tmp =
        (PubSubCallback*)realloc(pubsub->subs_callbacks, sizeof(PubSubCallback) * (pubsub->subscribe_count + 1));

    if (tmp == NULL) {
        printf("pubsub_subscribe err: No memory for realloc! (current count: %u)\n", pubsub->subscribe_count);
        return;
    }

    pubsub->subs_callbacks = tmp;
    pubsub->subs_callbacks[pubsub->subscribe_count] = callback;
    pubsub->subscribe_count++;
}

void pubsub_unsubscribe(PubSub* pubsub, void* callback) {
    if (pubsub == NULL || callback == NULL) {
        printf("pubsub_unsubscribe err: Invalid arguments (pubsub or callback is NULL)!\n");
        return;
    }

    if (pubsub->subscribe_count == 0 || pubsub->subs_callbacks == NULL) {
        printf("pubsub_unsubscribe err: No subscribers to remove!\n");
        return;
    }

    int index = -1;
    for (uint16_t i = 0; i < pubsub->subscribe_count; i++) {
        if (pubsub->subs_callbacks[i] == callback) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("pubsub_unsubscribe: Callback not found in list!\n");
        return;
    }

    for (uint16_t i = index; i < pubsub->subscribe_count - 1; i++) {
        pubsub->subs_callbacks[i] = pubsub->subs_callbacks[i + 1];
    }

    pubsub->subscribe_count--;

    PubSubCallback* tmp =
        (PubSubCallback*)realloc(pubsub->subs_callbacks, sizeof(PubSubCallback) * pubsub->subscribe_count);

    if (tmp == NULL) {
        printf("pubsub_unsubscribe warn: realloc failed, keeping old pointer.\n");
        return;
    }

    pubsub->subs_callbacks = tmp;
}

void pubsub_notify(PubSub* pubsub, void* msg) {
    if (pubsub == NULL) return;

    for (size_t i = 0; i < pubsub->subscribe_count; i++) {
        pubsub->subs_callbacks[i](msg);
    }
}