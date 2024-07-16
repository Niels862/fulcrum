#ifndef FUCO_TOKENLIST_H
#define FUCO_TOKENLIST_H

#include "token.h"

#define FUCO_TOKENLIST_INIT_SIZE 256

/* Only used during lexing, afterwards only the token pointer terminated by 
   FUCO_TOKEN_END_OF_SOURCE remains */
typedef struct {
    fuco_token_t *tokens;
    size_t size;
    size_t cap;
} fuco_tokenlist_t;

void fuco_tokenlist_init(fuco_tokenlist_t *list);

void fuco_tokenlist_destruct(fuco_tokenlist_t *list);

void fuco_tokenlist_write(fuco_token_t *tokens, FILE *file);

/* Returns reference to appended empty token, invalidated at next call */
fuco_token_t *fuco_tokenlist_append(fuco_tokenlist_t *list);

fuco_token_t *fuco_tokenlist_terminate(fuco_tokenlist_t *list);

#endif
