#include "token.h"
#include <stdlib.h>
#include <stdint.h>

fuco_token_descriptor_t const token_descriptors[] = {
    { FUCO_TOKEN_EMPTY, 0, "empty" },

    { FUCO_TOKEN_INTEGER, FUCO_TOKENTYPE_HAS_LEXEME, "integer" },
    { FUCO_TOKEN_IDENTIFIER, FUCO_TOKENTYPE_HAS_LEXEME, "identifier" },
    
    { FUCO_TOKEN_DEF, FUCO_TOKENTYPE_IS_KEYWORD, "def" },
    { FUCO_TOKEN_RETURN, FUCO_TOKENTYPE_IS_KEYWORD, "return" },

    { FUCO_TOKEN_BRACKET_OPEN, FUCO_TOKENTYPE_IS_SEPARATOR, "(" },
    { FUCO_TOKEN_BRACKET_CLOSE, FUCO_TOKENTYPE_IS_SEPARATOR, ")" },
    { FUCO_TOKEN_BRACE_OPEN, FUCO_TOKENTYPE_IS_SEPARATOR, "{" },
    { FUCO_TOKEN_BRACE_CLOSE, FUCO_TOKENTYPE_IS_SEPARATOR, "}" },
    { FUCO_TOKEN_SQBRACKET_OPEN, FUCO_TOKENTYPE_IS_SEPARATOR, "[" },
    { FUCO_TOKEN_SQBRACKET_CLOSE, FUCO_TOKENTYPE_IS_SEPARATOR, "]" },
    { FUCO_TOKEN_DOT, FUCO_TOKENTYPE_IS_SEPARATOR, "." },
    { FUCO_TOKEN_COMMA, FUCO_TOKENTYPE_IS_SEPARATOR, "," },
    { FUCO_TOKEN_SEMICOLON, FUCO_TOKENTYPE_IS_SEPARATOR, ";" },
    
    { FUCO_TOKEN_EOF, 0, "eof" }
};

#define FUCO_N_TOKENTYPES sizeof(token_descriptors) / sizeof(*token_descriptors)

char *fuco_tokentype_string(fuco_tokentype_t type) {
    return token_descriptors[type].string;
}

bool fuco_tokentype_has_attr(fuco_tokentype_t type, fuco_token_attr_t attr) {
    return token_descriptors[type].attr & attr;
}

size_t fuco_n_tokentypes() {
    return FUCO_N_TOKENTYPES;
}

void fuco_token_init(fuco_token_t *token) {
    token->lexeme = NULL;
    token->data = NULL;
    fuco_textsource_init(&token->source, NULL);
    token->type = FUCO_TOKEN_EMPTY;
}

void fuco_token_destruct(fuco_token_t *token) {
    if (token->lexeme != NULL) {
        free(token->lexeme);
    }
    if (token->data != NULL) {
        free(token->data);
    }
    token->type = FUCO_TOKEN_EMPTY;
    token->lexeme = NULL;
}

void fuco_token_write(fuco_token_t *token, FILE *stream) {
    fuco_textsource_write(&token->source, stream);
    fprintf(stream, ": %s", fuco_tokentype_string(token->type));
    if (token->lexeme != NULL) {
        fprintf(stream, ": '%s'", token->lexeme);
    }
    if (token->data != NULL) {
        switch (token->type) {
            case FUCO_TOKEN_INTEGER:
                fprintf(stream, " (=%ld)", *(uint64_t *)token->data);
                break;

            default:
                abort();
        }
    }
}
