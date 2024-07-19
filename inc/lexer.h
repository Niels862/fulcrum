#ifndef FUCO_LEXER_H
#define FUCO_LEXER_H

#include "tokenlist.h"
#include "textsource.h"
#include "strutils.h"
#include <stdbool.h>

#include "tokenizer.h" /* for parse_integer, remove later */

#define FUCO_LEXER_FILE_BUF_SIZE 1024

typedef struct {
    char data[FUCO_LEXER_FILE_BUF_SIZE];
    size_t i;
    size_t size;
    bool eof;
} fuco_filebuf_t;

typedef struct {
    fuco_filebuf_t filebuf;
    fuco_strbuf_t strbuf;
    fuco_tokenlist_t list;
    fuco_textsource_t start;
    fuco_textsource_t curr;
    FILE *file;
    char *filename; /* Will be replaced by a queue of files (jobs) later */
    int c;
} fuco_lexer_t;

bool fuco_is_number_start(int c);

bool fuco_is_number_continue(int c);

size_t fuco_filebuf_read(fuco_filebuf_t *filebuf, FILE *file);

void fuco_lexer_init(fuco_lexer_t *lexer, char *filename);

void fuco_lexer_destruct(fuco_lexer_t *lexer);

int fuco_lexer_open_next_file(fuco_lexer_t *lexer);

void fuco_lexer_next_char(fuco_lexer_t *lexer);

void fuco_lexer_skip_nontokens(fuco_lexer_t *lexer);

void fuco_lexer_append_token(fuco_lexer_t *lexer, fuco_tokentype_t type, 
                             char *lexeme, void *data);

fuco_tstream_t fuco_lexer_lex(fuco_lexer_t *lexer);

#endif
