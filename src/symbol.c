#include "symbol.h"
#include "tree.h"
#include "utils.h"
#include "strutils.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char *fuco_symboltype_string(fuco_symboltype_t type) {
    switch (type) {
        case FUCO_SYMBOL_NULL:
            return "null";
        
        case FUCO_SYMBOL_TYPE:
            return "type";
        
        case FUCO_SYMBOL_VARIABLE:
            return "var";
        
        case FUCO_SYMBOL_FUNCTION:
            return "func";
    }

    FUCO_UNREACHED();
}

void fuco_collision_error(fuco_token_t *token) {
    fuco_syntax_error(&token->source, 
                      "symbol '%s' already declared in this scope", 
                      token->lexeme);
}

void fuco_scope_init(fuco_scope_t *scope, fuco_scope_t *prev) {
    /* The scope map does not own the identifiers so they are not freed */
    fuco_map_init(&scope->map, fuco_hash_string, fuco_equal_string, NULL, NULL);
    scope->prev = prev;
}

void fuco_scope_destruct(fuco_scope_t *scope) {
    fuco_map_destruct(&scope->map);
}

fuco_symbol_t *fuco_scope_traverse(fuco_scope_t **pscope, char *ident) {
    fuco_scope_t *scope = *pscope;
    
    while (scope != NULL) {
        void **value = fuco_map_lookup(&scope->map, ident);

        if (value != NULL) {
            *pscope = scope->prev;
            return *value;
        }

        scope = scope->prev;
    }

    *pscope = NULL;
    return NULL;
}

fuco_symbol_t *fuco_scope_lookup(fuco_scope_t *scope, char *ident, 
                                 fuco_textsource_t *source, bool error) {    
    fuco_symbol_t *symbol = fuco_scope_traverse(&scope, ident);

    if (symbol == NULL && error) {
        fuco_syntax_error(source, "'%s' was not declared in this scope", ident);
    }
    
    return symbol;
}

fuco_symbol_t *fuco_scope_lookup_token(fuco_scope_t *scope, 
                                       fuco_token_t *token) {
    return fuco_scope_lookup(scope, token->lexeme, &token->source, true);
}

fuco_symbol_t *fuco_scope_insert(fuco_scope_t *scope, 
                                 fuco_token_t *token, fuco_symbol_t *symbol) {    
    void **value = fuco_map_insert(&scope->map, token->lexeme, symbol);
    
    if (value != NULL) {
        fuco_symbol_t *prev_symbol = *value;

        switch (symbol->type) {
            case FUCO_SYMBOL_NULL:
                FUCO_UNREACHED();

            case FUCO_SYMBOL_VARIABLE:
                fuco_collision_error(token);
                return NULL;

            case FUCO_SYMBOL_FUNCTION:
                if (prev_symbol->type == FUCO_SYMBOL_TYPE) {
                    if (prev_symbol->link != NULL) {
                        symbol->link = prev_symbol->link;
                    }

                    prev_symbol->link = symbol;
                    break;
                }

                __attribute__((fallthrough));
            case FUCO_SYMBOL_TYPE:
                if (prev_symbol->type == FUCO_SYMBOL_FUNCTION) {
                    *value = symbol;
                    symbol->link = prev_symbol;
                } else {
                    fuco_collision_error(token);
                    return NULL;
                }

        }
    }

    return symbol;
}

fuco_symbol_chunk_t *fuco_symbol_chunk_new() {
    fuco_symbol_chunk_t *chunk = malloc(sizeof(fuco_symbol_chunk_t));
    
    chunk->size = 0;
    chunk->next = NULL;

    return chunk;
}

void fuco_symboltable_init(fuco_symboltable_t *table, fuco_scope_t *global) {
    static fuco_token_t null_token = {
        .lexeme = "(null)", 
        .source = {
            .col = 0, .row = 0, .filename = NULL
        }, 
        .type = FUCO_TOKEN_EMPTY
    };

    static fuco_token_t int_token = {
        .lexeme = "Int",
        .source = {
            .col = 0, .row = 0, .filename = NULL
        },
        .type = FUCO_TOKEN_IDENTIFIER
    };

    static fuco_token_t float_token = {
        .lexeme = "Float",
        .source = {
            .col = 0, .row = 0, .filename = NULL
        },
        .type = FUCO_TOKEN_IDENTIFIER
    };

    table->front = table->back = fuco_symbol_chunk_new();
    table->size = 0;

    fuco_symboltable_insert(table, NULL, &null_token, 
                            NULL, FUCO_SYMBOL_NULL);
    fuco_symboltable_insert(table, global, &int_token, 
                            NULL, FUCO_SYMBOL_TYPE);
    fuco_symboltable_insert(table, global, &float_token, 
                            NULL, FUCO_SYMBOL_TYPE);
}

void fuco_symboltable_destruct(fuco_symboltable_t *table) {    
    fuco_symbol_chunk_t *chunk = table->back;
    while (chunk != NULL) {
        fuco_symbol_chunk_t *next = chunk->next;
        free(chunk);
        chunk = next;
    }
}

void fuco_symboltable_write(fuco_symboltable_t *table, FILE *file) {
    size_t max = 0;

    fuco_symbol_chunk_t *chunk = table->back;
    while (chunk != NULL) {
        for (size_t i = 0; i < chunk->size; i++) {
            size_t len = strlen(chunk->data[i].token->lexeme);
            if (len > max) {
                max = len;
            }
        }

        chunk = chunk->next;
    }

    chunk = table->back;
    while (chunk != NULL) {
        for (size_t i = 0; i < chunk->size; i++) {
            fuco_symbol_t *symbol = &chunk->data[i];
            fprintf(file, " %*s: %*d %6s (def=%d,val=%d,obj=%ld)", 
                    (int)max,
                    symbol->token->lexeme, fuco_ceil_log(table->size, 10),
                    symbol->id, fuco_symboltype_string(symbol->type),
                    symbol->def != NULL,
                    symbol->value != NULL, symbol->obj);
            
            if (symbol->link != NULL) {
                fprintf(file, " => %d", symbol->link->id);
            }

            fprintf(file, "\n");
        }

        chunk = chunk->next;
    }
}

fuco_symbol_t *fuco_symboltable_insert(fuco_symboltable_t *table,
                                       fuco_scope_t *scope,
                                       fuco_token_t *token,
                                       fuco_node_t *def,
                                       fuco_symboltype_t type) {
    fuco_symbol_chunk_t *chunk = table->front;
    
    if (chunk->size >= FUCO_SYMBOL_CHUNK_SIZE) {
        table->front = chunk = chunk->next = fuco_symbol_chunk_new();
    }

    fuco_symbol_t *symbol = &chunk->data[chunk->size];

    symbol->token = token;
    symbol->id = table->size;
    symbol->type = type;
    symbol->def = def;
    symbol->value = NULL;
    symbol->obj = 0;

    table->size++;
    chunk->size++;

    if (scope != NULL && 
        fuco_scope_insert(scope, symbol->token, symbol) == NULL) {
        return NULL;
    }

    return symbol;
}
