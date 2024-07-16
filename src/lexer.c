#include "lexer.h"
#include "utils.h"
#include "tokenlist.h"
#include <stdlib.h>
#include <stdio.h>

fuco_token_t *fuco_lexer_lex(char *filename) {
    char buf[FUCO_LEXER_FILE_BUF_SIZE];
    size_t size;

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        fuco_syntax_error(NULL, "could not open file: %s", filename);
    }

    fuco_tokenlist_t list;
    fuco_tokenlist_init(&list);    

    while ((size = fread(buf, 1, FUCO_LEXER_FILE_BUF_SIZE, file)) > 0) {
        /* ... */
    }

    fclose(file);

    return fuco_tokenlist_terminate(&list);
}
