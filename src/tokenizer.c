#include "tokenizer.h"
#include "tree.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

fuco_tokentype_descriptor_t const descriptors[] = {
    { FUCO_TOKEN_EMPTY, 0, "empty" },
    { FUCO_TOKEN_INTEGER, FUCO_TOKENTYPE_HAS_LEXEME, "integer" },
    { FUCO_TOKEN_IDENTIFIER, FUCO_TOKENTYPE_HAS_LEXEME, "identifier" },
    { FUCO_TOKEN_DEF, FUCO_TOKENTYPE_IS_KEYWORD, "def" },
    { FUCO_TOKEN_BRACKET_OPEN, FUCO_TOKENTYPE_IS_SEPARATOR, "(" },
    { FUCO_TOKEN_BRACKET_CLOSE, FUCO_TOKENTYPE_IS_SEPARATOR, ")" },
    { FUCO_TOKEN_BRACE_OPEN, FUCO_TOKENTYPE_IS_SEPARATOR, "{" },
    { FUCO_TOKEN_BRACE_CLOSE, FUCO_TOKENTYPE_IS_SEPARATOR, "}" },
    { FUCO_TOKEN_SQBRACKET_OPEN, FUCO_TOKENTYPE_IS_SEPARATOR, "[" },
    { FUCO_TOKEN_SQBRACKET_CLOSE, FUCO_TOKENTYPE_IS_SEPARATOR, "]" },
    { FUCO_TOKEN_DOT, FUCO_TOKENTYPE_IS_SEPARATOR, "." },
    { FUCO_TOKEN_COMMA, FUCO_TOKENTYPE_IS_SEPARATOR, "," },
    { FUCO_TOKEN_EOF, 0, "eof" }
};

#define FUCO_N_TOKENTYPES sizeof(descriptors) / sizeof(*descriptors)

bool fuco_is_nontoken(int c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

bool fuco_is_identifier_start(int c) {
    return isalpha(c) || c == '_';
}

bool fuco_is_identifier_continue(int c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

char *fuco_tokentype_string(fuco_tokentype_t type) {
    return descriptors[type].string;
}

void fuco_textsource_init(fuco_textsource_t *source, char *filename) {
    source->filename = filename;
    source->row = 1;
    source->col = 0;
}

void fuco_textsource_write(fuco_textsource_t *source, FILE *stream) {
    fprintf(stream, "%s:%ld:%ld", source->filename, source->row, source->col);
}

void fuco_token_init(fuco_token_t *token) {
    token->lexeme = NULL;
    fuco_textsource_init(&token->source, NULL);
    token->type = FUCO_TOKEN_EMPTY;
}

void fuco_token_destruct(fuco_token_t *token) {
    if (token->lexeme != NULL) {
        free(token->lexeme);
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
}

void fuco_filebuf_init(fuco_filebuf_t *buf) {
    buf->file = NULL;
    buf->p = buf->size = 0;
    buf->last = '\0';
    fuco_textsource_init(&buf->source, NULL);
}

void fuco_tokenizer_init(fuco_tokenizer_t *tokenizer) {
    tokenizer->file = NULL;
    tokenizer->filename = NULL;
    tokenizer->last = FUCO_TOKEN_EOF;

    fuco_filebuf_init(&tokenizer->buf);
    fuco_queue_init(&tokenizer->sources);
    fuco_token_init(&tokenizer->curr);
    fuco_strbuf_init(&tokenizer->temp);

    tokenizer->curr.type = FUCO_TOKEN_EOF;

    for (size_t i = 0; i < FUCO_N_TOKENTYPES; i++) {
        assert(i == descriptors[i].type);
    }
}

void fuco_tokenizer_destruct(fuco_tokenizer_t *tokenizer) {
    if (tokenizer->file != NULL) {
        fclose(tokenizer->file);
    }
    fuco_queue_destruct(&tokenizer->sources, free);
    fuco_token_destruct(&tokenizer->curr);
    fuco_strbuf_destruct(&tokenizer->temp);
}

void fuco_tokenizer_add_source_filename(fuco_tokenizer_t *tokenizer, 
                                        char *filename) {
    /* TODO: search absolute path here */
    fuco_queue_enqueue(&tokenizer->sources, filename);
}

void fuco_tokenizer_update_filebuf(fuco_tokenizer_t *tokenizer) {    
    if (tokenizer->file != tokenizer->buf.file) {
        tokenizer->buf.file = tokenizer->file;
        tokenizer->buf.p = tokenizer->buf.size = 0;

        fuco_textsource_init(&tokenizer->buf.source, tokenizer->filename);

        /* Read first char of file into char cache */
        tokenizer->buf.last = fuco_tokenizer_next_char(tokenizer, '\0');
    }
}

void fuco_tokenizer_next_token(fuco_tokenizer_t *tokenizer) {
    assert(tokenizer->curr.type == FUCO_TOKEN_EMPTY);

    tokenizer->curr.source = tokenizer->buf.source;
    fuco_strbuf_clear(&tokenizer->temp);

    fuco_tokenizer_update_filebuf(tokenizer);

    int c = tokenizer->buf.last;
    c = fuco_tokenizer_skip_nontokens(tokenizer, c);
    tokenizer->curr.source = tokenizer->buf.source;

    if (c < 0) {
        tokenizer->curr.type = FUCO_TOKEN_EOF;
    } else if (isdigit(c)) {
        tokenizer->curr.type = FUCO_TOKEN_INTEGER;

        do {
            fuco_strbuf_append_char(&tokenizer->temp, c);
            c = fuco_tokenizer_next_char(tokenizer, c);
        } while (isdigit(c));
    } else if (fuco_is_identifier_start(c)) {
        tokenizer->curr.type = FUCO_TOKEN_IDENTIFIER;

        do {
            fuco_strbuf_append_char(&tokenizer->temp, c);
            c = fuco_tokenizer_next_char(tokenizer, c);
        } while (fuco_is_identifier_continue(c));

        for (size_t i = 0; i < FUCO_N_TOKENTYPES; i++) {
            if (descriptors[i].attr & FUCO_TOKENTYPE_IS_KEYWORD
                && strcmp(descriptors[i].string, tokenizer->temp.data) == 0) {
                tokenizer->curr.type = descriptors[i].type;
                break;
            }
        }
    } else {
        for (size_t i = 0; i < FUCO_N_TOKENTYPES; i++) {
            if (descriptors[i].attr & FUCO_TOKENTYPE_IS_SEPARATOR 
                && descriptors[i].string[0] == c) {
                c = fuco_tokenizer_next_char(tokenizer, c);
                tokenizer->curr.type = descriptors[i].type;
                break;
            }
        }
    }

    if (tokenizer->curr.type == FUCO_TOKEN_EMPTY) {
        fprintf(stderr, "Error: unrecognized token: '%s'\n", fuco_repr_char(c));
    }

    if (descriptors[tokenizer->curr.type].attr & FUCO_TOKENTYPE_HAS_LEXEME) {
        tokenizer->curr.lexeme = fuco_strbuf_dup(&tokenizer->temp);
    } else {
        tokenizer->curr.lexeme = NULL;
    }

    tokenizer->buf.last = c;
}

int fuco_tokenizer_next_char(fuco_tokenizer_t *tokenizer, int c) {
    fuco_filebuf_t *buf = &tokenizer->buf;
    
    if (buf->p >= buf->size) {
        buf->size = fread(buf->data, 1, FUCO_FILEBUF_SIZE, buf->file);
        buf->p = 0;
    }

    if (c == '\n') {
        buf->source.row++;
        buf->source.col = 1;
    } else {
        buf->source.col++;
    }

    if (buf->size == 0) {
        return -1;
    }

    c = buf->data[buf->p];
    buf->p++;

    return (unsigned char)c;
}

char fuco_tokenizer_skip_nontokens(fuco_tokenizer_t *tokenizer, int c) {
    bool comment = false;
    do {
        if (c == '#') {
            comment = true;
        } else if (c == '\n') {
            comment = false;
        } else if (!comment 
                   && !fuco_is_nontoken(c)) {
            return c;
        }
    } while ((c = fuco_tokenizer_next_char(tokenizer, c)) >= 0);

    return c;
}

bool fuco_tokenizer_expect(fuco_tokenizer_t *tokenizer, fuco_tokentype_t type) {
    if (tokenizer->curr.type != type) {
        fprintf(stderr, "Error: expected %s, but got %s\n", 
                fuco_tokentype_string(type), 
                fuco_tokentype_string(tokenizer->curr.type));
        return false;
    }
    return true;
}

void fuco_tokenizer_handle_curr(fuco_tokenizer_t *tokenizer) {
    assert(tokenizer->curr.type != FUCO_TOKEN_EMPTY);

    tokenizer->last = tokenizer->curr.type;
}

void fuco_tokenizer_discard(fuco_tokenizer_t *tokenizer) {
    fuco_tokenizer_handle_curr(tokenizer);

    fuco_token_destruct(&tokenizer->curr);

    fuco_tokenizer_next_token(tokenizer);
}

void fuco_tokenizer_move(fuco_tokenizer_t *tokenizer, fuco_node_t *node) {
    fuco_token_t *token = &tokenizer->curr;
    fuco_tokenizer_handle_curr(tokenizer);

    node->token = *token;

    token->lexeme = NULL;
    fuco_token_destruct(token);

    fuco_tokenizer_next_token(tokenizer);
}

int fuco_tokenizer_open_next_source(fuco_tokenizer_t *tokenizer) {
    assert(tokenizer->curr.type == FUCO_TOKEN_EOF);

    fuco_tokenizer_handle_curr(tokenizer);

    if (fuco_queue_is_empty(&tokenizer->sources)) {
        return -1;
    }

    if (tokenizer->file != NULL) {
        fclose(tokenizer->file);
        tokenizer->file = NULL;
    }

    tokenizer->filename = fuco_queue_dequeue(&tokenizer->sources);
    if (tokenizer->filename == NULL) {
        return 1;
    }

    tokenizer->file = fopen(tokenizer->filename, "r");
    if (tokenizer->file == NULL) {
        fprintf(stderr, "Error: could not open file: %s\n", 
                tokenizer->filename);
        return 1;
    }

    fuco_token_destruct(&tokenizer->curr);
    
    fuco_tokenizer_next_token(tokenizer);

    return 0;
}
