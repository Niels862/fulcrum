#ifndef FUCO_SYMBOL_H
#define FUCO_SYMBOL_H

#include "token.h"
#include "map.h"
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

typedef enum {
    FUCO_SYMBOL_NULL,
    FUCO_SYMBOL_VARIABLE,
    FUCO_SYMBOL_TYPE,
    FUCO_SYMBOL_FUNCTION
} fuco_symboltype_t;

typedef struct fuco_symbol_t {
    fuco_token_t *token;
    fuco_symbolid_t id;
    fuco_symboltype_t type;
    fuco_node_t *def;
    void *value;
    /* IR generated object */
    size_t obj;
    struct fuco_symbol_t *next;
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

char *fuco_symboltype_string(fuco_symboltype_t type);

bool fuco_symboltype_is_callable(fuco_symboltype_t type);

void fuco_scope_init(fuco_scope_t *scope, fuco_scope_t *prev);

void fuco_scope_destruct(fuco_scope_t *scope);

fuco_symbol_t *fuco_scope_lookup(fuco_scope_t *scope, char *ident, 
                                 fuco_textsource_t *source, bool error);

fuco_symbol_t *fuco_scope_lookup_callable(fuco_scope_t *scope, 
                                                 char *ident, 
                                                 fuco_textsource_t *source, 
                                                 bool error);

fuco_symbol_t *fuco_scope_lookup_token(fuco_scope_t *scope, 
                                       fuco_token_t *token);

fuco_symbol_t *fuco_scope_insert(fuco_scope_t *scope, 
                                 fuco_token_t *token, fuco_symbol_t *symbol);

fuco_symbol_chunk_t *fuco_symbol_chunk_new();

void fuco_symboltable_init(fuco_symboltable_t *table, fuco_scope_t *global);

void fuco_symboltable_destruct(fuco_symboltable_t *table);

void fuco_symboltable_write(fuco_symboltable_t *table, FILE *file);

fuco_symbol_t *fuco_symboltable_insert(fuco_symboltable_t *table,
                                       fuco_scope_t *scope,
                                       fuco_token_t *token,
                                       fuco_node_t *def,
                                       fuco_symboltype_t type);

#endif
