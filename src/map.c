#include "map.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
    map->cap = 0;
    map->size = 0;
    map->data = NULL;
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

void fuco_map_write(fuco_map_t *map, FILE *file, fuco_write_t write_key_func, 
                    fuco_write_t write_value_func) {
    size_t size = 0;

    fprintf(file, "{\n");

    for (size_t i = 0; i < map->cap; i++) {
        fuco_map_entry_t *entry = map->data[i];

        while (entry != NULL) {
            if (size) {
                fprintf(file, ",\n");
            }
            fprintf(file, "  ");
            write_key_func(entry->key, file);
            fprintf(file, ": ");
            write_value_func(entry->value, file);

            size++;
            entry = entry->next;
        }
    }

    assert(size == map->size);
    fprintf(file, "\n} (%ld entries)\n", size);
}

void fuco_map_maybe_rehash(fuco_map_t *map) {
    if (map->cap == 0 || map->size >= FUCO_MAP_LOAD_FACTOR * map->cap) {
        fuco_map_rehash(map);
    }
}

void fuco_map_rehash(fuco_map_t *map) {
    if (map->cap == 0) {
        map->cap = FUCO_MAP_INIT_SIZE;
        map->data = calloc(map->cap, sizeof(fuco_map_entry_t *));
        return;
    }

    fuco_map_entry_t **data_new = calloc(2 * map->cap, 
                                            sizeof(fuco_map_entry_t *));

    for (size_t i = 0; i < map->cap; i++) {
        fuco_map_entry_t *entry = map->data[i];

        while (entry != NULL) {
            fuco_map_entry_t *next = entry->next;

            size_t idx = entry->hash % (2 * map->cap);

            entry->next = data_new[idx];
            data_new[idx] = entry;

            entry = next;
        }
    }

    free(map->data);

    map->data = data_new;
    map->cap *= 2;
}

void **fuco_map_lookup(fuco_map_t *map, void *key) {
    assert(key != NULL);
    
    if (map->cap == 0) {
        return NULL;
    }

    fuco_hashvalue_t hash = map->hash_func(key);
    size_t idx = hash % map->cap;

    fuco_map_entry_t *entry = map->data[idx];

    while (entry != NULL) {
        if (entry->hash == hash && map->equal_func(entry->key, key)) {
            return &entry->value;
        }
        
        entry = entry->next;
    }

    return NULL;
}

void **fuco_map_insert(fuco_map_t *map, void *key, void *value) {
    fuco_hashvalue_t hash = map->hash_func(key);

    /* TODO: prevent second call to hash_func() */
    void **old_value = fuco_map_lookup(map, key);
    if (old_value != NULL) {
        return old_value;
    }

    fuco_map_maybe_rehash(map);

    size_t idx = hash % map->cap;
    map->data[idx] = fuco_map_entry_new(key, value, hash, map->data[idx]);

    map->size++;

    return 0;
}
