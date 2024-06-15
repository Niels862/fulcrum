#ifndef FUCO_MAP_H
#define FUCO_MAP_H

#include "defs.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FUCO_MAP_INIT_SIZE 8

#define FUCO_MAP_LOAD_FACTOR 0.75

typedef uint64_t fuco_hashvalue_t;

typedef fuco_hashvalue_t(*fuco_map_hash_t)(void *);

typedef bool(*fuco_map_equal_t)(void *, void *);

typedef struct fuco_map_entry_t {
    void *key;
    void *value;
    fuco_hashvalue_t hash;
    struct fuco_map_entry_t *next;
} fuco_map_entry_t;

typedef struct {
    fuco_map_hash_t hash_func;
    fuco_map_equal_t equal_func;
    fuco_free_t key_free_func;
    fuco_free_t value_free_func;
    size_t cap;
    fuco_map_entry_t **data;
} fuco_map_t;

fuco_hashvalue_t fuco_hash_string(void *data);

bool fuco_equal_string(void *left, void *right);

fuco_map_entry_t *fuco_map_entry_new(void *key, void *value, 
                                     fuco_hashvalue_t hash, 
                                     fuco_map_entry_t *next);

void fuco_map_init(fuco_map_t *map, 
                   fuco_map_hash_t hash_func, fuco_map_equal_t equal_func, 
                   fuco_free_t key_free_func, fuco_free_t value_free_func);

void fuco_map_destruct(fuco_map_t *map);

void *fuco_map_lookup(fuco_map_t *map, void *key);

void fuco_map_insert(fuco_map_t *map, void *key, void *value);

#endif
