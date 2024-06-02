#include "tokenizer.h"
#include "assert.h"
#include <stdlib.h>
#include <ctype.h>

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
    static char *strs[] = {
        "null",
        "integer",
        "identifier",
        "eof"
    };

    return strs[type];
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

int fuco_tokenizer_open_next_source(fuco_tokenizer_t *tokenizer) {
    assert(tokenizer->last == FUCO_TOKEN_EOF);
    assert(tokenizer->curr.type == FUCO_TOKEN_EMPTY);

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
    
    return 0;
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

        tokenizer->curr.lexeme = fuco_strbuf_dup(&tokenizer->temp);
    } else if (fuco_is_identifier_start(c)) {
        tokenizer->curr.type = FUCO_TOKEN_IDENTIFIER;

        do {
            fuco_strbuf_append_char(&tokenizer->temp, c);
            c = fuco_tokenizer_next_char(tokenizer, c);
        } while (fuco_is_identifier_continue(c));

        tokenizer->curr.lexeme = fuco_strbuf_dup(&tokenizer->temp);
    } else {
        fprintf(stderr, "Error: unrecognized char: %d\n", c);
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

void fuco_tokenizer_handle_curr(fuco_tokenizer_t *tokenizer) {
    assert(tokenizer->curr.type != FUCO_TOKEN_EMPTY);

    tokenizer->last = tokenizer->curr.type;
}

void fuco_tokenizer_discard(fuco_tokenizer_t *tokenizer) {
    fuco_token_t *token = &tokenizer->curr;
    fuco_tokenizer_handle_curr(tokenizer);

    token->type = FUCO_TOKEN_EMPTY;
    if (token->lexeme != NULL) {
        free(token->lexeme);
        token->lexeme = NULL;
    }
}
