#ifndef FUCO_TOKENIZER_H
#define FUCO_TOKENIZER_H

#include <stddef.h>
#include <stdio.h>
#include "strutils.h"
#include "queue.h"

typedef enum {
    FUCO_TOKEN_NULL,
    FUCO_TOKEN_INTEGER,
    FUCO_TOKEN_EOF
} fuco_tokentype_t;

typedef struct {
    char *filename;
    size_t row;
    size_t col;
} fuco_textsource_t;

typedef struct {
    char *lexeme;
    fuco_textsource_t source;
    fuco_tokentype_t type;
} fuco_token_t;

#define FUCO_FILEBUF_SIZE 16

typedef struct {
    fuco_textsource_t source;
    FILE *file;
    size_t p;
    size_t size;
    char data[FUCO_FILEBUF_SIZE];
} fuco_filebuf_t;

typedef struct {
    FILE *file;
    char *filename;
    fuco_tokentype_t last;
    fuco_queue_t source_filenames;
    fuco_token_t curr;
    fuco_strbuf_t temp;
} fuco_tokenizer_t;

char *fuco_tokentype_string(fuco_tokentype_t type);

void fuco_textsource_init(fuco_textsource_t *source, char *filename);

void fuco_textsource_write(fuco_textsource_t *source, FILE *stream);

void fuco_token_init(fuco_token_t *token);

void fuco_token_destruct(fuco_token_t *token);

void fuco_token_write(fuco_token_t *token, FILE *stream);

void fuco_tokenizer_init(fuco_tokenizer_t *tokenizer);

void fuco_tokenizer_destruct(fuco_tokenizer_t *tokenizer);

/* Takes ownership of filename */
void fuco_tokenizer_add_source_filename(fuco_tokenizer_t *tokenizer, 
                                        char *filename);

int fuco_tokenizer_open_next_source(fuco_tokenizer_t *tokenizer);
/* 
 * Update filebuf:
 * - if not yet initialized: open first source file
 * - if current file at EOF: read next file (possibly repeatedly)
 */
void fuco_tokenizer_update_filebuf(fuco_tokenizer_t *tokenizer, 
                                   fuco_filebuf_t *buf);

void fuco_tokenizer_next_token(fuco_tokenizer_t *tokenizer);

/*
 * Reads next char fron current file in buf.
 * - if at block boundary: load next block
 * - if at EOF: returns 2
 * - if error: returns 1
 * - otherwise: returns 0
 */
int fuco_filebuf_next_char(fuco_filebuf_t *buf, char *c);

void fuco_filebuf_skip_nontokens(fuco_filebuf_t *buf);

#endif
