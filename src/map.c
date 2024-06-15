#include "map.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

fuco_hashvalue_t fuco_hash_string(void *data) {
    (void)data; 

    /* TODO: implement hash function */
    fuco_hashvalue_t hash = 42;
    
    return hash;
}

bool fuco_equal_string(void *left, void *right) {
    return strcmp(left, right) == 0;
}

fuco_map_entry_t *fuco_map_entry_new(void *key, void *value, 
                                     fuco_hashvalue_t hash, 
                                     fuco_map_entry_t *next) {
    fuco_map_entry_t *entry = malloc(sizeof(fuco_map_entry_t));

    entry->key = key;
    entry->value = value;
    entry->hash = hash;
    entry->next = next;

    return entry;
}

void fuco_map_entry_destruct(fuco_map_entry_t *entry, fuco_map_t *map) {
    if (map->key_free_func != NULL) {
        map->key_free_func(entry->key);
    }
    if (map->value_free_func != NULL) {
        map->value_free_func(entry->value);
    }

    free(entry);
}

void fuco_map_init(fuco_map_t *map, 
                   fuco_map_hash_t hash_func, fuco_map_equal_t equal_func, 
                   fuco_free_t key_free_func, fuco_free_t value_free_func) {
    map->hash_func = hash_func;
    map->equal_func = equal_func;
    map->key_free_func = key_free_func;
    map->value_free_func = value_free_func;
    map->cap = FUCO_MAP_INIT_SIZE;
    map->data = calloc(FUCO_MAP_INIT_SIZE, sizeof(fuco_map_entry_t *));
}


void fuco_map_destruct(fuco_map_t *map) {
    for (size_t i = 0; i < map->cap; i++) {
        fuco_map_entry_t *next, *entry = map->data[i];
        
        while (entry != NULL) {
            next = entry->next;
        
            fuco_map_entry_destruct(entry, map);

            entry = next;
        }
    }
    free(map->data);
}

void *fuco_map_lookup(fuco_map_t *map, void *key) {
    assert(key != NULL);
    
    fuco_hashvalue_t hash = map->hash_func(key);
    size_t idx = hash % map->cap;

    fuco_map_entry_t *entry = map->data[idx];
    void *value = NULL;

    while (entry != NULL && value == NULL) {
        if (entry->hash == hash && map->equal_func(entry->key, key)) {
            value = entry->value;
        } else {
            entry = entry->next;
        }
    }
    return value;
}

void fuco_map_insert(fuco_map_t *map, void *key, void *value) {
    assert(key != NULL);
    
    fuco_hashvalue_t hash = map->hash_func(key);
    size_t idx = hash % map->cap;

    /* TODO: first lookup before inserting */
    map->data[idx] = fuco_map_entry_new(key, value, hash, map->data[idx]);
}
