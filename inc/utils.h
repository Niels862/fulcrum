#ifndef FUCO_UTILS_H
#define FUCO_UTILS_H

#include "textsource.h"
#include <stdlib.h>
#include <stdbool.h>

#define FUCO_MIN(x, y) (((x) < (y)) ? (x) : (y))

#define FUCO_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

#define FUCO_ANSI_RED "\033[91m"
#define FUCO_ANSI_RESET "\033[0m"

#define FUCO_UNUSED(p) (void)(p)

#define FUCO_UNREACHED() \
        fuco_fatal_error(__FILE__, __LINE__, "executed unreachable section")

#define FUCO_NOT_IMPLEMENTED() \
        fuco_fatal_error(__FILE__, __LINE__, "not implemented")

void fuco_fatal_error(char const *file, int line, 
                      char const *msg) __attribute__((noreturn));

void fuco_syntax_error(fuco_textsource_t *source, char const *format, ...);

char fuco_base_char(int i);

/* Static rotation: max. 16 uses at once */
char *fuco_repr_char(char c);

int fuco_ceil_log(unsigned int i, unsigned int base);

void fuco_pointer_write(void *p, FILE *file);

#endif
