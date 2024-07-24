#ifndef FUCO_DEFS_H
#define FUCO_DEFS_H

#include <stdio.h>

typedef void(*fuco_free_t)(void *);

typedef void(*fuco_write_t)(void *, FILE *);

typedef struct fuco_symboltable_t fuco_symboltable_t;

typedef struct fuco_node_t fuco_node_t;

#endif