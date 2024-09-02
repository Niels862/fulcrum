#ifndef FUCO_TOKEN_H
#define FUCO_TOKEN_H

#include "textsource.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    FUCO_TOKENKIND_SYNTHETIC,
    FUCO_TOKENKIND_LITERAL,
    FUCO_TOKENKIND_KEYWORD,
    FUCO_TOKENKIND_SEPARATOR,
    FUCO_TOKENKIND_OPERATOR
} fuco_tokenkind_t;

typedef enum {
    FUCO_TOKEN_NULL,
    FUCO_TOKEN_EMPTY,
    FUCO_TOKEN_START_OF_SOURCE,
    FUCO_TOKEN_END_OF_FILE,
    FUCO_TOKEN_END_OF_SOURCE, /* Final token in tokenlist */

    FUCO_TOKEN_INTEGER,
    FUCO_TOKEN_IDENTIFIER,
    FUCO_TOKEN_OPERATOR,

    FUCO_TOKEN_DEF,
    FUCO_TOKEN_INLINE,
    FUCO_TOKEN_RETURN,
    FUCO_TOKEN_CONVERT,
    FUCO_TOKEN_WHILE,
    FUCO_TOKEN_FOR,
    FUCO_TOKEN_IF,
    FUCO_TOKEN_ELSE,
    FUCO_TOKEN_LET,
    FUCO_TOKEN_CONST,

    FUCO_TOKEN_BRACKET_OPEN,
    FUCO_TOKEN_BRACKET_CLOSE,
    FUCO_TOKEN_BRACE_OPEN,
    FUCO_TOKEN_BRACE_CLOSE,
    FUCO_TOKEN_SQBRACKET_OPEN,
    FUCO_TOKEN_SQBRACKET_CLOSE,
    FUCO_TOKEN_DOT,
    FUCO_TOKEN_COMMA,
    FUCO_TOKEN_SEMICOLON,
    FUCO_TOKEN_COLON,

    FUCO_TOKEN_ARROW,
    FUCO_TOKEN_PLUS,
    FUCO_TOKEN_MINUS,
    FUCO_TOKEN_ASTERISK,
    FUCO_TOKEN_SLASH,
    FUCO_TOKEN_PERCENT,
    FUCO_TOKEN_EQ,
    FUCO_TOKEN_NE,
    FUCO_TOKEN_GT,
    FUCO_TOKEN_GE,
    FUCO_TOKEN_LT,
    FUCO_TOKEN_LE,

    FUCO_N_TOKENTYPES
} fuco_tokentype_t;

typedef struct {
    char *lexeme;
    void *data;
    fuco_textsource_t source;
    fuco_tokentype_t type;
} fuco_token_t;

extern fuco_token_t null_token;

extern fuco_token_t int_token;

extern fuco_token_t float_token;

extern fuco_token_t bool_token;

extern fuco_token_t none_token;

fuco_tokenkind_t fuco_tokentype_kind(fuco_tokentype_t type);

char *fuco_tokentype_string(fuco_tokentype_t type);

fuco_tokentype_t fuco_tokentype_lookup_string(char *lexeme, 
                                              fuco_tokenkind_t kind);

void fuco_token_init(fuco_token_t *token);

void fuco_token_destruct(fuco_token_t *token);

void fuco_token_write(fuco_token_t *token, FILE *file);

char *fuco_token_string(fuco_token_t *token);

/* Static rotation: max. 16 uses at once */
char *fuco_token_static_string(fuco_token_t *token);

#endif
