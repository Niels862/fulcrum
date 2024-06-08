#ifndef FUCO_TOKENIZER_H
#define FUCO_TOKENIZER_H

#include "queue.h"
#include <stddef.h>
#include <stdio.h>
#include "strutils.h"

typedef struct fuco_node_t fuco_node_t;

typedef enum {
    FUCO_TOKEN_EMPTY,
    FUCO_TOKEN_INTEGER,
    FUCO_TOKEN_IDENTIFIER,
    
    FUCO_TOKEN_DEF,
    FUCO_TOKEN_RETURN,

    FUCO_TOKEN_BRACKET_OPEN,
    FUCO_TOKEN_BRACKET_CLOSE,
    FUCO_TOKEN_BRACE_OPEN,
    FUCO_TOKEN_BRACE_CLOSE,
    FUCO_TOKEN_SQBRACKET_OPEN,
    FUCO_TOKEN_SQBRACKET_CLOSE,
    FUCO_TOKEN_DOT,
    FUCO_TOKEN_COMMA,
    FUCO_TOKEN_SEMICOLON,
    
    FUCO_TOKEN_EOF
} fuco_tokentype_t;

typedef enum {
    FUCO_TOKENTYPE_IS_KEYWORD = 1,
    FUCO_TOKENTYPE_IS_SEPARATOR = 2,
    FUCO_TOKENTYPE_HAS_LEXEME = 4
} fuco_token_attr_t;

typedef struct {
    fuco_tokentype_t type;
    fuco_token_attr_t attr;
    char *string;
} fuco_token_descriptor_t;

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

#define FUCO_FILEBUF_SIZE 1024

/* file is an unowned reference to determine data validity */
typedef struct {
    fuco_textsource_t source;
    FILE *file;
    size_t p;
    size_t size;
    int last;
    char data[FUCO_FILEBUF_SIZE];
} fuco_filebuf_t;

typedef struct {
    FILE *file;
    char *filename;
    fuco_tokentype_t last;
    fuco_queue_t sources;
    fuco_token_t curr;
    fuco_strbuf_t temp;
    fuco_filebuf_t buf;
} fuco_tokenizer_t;

extern fuco_token_descriptor_t const token_descriptors[];

bool fuco_is_nontoken(int c);

bool fuco_is_identifier_start(int c);

bool fuco_is_identifier_continue(int c);

char *fuco_tokentype_string(fuco_tokentype_t type);

bool fuco_tokentype_has_attr(fuco_tokentype_t type, fuco_token_attr_t attr);

void fuco_textsource_init(fuco_textsource_t *source, char *filename);

void fuco_textsource_write(fuco_textsource_t *source, FILE *stream);

void fuco_token_init(fuco_token_t *token);

void fuco_token_destruct(fuco_token_t *token);

void fuco_token_write(fuco_token_t *token, FILE *stream);

void fuco_filebuf_init(fuco_filebuf_t *buf);

void fuco_tokenizer_init(fuco_tokenizer_t *tokenizer);

void fuco_tokenizer_destruct(fuco_tokenizer_t *tokenizer);

/* Takes ownership of filename */
void fuco_tokenizer_add_source_filename(fuco_tokenizer_t *tokenizer, 
                                        char *filename);

void fuco_tokenizer_update_filebuf(fuco_tokenizer_t *tokenizer);

int fuco_tokenizer_next_token(fuco_tokenizer_t *tokenizer);

int fuco_tokenizer_next_char(fuco_tokenizer_t *tokenizer, int c);

char fuco_tokenizer_skip_nontokens(fuco_tokenizer_t *tokenizer, int c);

bool fuco_tokenizer_expect(fuco_tokenizer_t *tokenizer, fuco_tokentype_t type);

void fuco_tokenizer_handle_curr(fuco_tokenizer_t *tokenizer);

/* 
 * Each token handler can only be used once per token and loads the next 
 * token afterwards 
 */

/* Token handler: discards current token */
int fuco_tokenizer_discard(fuco_tokenizer_t *tokenizer);

/* Token handler: moves current token to node */
int fuco_tokenizer_move(fuco_tokenizer_t *tokenizer, fuco_node_t *node);

/* Token handler: discards and loads next source. Token must be EOF. */
int fuco_tokenizer_open_next_source(fuco_tokenizer_t *tokenizer);

/* Helper functions */

bool fuco_tokenizer_discard_if(fuco_tokenizer_t *tokenizer, 
                               fuco_tokentype_t type);

bool fuco_tokenizer_move_if(fuco_tokenizer_t *tokenizer, 
                            fuco_tokentype_t type, fuco_node_t *node);

#endif
