#include "map.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

fuco_hashvalue_t fuco_hash_string(void *data) {
    char *str = data;

    fuco_hashvalue_t hash = 5381;
    int c;

    while ((c = *str) != '\0') {
        hash = ((hash << 5) + hash) + c;
        str++;
    }

    return hash;
}

bool fuco_equal_string(void *left, void *right) {
    return strcmp(left, right) == 0;
}

fuco_map_entry_t *fuco_map_entry_new(void *key, void *value, 
                                     fuco_hashvalue_t hash, 
                                     fuco_map_entry_t *next, bool single) {
    fuco_map_entry_t *entry = malloc(sizeof(fuco_map_entry_t));

    entry->key = key;
    entry->iter.value = value;
    entry->iter.next = single ? FUCO_MAP_SINGLE_ENTRY : NULL;
    entry->hash = hash;
    entry->next = next;

    return entry;
}

void fuco_map_entry_destruct(fuco_map_entry_t *entry, fuco_map_t *map) {
    if (map->key_free_func != NULL) {
        map->key_free_func(entry->key);
    }

    if (map->value_free_func != NULL && entry->iter.value != NULL) {
        map->value_free_func(entry->iter.value);
    }

    if (!FUCO_MAP_IS_SINGLE_ENTRY(entry)) {
        fuco_map_iter_t *iter = entry->iter.next;

        while (iter != NULL) {
            fuco_map_iter_t *next = iter->next;

            if (map->value_free_func != NULL && entry->iter.value != NULL) {
                map->value_free_func(entry->iter.value);
            }

            free(iter);
            
            iter = next;
        }
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

    if (map->data != NULL) {
        free(map->data);
    }
}

fuco_map_entry_t *fuco_map_lookup_entry_by_hash(fuco_map_t *map, void *key, 
                                                fuco_hashvalue_t hash) {
    assert(key != NULL);
    
    size_t idx = hash % map->cap;

    fuco_map_entry_t *entry = map->data[idx];

    while (entry != NULL) {
        if (entry->hash == hash && map->equal_func(entry->key, key)) {
            return entry;
        }
        
        entry = entry->next;
    }

    return NULL;
}

void *fuco_map_lookup(fuco_map_t *map, void *key) {
    assert(key != NULL);

    fuco_hashvalue_t hash = map->hash_func(key);
    fuco_map_entry_t *entry = fuco_map_lookup_entry_by_hash(map, key, hash);

    if (entry == NULL || !FUCO_MAP_IS_SINGLE_ENTRY(entry)) {
        return NULL;
    }

    return entry->iter.value;
}

void *fuco_map_multi_lookup(fuco_map_t *map, void *key) {
    assert(key != NULL);
    
    fuco_hashvalue_t hash = map->hash_func(key);
    fuco_map_entry_t *entry = fuco_map_lookup_entry_by_hash(map, key, hash);

    if (entry == NULL || FUCO_MAP_IS_SINGLE_ENTRY(entry)) {
        return NULL;
    }

    return &entry->iter;
}

int fuco_map_insert(fuco_map_t *map, void *key, void *value) {
    fuco_hashvalue_t hash = map->hash_func(key);
    size_t idx = hash % map->cap;

    if (fuco_map_lookup_entry_by_hash(map, key, hash) != NULL) {
        return 1;
    }

    map->data[idx] = fuco_map_entry_new(key, value, hash, 
                                        map->data[idx], true);

    return 0;
}

int fuco_map_multi_insert(fuco_map_t *map, void *key, void *value) {
    fuco_hashvalue_t hash = map->hash_func(key);
    fuco_map_entry_t *entry = fuco_map_lookup_entry_by_hash(map, key, hash);

    if (entry == NULL) {
        size_t idx = hash % map->cap;
        map->data[idx] = fuco_map_entry_new(key, value, hash, 
                                            map->data[idx], false);
    } else {
        if (FUCO_MAP_IS_SINGLE_ENTRY(entry)) {
            return 1;
        }

        fuco_map_iter_t *iter = malloc(sizeof(fuco_map_iter_t));
        *iter = entry->iter;

        entry->iter.value = value;
        entry->iter.next = iter;
    }

    return 0;
}