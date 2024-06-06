#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

void fuco_not_implemented(char const *file, int line) {
    fprintf(stderr, "%s:%d: not implemented\n", file, line);
    exit(1);
}

char fuco_base_char(int i) {
    assert(i >= 0 && i < 36);

    if (i < 10) {
        return '0' + i;
    } else {
        return 'a' + (i - 10);
    }
}

char *fuco_repr_char(int c) {
    static char buf[5];

    if (isprint(c)) {
        buf[0] = c;
        buf[1] = '\0';
    } else {
        buf[0] = '\\';

        switch (c) {
            case '\0':
                buf[1] = '0';
                break;

            case '\n':
                buf[1] = 'n';
                break;
            
            case '\r':
                buf[1] = 'r';
                break;
            
            case '\t':
                buf[1] = 't';
                break;
            
            default:
                buf[1] = '\0';
                break;
        }

        if (buf[1] == '\0') {
            buf[1] = 'x';
            buf[2] = fuco_base_char(c / 16);
            buf[3] = fuco_base_char(c % 16);
            buf[4] = '\0';
        } else {
            buf[2] = '\0';
        }
    }

    return buf;
}
