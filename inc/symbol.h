#ifndef FUCO_SYMBOL_H
#define FUCO_SYMBOL_H

#include "map.h"
#include "tokenizer.h"
#include <stdint.h>
#include <stdio.h>

typedef uint32_t fuco_symbolid_t;

#define FUCO_INVALID_SYMBOLID (fuco_symbolid_t)0

typedef struct {
    fuco_token_t *token;
    fuco_symbolid_t id;
} fuco_symbol_t;

typedef struct fuco_scope_t {
    fuco_map_t map;
    struct fuco_scope_t *prev;
} fuco_scope_t;

#define FUCO_SYMBOLTABLE_INIT_SIZE 64

typedef struct {
    fuco_symbol_t *list;
    size_t size;
    size_t cap;
} fuco_symboltable_t;

void fuco_scope_init(fuco_scope_t *scope, fuco_scope_t *prev);

void fuco_scope_destruct(fuco_scope_t *scope);

fuco_symbol_t *fuco_scope_lookup(fuco_scope_t *scope, char *ident);

fuco_symbol_t *fuco_scope_insert(fuco_scope_t *scope, 
                                 fuco_token_t *token, fuco_symbol_t *symbol);

void fuco_symboltable_init(fuco_symboltable_t *table);

void fuco_symboltable_destruct(fuco_symboltable_t *table);

void fuco_symboltable_write(fuco_symboltable_t *table, FILE *stream);

fuco_symbol_t *fuco_symboltable_insert(fuco_symboltable_t *table, 
                                       fuco_scope_t *scope,
                                       fuco_token_t *token);

#endif
