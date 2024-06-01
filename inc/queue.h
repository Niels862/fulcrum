#ifndef FUCO_QUEUE_H
#define FUCO_QUEUE_H

#include <stddef.h>
#include <stdbool.h>
#include "defs.h"

#define FUCO_QUEUE_INIT_SIZE 16

/* Note: dequeued elements are not deleted and stay valid until destruction */
typedef struct {
    void **data;
    size_t front;
    size_t back;
    size_t cap;
} fuco_queue_t;

void fuco_queue_init(fuco_queue_t *queue);

void fuco_queue_destruct(fuco_queue_t *queue, fuco_free_t func);

void fuco_queue_enqueue(fuco_queue_t *queue, void *data);

void *fuco_queue_dequeue(fuco_queue_t *queue);

void fuco_queue_resize(fuco_queue_t *queue);

bool fuco_queue_is_empty(fuco_queue_t *queue);

#endif
