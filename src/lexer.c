#include "lexer.h"
#include "utils.h"
#include "tokenlist.h"
#include <stdlib.h>
#include <stdio.h>
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

    fuco_tokenlist_init(&lexer->list);
    fuco_textsource_init(&lexer->source, NULL);

    lexer->file = NULL;
    lexer->filename = filename;
    lexer->c = '\0';
}

void fuco_lexer_destruct(fuco_lexer_t *lexer) {
    fuco_tokenlist_destruct(&lexer->list);
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

    fuco_textsource_init(&lexer->source, lexer->filename);

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

    fuco_textsource_next_char(&lexer->source, lexer->c);

    if (lexer->filebuf.eof) {
        lexer->c = -1;
    } else {
        lexer->c = (unsigned char)lexer->filebuf.data[lexer->filebuf.i];
    }

    printf("read %ld / %ld...\n", lexer->filebuf.i, lexer->filebuf.size);
}

void fuco_lexer_append_token(fuco_lexer_t *lexer) {
    fuco_token_t *token = fuco_tokenlist_append(&lexer->list);
    
    FUCO_UNUSED(token);

    /* TODO */
}

fuco_tstream_t fuco_lexer_lex(fuco_lexer_t *lexer) {
    fuco_lexer_open_next_file(lexer);

    while (true) {
        fuco_textsource_write(&lexer->source, stderr);
        fprintf(stderr, "-> '%s'\n", fuco_repr_char(lexer->c));

        if (isalpha(lexer->c) || lexer->c == '_') {
            /* TODO identifier */
        } else if (isdigit(lexer->c)) {
            /* TODO number */
        } else if (lexer->c == -1) {
            fuco_lexer_append_token(lexer);

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
