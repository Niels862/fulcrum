#ifndef FUCO_STRUTILS_H
#define FUCO_STRUTILS_H

#include "map.h"
#include <stddef.h>
#include <stdio.h>

#define FUCO_STRBUF_INIT_SIZE 256

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} fuco_strbuf_t;

void fuco_strbuf_init(fuco_strbuf_t *buf);

void fuco_strbuf_clear(fuco_strbuf_t *buf);

void fuco_strbuf_append(fuco_strbuf_t *buf, char *post);

void fuco_strbuf_append_char(fuco_strbuf_t *buf, char c);

char *fuco_strbuf_dup(fuco_strbuf_t *buf);

void fuco_strbuf_destruct(fuco_strbuf_t *buf);

char *fuco_strdup(char *str);

void fuco_string_write(void *string, FILE *file);

fuco_hashvalue_t fuco_string_hash(void *data);

bool fuco_string_equal(void *left, void *right);

#endif
