#ifndef FUCO_LEXER_H
#define FUCO_LEXER_H

#define _POSIX_C_SOURCE 200809L

#include "tokenlist.h"
#include "textsource.h"
#include "strutils.h"
#include "queue.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

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
    fuco_queue_t jobs;
    FILE *file;
    int c;
} fuco_lexer_t;

bool fuco_is_nontoken(int c);

bool fuco_is_identifier_start(int c);

bool fuco_is_identifier_continue(int c);

bool fuco_is_number_start(int c);

bool fuco_is_number_continue(int c);

bool fuco_is_operator(int c);

uint64_t *fuco_parse_integer(char *lexeme);

size_t fuco_filebuf_read(fuco_filebuf_t *filebuf, FILE *file);

void fuco_lexer_init(fuco_lexer_t *lexer);

void fuco_lexer_destruct(fuco_lexer_t *lexer);

void fuco_lexer_add_job(fuco_lexer_t *lexer, char *filename);

int fuco_lexer_open_next_file(fuco_lexer_t *lexer);

void fuco_lexer_next_char(fuco_lexer_t *lexer);

void fuco_lexer_skip_nontokens(fuco_lexer_t *lexer);

void fuco_lexer_append_token(fuco_lexer_t *lexer, fuco_tokentype_t type, 
                             char *lexeme, void *data);

fuco_tstream_t fuco_lexer_lex(fuco_lexer_t *lexer);

#endif
