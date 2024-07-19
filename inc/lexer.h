#ifndef FUCO_LEXER_H
#define FUCO_LEXER_H

#include "tokenlist.h"
#include "textsource.h"
#include <stdbool.h>

#define FUCO_LEXER_FILE_BUF_SIZE 1024

typedef struct {
    char data[FUCO_LEXER_FILE_BUF_SIZE];
    size_t i;
    size_t size;
    bool eof;
} fuco_filebuf_t;

typedef struct {
    fuco_filebuf_t filebuf;
    fuco_tokenlist_t list;
    fuco_textsource_t source;
    FILE *file;
    char *filename; /* Will be replaced by a queue of files (jobs) later */
    int c;
} fuco_lexer_t;

size_t fuco_filebuf_read(fuco_filebuf_t *filebuf, FILE *file);

void fuco_lexer_init(fuco_lexer_t *lexer, char *filename);

void fuco_lexer_destruct(fuco_lexer_t *lexer);

int fuco_lexer_open_next_file(fuco_lexer_t *lexer);

void fuco_lexer_next_char(fuco_lexer_t *lexer);

void fuco_lexer_append_token(fuco_lexer_t *lexer);

fuco_tstream_t fuco_lexer_lex(fuco_lexer_t *lexer);

#endif
