#include "pubsub_free.h"

#include "middleware/log.h"
#include "stdio.h"
#include "stdlib.h"

static const char* TAG = "pubsub";

PubSubFree* pubsub_free_alloc() {
    PubSubFree* pubsub = (PubSubFree*)malloc(sizeof(PubSubFree));
    if (pubsub == NULL) return NULL;

    pubsub->subs_queue = NULL;
    pubsub->subscribe_count = 0;
    pubsub->mutex = xSemaphoreCreateMutex();

    return pubsub;
}

void pubsub_free_dealloc(PubSubFree* pubsub) {
    xSemaphoreTake(pubsub->mutex, portMAX_DELAY);

    vSemaphoreDelete(pubsub->mutex);
    free(pubsub->subs_queue);
    free(pubsub);
}

void pubsub_free_subscribe(PubSubFree* pubsub, QueueHandle_t queue) {
    if (pubsub == NULL || pubsub->mutex == NULL) return;

    if (xSemaphoreTake(pubsub->mutex, portMAX_DELAY) == pdTRUE) {
        QueueHandle_t* tmp =
            (QueueHandle_t*)realloc(pubsub->subs_queue, sizeof(QueueHandle_t) * (pubsub->subscribe_count + 1));

        if (tmp == NULL) {
            LOG_ERROR(TAG, "Subscribe err: No memory!");
            xSemaphoreGive(pubsub->mutex);
            return;
        }

        pubsub->subs_queue = tmp;
        pubsub->subs_queue[pubsub->subscribe_count] = queue;
        pubsub->subscribe_count++;

        xSemaphoreGive(pubsub->mutex);
    }
}

void pubsub_free_unsubscribe(PubSubFree* pubsub, QueueHandle_t queue) {
    if (pubsub == NULL || pubsub->mutex == NULL) return;

    if (xSemaphoreTake(pubsub->mutex, portMAX_DELAY) == pdTRUE) {
        for (size_t i = 0; i < pubsub->subscribe_count; i++) {
            if (pubsub->subs_queue[i] == queue) {
                // Shift remaining elements
                for (size_t j = i; j < pubsub->subscribe_count - 1; j++) {
                    pubsub->subs_queue[j] = pubsub->subs_queue[j + 1];
                }
                pubsub->subscribe_count--;

                // Shrink memory
                if (pubsub->subscribe_count > 0) {
                    QueueHandle_t* tmp =
                        (QueueHandle_t*)realloc(pubsub->subs_queue, sizeof(QueueHandle_t) * pubsub->subscribe_count);
                    if (tmp != NULL) {
                        pubsub->subs_queue = tmp;
                    }
                } else {
                    free(pubsub->subs_queue);
                    pubsub->subs_queue = NULL;
                }
                break;
            }
        }
        xSemaphoreGive(pubsub->mutex);
    }
}

void pubsub_free_notify(PubSubFree* pubsub, void* msg) {
    if (pubsub == NULL || pubsub->mutex == NULL) return;

    if (xSemaphoreTake(pubsub->mutex, portMAX_DELAY) == pdTRUE) {
        for (size_t i = 0; i < pubsub->subscribe_count; i++) {
            if (xQueueSend(pubsub->subs_queue[i], msg, 0) != pdTRUE) {
                LOG_WARN(TAG, "Notify err: Queue %d full", (int)i);
            }
        }
        xSemaphoreGive(pubsub->mutex);
    }
}