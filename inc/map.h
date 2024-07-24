#ifndef FUCO_MAP_H
#define FUCO_MAP_H

#include "defs.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FUCO_MAP_INIT_SIZE 8

#define FUCO_MAP_LOAD_FACTOR 0.75

#define FUCO_MAP_SINGLE_ENTRY ((struct fuco_map_iter_t *)0x1)

#define FUCO_MAP_IS_SINGLE_ITER(iter) ((iter)->next == FUCO_MAP_SINGLE_ENTRY)

typedef uint64_t fuco_hashvalue_t;

typedef fuco_hashvalue_t(*fuco_map_hash_t)(void *);

typedef bool(*fuco_map_equal_t)(void *, void *);

typedef struct fuco_map_iter_t {
    void *value;
    struct fuco_map_iter_t *next;
} fuco_map_iter_t;

typedef struct fuco_map_entry_t {
    void *key;
    fuco_map_iter_t iter;
    fuco_hashvalue_t hash;
    struct fuco_map_entry_t *next;
} fuco_map_entry_t;

typedef struct {
    fuco_map_hash_t hash_func;
    fuco_map_equal_t equal_func;
    fuco_free_t key_free_func;
    fuco_free_t value_free_func;
    size_t size;
    size_t cap;
    fuco_map_entry_t **data;
} fuco_map_t;

fuco_hashvalue_t fuco_hash_string(void *data);

bool fuco_equal_string(void *left, void *right);

fuco_map_entry_t *fuco_map_entry_new(void *key, void *value, 
                                     fuco_hashvalue_t hash, 
                                     fuco_map_entry_t *next, bool single);

void fuco_map_entry_destruct(fuco_map_entry_t *entry, fuco_map_t *map);

void fuco_map_init(fuco_map_t *map, 
                   fuco_map_hash_t hash_func, fuco_map_equal_t equal_func, 
                   fuco_free_t key_free_func, fuco_free_t value_free_func);

void fuco_map_destruct(fuco_map_t *map);

void fuco_map_write(fuco_map_t *map, FILE *file, fuco_write_t write_key_func, 
                    fuco_write_t write_value_func);

void fuco_map_maybe_rehash(fuco_map_t *map);

void fuco_map_rehash(fuco_map_t *map);

fuco_map_entry_t *fuco_map_lookup_entry_by_hash(fuco_map_t *map, void *key, 
                                                fuco_hashvalue_t hash);

fuco_map_iter_t *fuco_map_lookup(fuco_map_t *map, void *key);

int fuco_map_insert(fuco_map_t *map, void *key, void *value);

int fuco_map_multi_insert(fuco_map_t *map, void *key, void *value);

#endif
