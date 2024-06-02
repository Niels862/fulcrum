#include "strutils.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void fuco_strbuf_init(fuco_strbuf_t *buf) {
    buf->data = malloc(FUCO_STRBUF_INIT_SIZE);
    buf->len = 0;
    buf->cap = FUCO_STRBUF_INIT_SIZE;
}

void fuco_strbuf_clear(fuco_strbuf_t *buf) {
    buf->len = 0;
}

void fuco_strbuf_append(fuco_strbuf_t *buf, char *post) {
    FUCO_UNUSED(buf), FUCO_UNUSED(post);

    FUCO_NOT_IMPLEMENTED();
}

void fuco_strbuf_append_char(fuco_strbuf_t *buf, char c) {
    if (buf->len + 1 >= buf->cap) {
        buf->data = realloc(buf->data, 2 * buf->cap);
        buf->cap *= 2;
    }
    buf->data[buf->len] = c;
    buf->len++;
}

char *fuco_strbuf_dup(fuco_strbuf_t *buf) {
    char *new = malloc(buf->len + 1);
    memcpy(new, buf->data, buf->len + 1);

    return new;
}

void fuco_strbuf_destruct(fuco_strbuf_t *buf) {
    free(buf->data);
}

char *fuco_strdup(char *str) {
    size_t len = strlen(str);
    char *new = malloc(len + 1);
    memcpy(new, str, len + 1);

    return new;
}
