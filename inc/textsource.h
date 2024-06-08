#ifndef FUCO_TEXTSOURCE_H
#define FUCO_TEXTSOURCE_H

#include <stddef.h>

typedef struct {
    char *filename;
    size_t row;
    size_t col;
} fuco_textsource_t;

#endif
