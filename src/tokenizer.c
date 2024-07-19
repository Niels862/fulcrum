#include "tokenizer.h"
#include "tree.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

bool fuco_is_nontoken(int c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

bool fuco_is_identifier_start(int c) {
    return isalpha(c) || c == '_';
}

bool fuco_is_identifier_continue(int c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

bool fuco_is_operator(int c) {
    return c == '-' || c == '+' || c == '*' || c == '/'
        || c == '%' || c == '!' || c == '^' || c == '&'
        || c == '~' || c == '|' || c == '<' || c == '>'
        || c == '=';
}

uint64_t *fuco_parse_integer(char *lexeme) {
    uint64_t data = 0;

    while (*lexeme != '\0') {
        assert(isdigit(*lexeme));

        data = 10 * data + *lexeme - '0';

        lexeme++;
    }

    uint64_t *p = malloc(sizeof(uint64_t));
    *p = data;

    return p;
}

void fuco_filebuf_init(fuco_filebuf_t_depr *buf) {
    buf->file = NULL;
    buf->p = buf->size = 0;
    buf->last = '\0';
    fuco_textsource_init(&buf->source, NULL);
}

void fuco_tokenizer_init(fuco_tokenizer_t *tokenizer) {
    tokenizer->file = NULL;
    tokenizer->filename = NULL;
    tokenizer->last = FUCO_TOKEN_END_OF_FILE;

    fuco_filebuf_init(&tokenizer->buf);
    fuco_queue_init(&tokenizer->sources);
    fuco_token_init(&tokenizer->curr);
    fuco_strbuf_init(&tokenizer->temp);

    tokenizer->curr.type = FUCO_TOKEN_END_OF_FILE;

    for (size_t i = 0; i < fuco_n_tokentypes(); i++) {
        assert(i == token_descriptors[i].type);
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

int fuco_tokenizer_next_token(fuco_tokenizer_t *tokenizer) {
    assert(tokenizer->curr.type == FUCO_TOKEN_EMPTY);

    fuco_token_t *curr = &tokenizer->curr;
    fuco_strbuf_clear(&tokenizer->temp);

    fuco_tokenizer_update_filebuf(tokenizer);

    int c = tokenizer->buf.last;
    c = fuco_tokenizer_skip_nontokens(tokenizer, c);
    curr->source = tokenizer->buf.source;

    if (c < 0) {
        curr->type = FUCO_TOKEN_END_OF_FILE;
    } else if (isdigit(c)) {
        curr->type = FUCO_TOKEN_INTEGER;

        do {
            fuco_strbuf_append_char(&tokenizer->temp, c);
            c = fuco_tokenizer_next_char(tokenizer, c);
        } while (isdigit(c));
    } else if (fuco_is_identifier_start(c)) {
        curr->type = FUCO_TOKEN_IDENTIFIER;

        do {
            fuco_strbuf_append_char(&tokenizer->temp, c);
            c = fuco_tokenizer_next_char(tokenizer, c);
        } while (fuco_is_identifier_continue(c));

        for (fuco_tokentype_t type = 0; type < fuco_n_tokentypes(); type++) {
            if (fuco_tokentype_has_attr(type, FUCO_TOKENTYPE_IS_KEYWORD)
                && strcmp(fuco_tokentype_string(type), 
                          tokenizer->temp.data) == 0) {
                tokenizer->curr.type = type;
                break;
            }
        }
    } else if (fuco_is_operator(c)) {
        curr->type = FUCO_TOKEN_OPERATOR;

        do {
            fuco_strbuf_append_char(&tokenizer->temp, c);
            c = fuco_tokenizer_next_char(tokenizer, c);
        } while (fuco_is_operator(c));
    } else {
        for (fuco_tokentype_t type = 0; type < fuco_n_tokentypes(); type++) {
            if (fuco_tokentype_has_attr(type, FUCO_TOKENTYPE_IS_SEPARATOR) 
                && fuco_tokentype_string(type)[0] == c) {
                c = fuco_tokenizer_next_char(tokenizer, c);
                curr->type = type;
                break;
            }
        }
    }

    if (curr->type == FUCO_TOKEN_EMPTY) {
        fuco_syntax_error(&curr->source, "unrecognized token: '%s'", 
                          fuco_repr_char(c));
        
        return 1;
    }

    if (fuco_tokentype_has_attr(curr->type, 
                                FUCO_TOKENTYPE_HAS_LEXEME)) {
        curr->lexeme = fuco_strbuf_dup(&tokenizer->temp);

        switch (curr->type) {
            case FUCO_TOKEN_INTEGER:
                curr->data = fuco_parse_integer(curr->lexeme);
                break;

            default:
                break;
        }
    } else {
        curr->lexeme = NULL;
    }

    tokenizer->buf.last = c;

    return 0;
}

int fuco_tokenizer_next_char(fuco_tokenizer_t *tokenizer, int c) {
    fuco_filebuf_t_depr *buf = &tokenizer->buf;
    
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

int fuco_tokenizer_skip_nontokens(fuco_tokenizer_t *tokenizer, int c) {
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
        fuco_syntax_error(&tokenizer->curr.source, "expected %s, but got %s", 
                          fuco_tokentype_string(type), 
                          fuco_token_string(&tokenizer->curr));

        return false;
    }
    return true;
}

void fuco_tokenizer_handle_curr(fuco_tokenizer_t *tokenizer) {
    assert(tokenizer->curr.type != FUCO_TOKEN_EMPTY);

    tokenizer->last = tokenizer->curr.type;
}

int fuco_tokenizer_discard(fuco_tokenizer_t *tokenizer) {
    fuco_tokenizer_handle_curr(tokenizer);

    fuco_token_destruct(&tokenizer->curr);

    if (fuco_tokenizer_next_token(tokenizer)) {
        return 1;
    }

    return 0;
}

int fuco_tokenizer_move(fuco_tokenizer_t *tokenizer, fuco_node_t *node) {
    fuco_token_t *token = &tokenizer->curr;
    fuco_tokenizer_handle_curr(tokenizer);

    node->token = *token;

    token->lexeme = NULL;
    token->data = NULL;
    fuco_token_destruct(token);

    if (fuco_tokenizer_next_token(tokenizer)) {
        return 1;
    }

    return 0;
}

int fuco_tokenizer_open_next_source(fuco_tokenizer_t *tokenizer) {
    assert(tokenizer->curr.type == FUCO_TOKEN_END_OF_FILE);

    fuco_tokenizer_handle_curr(tokenizer);

    if (fuco_queue_is_empty(&tokenizer->sources)) {
        return 1;
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
    
    if (fuco_tokenizer_next_token(tokenizer)) {
        return 1;
    }

    return 0;
}

bool fuco_tokenizer_discard_if(fuco_tokenizer_t *tokenizer, 
                               fuco_tokentype_t type) {
    if (!fuco_tokenizer_expect(tokenizer, type)) {
        return false;
    }

    if (fuco_tokenizer_discard(tokenizer)) {
        return false;
    }

    return true;
}

bool fuco_tokenizer_move_if(fuco_tokenizer_t *tokenizer, 
                            fuco_tokentype_t type, fuco_node_t *node) {
    if (!fuco_tokenizer_expect(tokenizer, type)) {
        return false;
    }

    if (fuco_tokenizer_move(tokenizer, node)) {
        return false;
    }

    return true;
}

bool fuco_tokenizer_discard_operator_if(fuco_tokenizer_t *tokenizer, 
                                        fuco_tokentype_t type) {
    if (tokenizer->curr.type != FUCO_TOKEN_OPERATOR) {
        fuco_syntax_error(&tokenizer->curr.source, "expected %s, but got %s", 
                          fuco_tokentype_string(type), 
                          fuco_token_string(&tokenizer->curr));

        return false;
    }

    char *operator = token_descriptors[type].string;
    char *lexeme = tokenizer->curr.lexeme;

    size_t i = 0;
    while (operator[i] == lexeme[i] && operator[i] != '\0') {
        i++;
    }

    if (operator[i] == '\0') {
        if (lexeme[i] == '\0') {
            fuco_tokenizer_discard(tokenizer);
        } else {
            size_t j;
            for (j = i; lexeme[j] != '\0'; j++) {
                lexeme[j - i] = lexeme[j];
            }
            
            lexeme[j - i] = '\0';
        }

        tokenizer->last = type;

        return true;
    } else {
        /* Will fail, used for syntax_error message */
        fuco_tokenizer_expect(tokenizer, type);

        return false;
    }
}
