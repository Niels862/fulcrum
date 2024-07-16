#include "token.h"
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

fuco_token_descriptor_t const token_descriptors[] = {
    { FUCO_TOKEN_EMPTY, 0, "empty" },

    { FUCO_TOKEN_START_OF_SOURCE, 0, "start of source" },

    { FUCO_TOKEN_INTEGER, FUCO_TOKENTYPE_HAS_LEXEME, "integer" },
    { FUCO_TOKEN_IDENTIFIER, FUCO_TOKENTYPE_HAS_LEXEME, "identifier" },
    { FUCO_TOKEN_OPERATOR, FUCO_TOKENTYPE_HAS_LEXEME, "operator" },

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
    { FUCO_TOKEN_COLON, FUCO_TOKENTYPE_IS_SEPARATOR, ":" },

    { FUCO_TOKEN_ARROW, 0, "->" },

    { FUCO_TOKEN_END_OF_FILE, 0, "end of file" },

    { FUCO_TOKEN_END_OF_SOURCE, 0, "end of source" }
};

#define FUCO_N_TOKENTYPES sizeof(token_descriptors) / sizeof(*token_descriptors)

char *fuco_tokentype_string(fuco_tokentype_t type) {
    assert(type < FUCO_N_TOKENTYPES);

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
    token->data = NULL;
}

void fuco_token_write(fuco_token_t *token, FILE *file) {
    fuco_textsource_write(&token->source, file);
    fprintf(file, ": %s", fuco_tokentype_string(token->type));
    
    if (token->lexeme != NULL) { /* TODO: use HAS_LEXEME */
        fprintf(file, ": '%s'", token->lexeme);
    }
    
    if (token->data != NULL) {
        switch (token->type) {
            case FUCO_TOKEN_INTEGER:
                fprintf(file, " (=%ld)", *(uint64_t *)token->data);
                break;

            default:
                abort();
        }
    }
}

char *fuco_token_string(fuco_token_t *token) {
    static char *strs[16];
    static size_t idx = 0;

    char **str = &strs[idx];

    if (token_descriptors[token->type].attr & FUCO_TOKENTYPE_HAS_LEXEME) {
        *str = token->lexeme;
    } else {
        *str = fuco_tokentype_string(token->type);
    }

    idx = (idx + 1) % 16;

    return *str;
}
