#include "tokenizer.h"
#include <stdlib.h>

char *fuco_tokentype_string(fuco_tokentype_t type) {
    static char *strs[] = {
        "null",
        "integer",
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
    token->type = FUCO_TOKEN_EOF;
}

void fuco_token_destruct(fuco_token_t *token) {
    if (token->lexeme != NULL) {
        free(token->lexeme);
    }
}

void fuco_token_write(fuco_token_t *token, FILE *stream) {
    fuco_textsource_write(&token->source, stream);
    fprintf(stream, ": '%s'", fuco_tokentype_string(token->type));
    if (token->lexeme != NULL) {
        fprintf(stream, ": %s", token->lexeme);
    }
}

void fuco_tokenizer_init(fuco_tokenizer_t *tokenizer) {
    tokenizer->file = NULL;
    tokenizer->filename = NULL;
    tokenizer->last = FUCO_TOKEN_EOF;
    fuco_queue_init(&tokenizer->source_filenames);
    fuco_token_init(&tokenizer->curr);
    fuco_strbuf_init(&tokenizer->temp);
}

void fuco_tokenizer_destruct(fuco_tokenizer_t *tokenizer) {
    if (tokenizer->file != NULL) {
        fclose(tokenizer->file);
    }
    fuco_queue_destruct(&tokenizer->source_filenames, free);
    fuco_token_destruct(&tokenizer->curr);
    fuco_strbuf_destruct(&tokenizer->temp);
}

void fuco_tokenizer_add_source_filename(fuco_tokenizer_t *tokenizer, 
                                        char *filename) {
    /* TODO: search absolute path here */
    fuco_queue_enqueue(&tokenizer->source_filenames, filename);
}

int fuco_tokenizer_open_next_source(fuco_tokenizer_t *tokenizer) {
    if (tokenizer->last != FUCO_TOKEN_EOF) {
        abort();
    }
    
    if (tokenizer->file != NULL) {
        fclose(tokenizer->file);
        tokenizer->file = NULL;
    }

    tokenizer->filename = fuco_queue_dequeue(&tokenizer->source_filenames);
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

void fuco_tokenizer_update_filebuf(fuco_tokenizer_t *tokenizer, fuco_filebuf_t *buf) {
    if (tokenizer->file != buf->file) {
        buf->file = tokenizer->file;
        buf->p = buf->size = 0;
        fuco_textsource_init(&buf->source, tokenizer->filename);
    }
}

void fuco_tokenizer_next_token(fuco_tokenizer_t *tokenizer) {
    static fuco_filebuf_t buf = {
        .file = NULL,
        .p = 0,
        .size = 0
    };

    tokenizer->last = tokenizer->curr.type;

    fuco_tokenizer_update_filebuf(tokenizer, &buf);

    fuco_filebuf_skip_nontokens(&buf);

    tokenizer->curr.source = buf.source;

    if (buf.size == 0) {
        tokenizer->curr.type = FUCO_TOKEN_EOF;
    }

    fuco_token_write(&tokenizer->curr, stderr);
    printf("\n");
}

int fuco_filebuf_next_char(fuco_filebuf_t *buf, char *c) {
    if (buf->p >= buf->size) {
        buf->size = fread(buf->data, 1, FUCO_FILEBUF_SIZE, buf->file);
        buf->p = 0;
    }

    if (*c == '\n') {
        buf->source.row++;
        buf->source.col = 1;
    } else {
        buf->source.col++;
    }

    if (buf->size == 0) {
        return 2;
    }
    
    *c = buf->data[buf->p];
    buf->p++;

    return 0;
}

void fuco_filebuf_skip_nontokens(fuco_filebuf_t *buf) {
    char c = '\0';
    int res;
    bool comment = false;
    while ((res = fuco_filebuf_next_char(buf, &c)) != 2) {
        if (c == '#') {
            comment = true;
        } else if (c == '\n') {
            comment = false;
        } else if (!comment 
                   && !(c == ' ' || c == '\n' || c == '\r' || c == '\t')) {
            return;
        }
    }
}
