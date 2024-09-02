#include "lexer.h"
#include "utils.h"
#include "tokenlist.h"
#include <stdlib.h>
#include <stdio.h>
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

bool fuco_is_number_start(int c) {
    return isdigit(c);
}

bool fuco_is_number_continue(int c) {
    return isdigit(c);
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

size_t fuco_filebuf_read(fuco_filebuf_t *filebuf, FILE *file) {
    filebuf->i = 0;
    filebuf->size = fread(filebuf->data, 1, FUCO_LEXER_FILE_BUF_SIZE, file);
    filebuf->eof = (filebuf->size == 0);

    return filebuf->size;
}

void fuco_lexer_init(fuco_lexer_t *lexer, char *filename) {
    lexer->filebuf.i = lexer->filebuf.size = 0;
    lexer->filebuf.eof = true; /* Start at EOF -> 'next' file opened */

    fuco_strbuf_init(&lexer->strbuf);
    fuco_tokenlist_init(&lexer->list);
    fuco_textsource_init(&lexer->curr, NULL);

    lexer->file = NULL;
    lexer->filename = filename;
    lexer->c = '\0';
}

void fuco_lexer_destruct(fuco_lexer_t *lexer) {
    fuco_strbuf_destruct(&lexer->strbuf);
    fuco_tokenlist_destruct(&lexer->list);

    if (lexer->file != NULL) {
        fclose(lexer->file);
    }
}

int fuco_lexer_open_next_file(fuco_lexer_t *lexer) {
    assert(lexer->filename != NULL); /* Should be checked by caller */
    
    if (lexer->file != NULL) {
        fclose(lexer->file);
    }

    lexer->file = fopen(lexer->filename, "r");

    if (lexer->file == NULL) {
        fuco_syntax_error(NULL, "could not open file: %s", 
                          lexer->filename);
        return 1;
    }

    fuco_textsource_init(&lexer->curr, lexer->filename);

    lexer->filename = NULL;

    lexer->filebuf.i = lexer->filebuf.size = 0;
    lexer->filebuf.eof = false;

    fuco_lexer_next_char(lexer);

    return 0;
}

void fuco_lexer_next_char(fuco_lexer_t *lexer) {
    assert(!lexer->filebuf.eof); /* Handled by caller */

    if (lexer->filebuf.size != 0) {
        lexer->filebuf.i++;
    }

    if (lexer->filebuf.i >= lexer->filebuf.size) {
        fuco_filebuf_read(&lexer->filebuf, lexer->file);
    }

    fuco_textsource_next_char(&lexer->curr, lexer->c);

    if (lexer->filebuf.eof) {
        lexer->c = -1;
    } else {
        lexer->c = (unsigned char)lexer->filebuf.data[lexer->filebuf.i];
    }
}

void fuco_lexer_skip_nontokens(fuco_lexer_t *lexer) {
    bool comment = false;
    
    while (lexer->c != -1) {
        if (lexer->c == '#') {
            comment = true;
        } else if (lexer->c == '\n') {
            comment = false;
        } else if (!(comment && isprint(lexer->c)) 
                   && !fuco_is_nontoken(lexer->c)) {
            return;
        }

        fuco_lexer_next_char(lexer);
    }
}

void fuco_lexer_append_token(fuco_lexer_t *lexer, fuco_tokentype_t type, char *lexeme, void *data) {
    fuco_token_t *token = fuco_tokenlist_append(&lexer->list);
    
    token->type = type;
    token->lexeme = lexeme;
    token->data = data;
    token->source = lexer->start;
}

fuco_tstream_t fuco_lexer_lex(fuco_lexer_t *lexer) {
    if (fuco_lexer_open_next_file(lexer)) {
        return NULL;
    }

    fuco_tokentype_t type;
    char *lexeme;
    void *data;

    while (true) {
        type = FUCO_TOKEN_EMPTY;

        fuco_lexer_skip_nontokens(lexer);

        lexer->start = lexer->curr;
        fuco_strbuf_clear(&lexer->strbuf);

        if (fuco_is_identifier_start(lexer->c)) {
            type = FUCO_TOKEN_IDENTIFIER;

            do {
                fuco_strbuf_append_char(&lexer->strbuf, lexer->c);
                fuco_lexer_next_char(lexer);
            } while (fuco_is_identifier_continue(lexer->c));

            type = fuco_tokentype_lookup_string(lexer->strbuf.data, 
                                                FUCO_TOKENKIND_KEYWORD);

            if (type == FUCO_TOKEN_EMPTY) {
                type = FUCO_TOKEN_IDENTIFIER;
                lexeme = fuco_strbuf_dup(&lexer->strbuf);
            } else {
                lexeme = NULL;
            }
            
            fuco_lexer_append_token(lexer, type, lexeme, NULL);
        } else if (fuco_is_number_start(lexer->c)) {
            do {
                fuco_strbuf_append_char(&lexer->strbuf, lexer->c);
                fuco_lexer_next_char(lexer);
            } while (fuco_is_number_continue(lexer->c));

            data = fuco_parse_integer(lexer->strbuf.data);
            if (data == NULL) {
                return NULL;
            }

            lexeme = fuco_strbuf_dup(&lexer->strbuf);

            fuco_lexer_append_token(lexer, FUCO_TOKEN_INTEGER, lexeme, data);
        } else if (fuco_is_operator(lexer->c)) { /* For now: greedy operators */
            do {
                fuco_strbuf_append_char(&lexer->strbuf, lexer->c);
                fuco_lexer_next_char(lexer);
            } while (fuco_is_operator(lexer->c));

            type = fuco_tokentype_lookup_string(lexer->strbuf.data, 
                                                FUCO_TOKENKIND_OPERATOR);
            
            if (type == FUCO_TOKEN_EMPTY) {
                fuco_syntax_error(&lexer->start, "invalid operator: '%s'", 
                                  lexer->strbuf.data);
                return NULL;
            }

            fuco_lexer_append_token(lexer, type, NULL, NULL);
        } else if (lexer->c == -1) {
            fuco_lexer_append_token(lexer, FUCO_TOKEN_END_OF_FILE, NULL, NULL);

            if (lexer->filename == NULL) {
                return fuco_tokenlist_terminate(&lexer->list);
            } else {
                if (fuco_lexer_open_next_file(lexer)) {
                    return NULL;
                }
            }
        } else {
            fuco_strbuf_append_char(&lexer->strbuf, lexer->c);

            fuco_lexer_next_char(lexer);

            type = fuco_tokentype_lookup_string(lexer->strbuf.data, 
                                                FUCO_TOKENKIND_SEPARATOR);

            if (type == FUCO_TOKEN_EMPTY) {
                fuco_syntax_error(&lexer->start, "invalid character: '%s'", 
                                  fuco_repr_char(lexer->strbuf.data[0]));
                return NULL;
            }

            fuco_lexer_append_token(lexer, type, NULL, NULL);
        }
    }
}
