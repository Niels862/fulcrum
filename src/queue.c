#include "queue.h"
#include <stdlib.h>
#include <string.h>

void fuco_queue_init(fuco_queue_t *queue) {
    queue->data = malloc(FUCO_QUEUE_INIT_SIZE * sizeof(void *));
    queue->front = queue->back = 0;
    queue->cap = FUCO_QUEUE_INIT_SIZE;
}

void fuco_queue_destruct(fuco_queue_t *queue, fuco_free_t func) {
    for (size_t i = 0; i < queue->back; i++) {
        func(queue->data[i]);
    }
    free(queue->data);
}

void fuco_queue_enqueue(fuco_queue_t *queue, void *entry) {
    if (queue->front + 1 >= queue->cap) {
        fuco_queue_resize(queue);
    }

    queue->data[queue->back] = entry;
    queue->back++;
}

void *fuco_queue_dequeue(fuco_queue_t *queue) {
    if (fuco_queue_is_empty(queue)) {
        return NULL;
    }

    void *entry = queue->data[queue->front];
    queue->front++;
    return entry;
}

void fuco_queue_resize(fuco_queue_t *queue) {
    queue->data = realloc(queue->data, queue->cap);
    queue->cap *= 2;
}

bool fuco_queue_is_empty(fuco_queue_t *queue) {
    return queue->back == queue->front;
}
