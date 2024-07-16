#ifndef FUCO_TOKEN_H
#define FUCO_TOKEN_H

#include "textsource.h"
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    FUCO_TOKEN_EMPTY,

    FUCO_TOKEN_START_OF_SOURCE,

    FUCO_TOKEN_INTEGER,
    FUCO_TOKEN_IDENTIFIER,
    FUCO_TOKEN_OPERATOR,

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
    FUCO_TOKEN_COLON,

    FUCO_TOKEN_ARROW,

    FUCO_TOKEN_END_OF_FILE,
    FUCO_TOKEN_END_OF_SOURCE /* Final token in tokenlist */
} fuco_tokentype_t;

typedef enum {
    FUCO_TOKENTYPE_IS_KEYWORD = 0x1,
    FUCO_TOKENTYPE_IS_SEPARATOR = 0x2,
    FUCO_TOKENIZER_IS_OPERATOR = 0x4,
    FUCO_TOKENTYPE_HAS_LEXEME = 0x8,
} fuco_token_attr_t;

typedef struct {
    fuco_tokentype_t type;
    fuco_token_attr_t attr;
    char *string;
} fuco_token_descriptor_t;

typedef struct {
    char *lexeme;
    void *data;
    fuco_textsource_t source;
    fuco_tokentype_t type;
} fuco_token_t;

extern fuco_token_descriptor_t const token_descriptors[];

char *fuco_tokentype_string(fuco_tokentype_t type);

bool fuco_tokentype_has_attr(fuco_tokentype_t type, fuco_token_attr_t attr);

size_t fuco_n_tokentypes();

void fuco_token_init(fuco_token_t *token);

void fuco_token_destruct(fuco_token_t *token);

void fuco_token_write(fuco_token_t *token, FILE *file);

/* Static rotation: max. 16 uses at once */
char *fuco_token_string(fuco_token_t *token);

#endif
