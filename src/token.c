#include "token.h"
#include "utils.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

fuco_token_t null_token = {
    .lexeme = "(null)", 
    .source = {
        .col = 0, .row = 0, .filename = NULL
    }, 
    .type = FUCO_TOKEN_EMPTY
};

fuco_token_t int_token = {
    .lexeme = "Int",
    .source = {
        .col = 0, .row = 0, .filename = NULL
    },
    .type = FUCO_TOKEN_IDENTIFIER
};

fuco_token_t float_token = {
    .lexeme = "Float",
    .source = {
        .col = 0, .row = 0, .filename = NULL
    },
    .type = FUCO_TOKEN_IDENTIFIER
};

fuco_token_t bool_token = {
    .lexeme = "Bool",
    .source = {
        .col = 0, .row = 0, .filename = NULL
    },
    .type = FUCO_TOKEN_IDENTIFIER
};

fuco_token_t none_token = {
    .lexeme = "None",
    .source = {
        .col = 0, .row = 0, .filename = NULL
    },
    .type = FUCO_TOKEN_IDENTIFIER
};

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

    { FUCO_TOKEN_ARROW, FUCO_TOKENTYPE_IS_OPERATOR, "->" },
    { FUCO_TOKEN_PLUS, FUCO_TOKENTYPE_IS_OPERATOR, "+" },
    { FUCO_TOKEN_MINUS, FUCO_TOKENTYPE_IS_OPERATOR, "-" },
    { FUCO_TOKEN_ASTERISK, FUCO_TOKENTYPE_IS_OPERATOR, "*" },
    { FUCO_TOKEN_SLASH, FUCO_TOKENTYPE_IS_OPERATOR, "/" },
    { FUCO_TOKEN_PERCENT, FUCO_TOKENTYPE_IS_OPERATOR, "%" },

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

fuco_tokentype_t fuco_tokentype_lookup_string(char *string, 
                                              fuco_token_attr_t attr_mask) {
    for (size_t i = 0; i < FUCO_N_TOKENTYPES; i++) {
        if ((token_descriptors[i].attr & attr_mask) == attr_mask
            && strcmp(token_descriptors[i].string, string) == 0) {
            return i;
        }
    }

    return FUCO_TOKEN_EMPTY;
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
                FUCO_UNREACHED();
        }
    }
}

char *fuco_token_string(fuco_token_t *token) {
    if (token_descriptors[token->type].attr & FUCO_TOKENTYPE_HAS_LEXEME) {
        return token->lexeme;
    }   
    return fuco_tokentype_string(token->type);
}

char *fuco_token_static_string(fuco_token_t *token) {
    static char *strs[16];
    static size_t idx = 0;

    char **str = &strs[idx];
    *str = fuco_token_string(token);

    idx = (idx + 1) % 16;

    return *str;
}
