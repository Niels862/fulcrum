#ifndef FUCO_QUEUE_H
#define FUCO_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#define FUCO_QUEUE_INIT_SIZE 16

typedef struct {
    void **data;
    size_t cap;
    size_t size;
    size_t current;
} fuco_queue_t;

void fuco_queue_init(fuco_queue_t *queue);

void fuco_queue_destruct(fuco_queue_t *queue);

void fuco_queue_enqueue(fuco_queue_t *queue, void *data);

void *fuco_queue_dequeue(fuco_queue_t *queue);

bool fuco_queue_empty(fuco_queue_t *queue);

#endif
