#include "textsource.h"
#include <stdio.h>

void fuco_textsource_init(fuco_textsource_t *source, char *filename) {
    source->filename = filename;
    source->row = 1;
    source->col = 0;
}

void fuco_textsource_write(fuco_textsource_t *source, FILE *file) {
    fprintf(file, "%s:%ld:%ld", source->filename, source->row, source->col);
}
