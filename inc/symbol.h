#ifndef FUCO_SYMBOL_H
#define FUCO_SYMBOL_H

#include "map.h"
#include "tokenizer.h"
#include "ir.h"
#include <stdint.h>
#include <stdio.h>

typedef uint32_t fuco_symbolid_t;

#define FUCO_SYMBOLID_INVALID (fuco_symbolid_t)0

typedef struct {
    fuco_symbolid_t *data;
    size_t size;
    size_t cap;
} fuco_symbol_context_t;

typedef struct {
    fuco_symbol_context_t ctx;
} fuco_function_def_t;

typedef struct {
    fuco_token_t *token;
    fuco_symbolid_t id;
    fuco_node_t *def;
    /* In inline function generation: actual parameter value */
    void *value;
    /* IR generated object */
    size_t obj;
} fuco_symbol_t;

typedef struct fuco_scope_t {
    fuco_map_t map;
    struct fuco_scope_t *prev;
} fuco_scope_t;

#define FUCO_SYMBOL_CHUNK_SIZE 512

typedef struct fuco_symbol_chunk_t {
    fuco_symbol_t data[FUCO_SYMBOL_CHUNK_SIZE];
    size_t size;
    struct fuco_symbol_chunk_t *next;
} fuco_symbol_chunk_t;

struct fuco_symboltable_t {
    /* Insertion at front: [back, ..., front] */
    fuco_symbol_chunk_t *back;
    fuco_symbol_chunk_t *front;
    size_t size;
};

void fuco_scope_init(fuco_scope_t *scope, fuco_scope_t *prev);

void fuco_scope_destruct(fuco_scope_t *scope);

fuco_symbol_t *fuco_scope_lookup(fuco_scope_t *scope, char *ident, 
                                 fuco_textsource_t *source, bool error);

fuco_symbol_t *fuco_scope_lookup_token(fuco_scope_t *scope, 
                                       fuco_token_t *token);

fuco_symbol_t *fuco_scope_insert(fuco_scope_t *scope, 
                                 fuco_token_t *token, fuco_symbol_t *symbol);

fuco_symbol_chunk_t *fuco_symbol_chunk_new();

void fuco_symboltable_init(fuco_symboltable_t *table);

void fuco_symboltable_destruct(fuco_symboltable_t *table);

void fuco_symboltable_write(fuco_symboltable_t *table, FILE *file);

fuco_symbol_t *fuco_symboltable_insert(fuco_symboltable_t *table, 
                                       fuco_scope_t *scope,
                                       fuco_token_t *token,
                                       fuco_node_t *def);

#endif
