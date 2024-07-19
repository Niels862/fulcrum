#include "lexer.h"
#include "utils.h"
#include "tokenlist.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

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

void fuco_lexer_append_token(fuco_lexer_t *lexer, fuco_tokentype_t type, char *lexeme, void *data) {
    assert((token_descriptors[type].attr & FUCO_TOKENTYPE_HAS_LEXEME) == 0 
           || lexeme != NULL);
    
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
        fuco_textsource_write(&lexer->curr, stderr);
        fprintf(stderr, "-> '%s'\n", fuco_repr_char(lexer->c));

        lexer->start = lexer->curr;
        fuco_strbuf_clear(&lexer->strbuf);

        if (isalpha(lexer->c) || lexer->c == '_') {
            type = FUCO_TOKEN_IDENTIFIER;

            do {
                fuco_strbuf_append_char(&lexer->strbuf, lexer->c);
                fuco_lexer_next_char(lexer);
            } while (isalpha(lexer->c) || isdigit(lexer->c) || lexer->c == '_');

            for (size_t i = 0; i < fuco_n_tokentypes(); i++) {
                if (token_descriptors[i].attr & FUCO_TOKENTYPE_IS_KEYWORD) {
                    char *keyword = token_descriptors[i].string;
                    if (strcmp(lexer->strbuf.data, keyword) == 0) {
                        type = i;
                        break;
                    }
                }
            }

            if (type == FUCO_TOKEN_IDENTIFIER) {
                lexeme = fuco_strbuf_dup(&lexer->strbuf);
            } else {
                lexeme = NULL;
            }
            
            fuco_lexer_append_token(lexer, type, lexeme, NULL);
        } else if (isdigit(lexer->c)) {
            do {
                fuco_strbuf_append_char(&lexer->strbuf, lexer->c);
                fuco_lexer_next_char(lexer);
            } while (isdigit(lexer->c));

            data = fuco_parse_integer(lexer->strbuf.data);
            if (data == NULL) {
                return NULL;
            }

            lexeme = fuco_strbuf_dup(&lexer->strbuf);

            fuco_lexer_append_token(lexer, FUCO_TOKEN_INTEGER, lexeme, data);
        } else if (lexer->c == -1) {
            fuco_lexer_append_token(lexer, FUCO_TOKEN_END_OF_FILE, NULL, NULL);

            if (lexer->filename == NULL) {
                return fuco_tokenlist_terminate(&lexer->list);
            } else {
                if (fuco_lexer_open_next_file(lexer)) {
                    return NULL;
                }
            }
        }

        fuco_lexer_next_char(lexer);
    }
}
