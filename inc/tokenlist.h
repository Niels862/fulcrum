#ifndef FUCO_TOKENLIST_H
#define FUCO_TOKENLIST_H

#include "token.h"

typedef fuco_token_t *fuco_tstream_t;

#define FUCO_TOKENLIST_INIT_SIZE 256

/* Only used during lexing, afterwards only the token pointer terminated by 
   FUCO_TOKEN_END_OF_SOURCE remains */
typedef struct {
   /* Note: no invalid token pointers on reallocation if handled properly, as 
      they are only valid until next append */
    fuco_token_t *tokens;
    size_t size;
    size_t cap;
} fuco_tokenlist_t;

#define FUCO_PARTIAL_TSTREAM(list) (fuco_tstream_t)((list)->tokens)

void fuco_tokenlist_init(fuco_tokenlist_t *list);

void fuco_tokenlist_destruct(fuco_tokenlist_t *list);

/* Destructs entire tstream and its tokens starting from any position 
   in tstream */
void fuco_tstream_destruct(fuco_tstream_t tstream);

void fuco_tstream_write(fuco_tstream_t tstream, FILE *file);

/* Returns reference to appended empty token, invalidated at next call */
fuco_token_t *fuco_tokenlist_append(fuco_tokenlist_t *list);

fuco_tstream_t fuco_tokenlist_terminate(fuco_tokenlist_t *list);

#endif
