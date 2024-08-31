#include "token.h"
#include "utils.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

fuco_token_t null_token = {
    .lexeme = NULL, 
    .source = {
        .col = 0, .row = 0, .filename = NULL
    }, 
    .type = FUCO_TOKEN_NULL
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

fuco_tokenkind_t fuco_tokentype_kind(fuco_tokentype_t type) {
    switch (type) {
        case FUCO_TOKEN_NULL:
        case FUCO_TOKEN_EMPTY:
        case FUCO_TOKEN_START_OF_SOURCE:
        case FUCO_TOKEN_END_OF_FILE:
        case FUCO_TOKEN_END_OF_SOURCE:
            return FUCO_TOKENKIND_SYNTHETIC;

        case FUCO_TOKEN_INTEGER:
        case FUCO_TOKEN_IDENTIFIER:
        case FUCO_TOKEN_OPERATOR:
            return FUCO_TOKENKIND_LITERAL;

        case FUCO_TOKEN_DEF:
        case FUCO_TOKEN_INLINE:
        case FUCO_TOKEN_RETURN:
        case FUCO_TOKEN_CONVERT:
        case FUCO_TOKEN_WHILE:
        case FUCO_TOKEN_FOR:
        case FUCO_TOKEN_IF:
        case FUCO_TOKEN_ELSE:
        case FUCO_TOKEN_LET:
        case FUCO_TOKEN_CONST:
            return FUCO_TOKENKIND_KEYWORD;

        case FUCO_TOKEN_BRACKET_OPEN:
        case FUCO_TOKEN_BRACKET_CLOSE:
        case FUCO_TOKEN_BRACE_OPEN:
        case FUCO_TOKEN_BRACE_CLOSE:
        case FUCO_TOKEN_SQBRACKET_OPEN:
        case FUCO_TOKEN_SQBRACKET_CLOSE:
        case FUCO_TOKEN_DOT:
        case FUCO_TOKEN_COMMA:
        case FUCO_TOKEN_SEMICOLON:
        case FUCO_TOKEN_COLON:
            return FUCO_TOKENKIND_SEPARATOR;

        case FUCO_TOKEN_ARROW:
        case FUCO_TOKEN_PLUS:
        case FUCO_TOKEN_MINUS:
        case FUCO_TOKEN_ASTERISK:
        case FUCO_TOKEN_SLASH:
        case FUCO_TOKEN_PERCENT:
            return FUCO_TOKENKIND_OPERATOR;

        default:
            break;
    }

    FUCO_UNREACHED();
}

char *fuco_tokentype_string(fuco_tokentype_t type) {
    switch (type) {
        case FUCO_TOKEN_NULL:
            return "(null)";

        case FUCO_TOKEN_EMPTY:
            return "(empty)";

        case FUCO_TOKEN_START_OF_SOURCE:
            return "(start of source)";

        case FUCO_TOKEN_END_OF_FILE:
            return "(end of file)";

        case FUCO_TOKEN_END_OF_SOURCE:
            return "(end of source)";

        case FUCO_TOKEN_INTEGER:
            return "(integer)";

        case FUCO_TOKEN_IDENTIFIER:
            return "(identifier)";
        
        case FUCO_TOKEN_OPERATOR:
            return "(operator)";

        case FUCO_TOKEN_DEF:
            return "def";

        case FUCO_TOKEN_INLINE:
            return "inline";

        case FUCO_TOKEN_RETURN:
            return "return";

        case FUCO_TOKEN_CONVERT:
            return "convert";

        case FUCO_TOKEN_WHILE:
            return "while";

        case FUCO_TOKEN_FOR:
            return "for";

        case FUCO_TOKEN_IF:
            return "if";

        case FUCO_TOKEN_ELSE:
            return "else";

        case FUCO_TOKEN_LET:
            return "let";

        case FUCO_TOKEN_CONST:
            return "const";

        case FUCO_TOKEN_BRACKET_OPEN:
            return "(";

        case FUCO_TOKEN_BRACKET_CLOSE:
            return ")";

        case FUCO_TOKEN_BRACE_OPEN:
            return "{";

        case FUCO_TOKEN_BRACE_CLOSE:
            return "}";

        case FUCO_TOKEN_SQBRACKET_OPEN:
            return "[";

        case FUCO_TOKEN_SQBRACKET_CLOSE:
            return "]";

        case FUCO_TOKEN_DOT:
            return ".";

        case FUCO_TOKEN_COMMA:
            return ",";

        case FUCO_TOKEN_SEMICOLON:
            return ";";

        case FUCO_TOKEN_COLON:
            return ":";

        case FUCO_TOKEN_ARROW:
            return "->";

        case FUCO_TOKEN_PLUS:
            return "+";

        case FUCO_TOKEN_MINUS:
            return "-";

        case FUCO_TOKEN_ASTERISK:
            return "*";

        case FUCO_TOKEN_SLASH:
            return "/";

        case FUCO_TOKEN_PERCENT:
            return "%"; 

        default:
            break;
    }

    FUCO_UNREACHED();
}

fuco_tokentype_t fuco_tokentype_lookup_string(char *lexeme, 
                                              fuco_tokenkind_t kind) {
    for (size_t i = 0; i < FUCO_N_TOKENTYPES; i++) {
        char *str = fuco_tokentype_string(i);

        if (fuco_tokentype_kind(i) == kind && strcmp(str, lexeme) == 0) {
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
    if (fuco_tokentype_kind(token->type) == FUCO_TOKENKIND_LITERAL) {
        assert(token->lexeme != NULL);

        return token->lexeme;
    }   

    printf("%d\n", token->type);

    assert(token->lexeme == NULL);

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
