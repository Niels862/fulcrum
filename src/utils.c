#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void fuco_not_implemented(char const *file, int line) {
    fprintf(stderr, "%s:%d: not implemented\n", file, line);
    exit(1);
}
