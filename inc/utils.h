#ifndef FUCO_UTILS_H
#define FUCO_UTILS_H

#include "textsource.h"

#define FUCO_ANSI_RED "\033[91m"
#define FUCO_ANSI_RESET "\033[0m"

#define FUCO_UNUSED(p) (void)(p)
#define FUCO_NOT_IMPLEMENTED() fuco_not_implemented(__FILE__, __LINE__)

void fuco_not_implemented(char const *file, int line);

void fuco_syntax_error(fuco_textsource_t *source, char const *format, ...);

char fuco_base_char(int i);

char *fuco_repr_char(char c);

int fuco_ceil_log(unsigned int i, unsigned int base);

#endif
