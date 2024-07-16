#ifndef FUCO_TEXTSOURCE_H
#define FUCO_TEXTSOURCE_H

#include <stdio.h>
#include <stddef.h>

typedef struct {
    char *filename;
    size_t row;
    size_t col;
} fuco_textsource_t;

void fuco_textsource_init(fuco_textsource_t *source, char *filename);

void fuco_textsource_write(fuco_textsource_t *source, FILE *file);

#endif
