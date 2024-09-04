#include <queue.h>
#include <stdlib.h>
#include <assert.h>

void fuco_queue_init(fuco_queue_t *queue) {
    queue->cap = FUCO_QUEUE_INIT_SIZE;
    queue->data = malloc(queue->cap * sizeof(void *));
    queue->current = 0;
    queue->size = 0;
}

void fuco_queue_destruct(fuco_queue_t *queue) {
    free(queue->data);
}

void fuco_queue_enqueue(fuco_queue_t *queue, void *data) {
    if (queue->size > queue->cap) {
        queue->cap *= 2;
        queue->data = realloc(queue->data, queue->cap);
    }

    queue->data[queue->size] = data;
    queue->size++;
}

void *fuco_queue_dequeue(fuco_queue_t *queue) {
    assert(!fuco_queue_empty(queue));

    void *data = queue->data[queue->current];
    queue->current++;
    return data;
}

bool fuco_queue_empty(fuco_queue_t *queue) {
    return queue->current >= queue->size;
}
