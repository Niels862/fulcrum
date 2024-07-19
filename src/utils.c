#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

void fuco_not_implemented(char const *file, int line) {
    fprintf(stderr, "%s:%d: not implemented\n", file, line);
    exit(1);
}

void fuco_syntax_error(fuco_textsource_t *source, char const *format, ...) {
    fprintf(stderr, FUCO_ANSI_RED "Error: ");

    if (source != NULL) {
        fuco_textsource_write(source, stderr);
        fprintf(stderr, ": ");
    }

    va_list args;
    va_start(args, format);

    vfprintf(stderr, format, args);

    va_end(args);

    fprintf(stderr, "\n" FUCO_ANSI_RESET);
}

char fuco_base_char(int i) {
    assert(i >= 0 && i < 36);

    if (i < 10) {
        return '0' + i;
    } else {
        return 'a' + (i - 10);
    }
}

char *fuco_repr_char(char c) {
    static char bufs[16][5];
    static size_t idx = 0;

    char *buf = bufs[idx];
    unsigned char b = c;

    idx = (idx + 1) % 16;

    if (isprint(b)) {
        buf[0] = c;
        buf[1] = '\0';
    } else {
        buf[0] = '\\';

        switch (b) {
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
            buf[2] = fuco_base_char(b / 16);
            buf[3] = fuco_base_char(b % 16);
            buf[4] = '\0';
        } else {
            buf[2] = '\0';
        }
    }

    return buf;
}

int fuco_ceil_log(unsigned int i, unsigned int base) {
    int log = 0;
    
    while (i > 0) {
        log++;
        i /= base;
    }

    return log;
}
