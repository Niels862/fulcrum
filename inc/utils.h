#ifndef FUCO_UTILS_H
#define FUCO_UTILS_H

#define FUCO_UNUSED(p) (void)(p)
#define FUCO_NOT_IMPLEMENTED() fuco_not_implemented(__FILE__, __LINE__)

void fuco_not_implemented(char const *file, int line);

char fuco_base_char(int i);

char *fuco_repr_char(int c);

#endif
