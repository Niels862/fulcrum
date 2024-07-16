#include "tokenlist.h"
#include <stdlib.h>

void fuco_tokenlist_init(fuco_tokenlist_t *list) {
    list->cap = FUCO_TOKENLIST_INIT_SIZE;
    list->tokens = malloc(list->cap * sizeof(fuco_token_t));
    list->size = 0;
}

void fuco_tokenlist_destruct(fuco_tokenlist_t *list) {
    free(list->tokens);
}

void fuco_tokenlist_write(fuco_token_t *tokens, FILE *file) {
    fuco_token_t *token = tokens;

    fprintf(file, "{\n");

    do {
        fprintf(file, "  ");
        fuco_token_write(token, file);
        fprintf(file, "\n");
    } while (token->type != FUCO_TOKEN_END_OF_SOURCE && token++);

    fprintf(file, "}\n");
}

fuco_token_t *fuco_tokenlist_append(fuco_tokenlist_t *list) {
    if (list->size >= list->cap) {
        list->cap *= 2;
        list->tokens = realloc(list->tokens, list->cap * sizeof(fuco_token_t));
    }

    fuco_token_t *token = &list->tokens[list->size];
    fuco_token_init(token);

    list->size++;

    return token;
}

fuco_token_t *fuco_tokenlist_terminate(fuco_tokenlist_t *list) {
    fuco_token_t *token = fuco_tokenlist_append(list);

    token->type = FUCO_TOKEN_END_OF_SOURCE;

    return list->tokens;
}
