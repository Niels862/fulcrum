#ifndef FUCO_TOKENIZER_H
#define FUCO_TOKENIZER_H

#include "strutils.h"
#include "textsource.h"
#include "queue.h"
#include "token.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

typedef struct fuco_node_t fuco_node_t;

#define FUCO_FILEBUF_SIZE 1024

/* file is an unowned reference to determine data validity */
typedef struct {
    fuco_textsource_t source;
    FILE *file;
    size_t p;
    size_t size;
    int last;
    char data[FUCO_FILEBUF_SIZE];
} fuco_filebuf_t;

typedef struct {
    FILE *file;
    char *filename;
    fuco_tokentype_t last;
    fuco_queue_t sources;
    fuco_token_t curr;
    fuco_strbuf_t temp;
    fuco_filebuf_t buf;
} fuco_tokenizer_t;

extern fuco_token_descriptor_t const token_descriptors[];

bool fuco_is_nontoken(int c);

bool fuco_is_identifier_start(int c);

bool fuco_is_identifier_continue(int c);

uint64_t *fuco_parse_integer(char *lexeme);

void fuco_filebuf_init(fuco_filebuf_t *buf);

void fuco_tokenizer_init(fuco_tokenizer_t *tokenizer);

void fuco_tokenizer_destruct(fuco_tokenizer_t *tokenizer);

/* Takes ownership of filename */
void fuco_tokenizer_add_source_filename(fuco_tokenizer_t *tokenizer, 
                                        char *filename);

void fuco_tokenizer_update_filebuf(fuco_tokenizer_t *tokenizer);

int fuco_tokenizer_next_token(fuco_tokenizer_t *tokenizer);

int fuco_tokenizer_next_char(fuco_tokenizer_t *tokenizer, int c);

int fuco_tokenizer_skip_nontokens(fuco_tokenizer_t *tokenizer, int c);

bool fuco_tokenizer_expect(fuco_tokenizer_t *tokenizer, fuco_tokentype_t type);

void fuco_tokenizer_handle_curr(fuco_tokenizer_t *tokenizer);

/* 
 * Each token handler can only be used once per token and loads the next 
 * token afterwards 
 */

/* Token handler: discards current token */
int fuco_tokenizer_discard(fuco_tokenizer_t *tokenizer);

/* Token handler: moves current token to node */
int fuco_tokenizer_move(fuco_tokenizer_t *tokenizer, fuco_node_t *node);

/* Token handler: discards and loads next source. Token must be EOF. */
int fuco_tokenizer_open_next_source(fuco_tokenizer_t *tokenizer);

/* Helper functions */

bool fuco_tokenizer_discard_if(fuco_tokenizer_t *tokenizer, 
                               fuco_tokentype_t type);

bool fuco_tokenizer_move_if(fuco_tokenizer_t *tokenizer, 
                            fuco_tokentype_t type, fuco_node_t *node);

#endif
