#include "map.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

    if (!FUCO_MAP_IS_SINGLE_ITER(&entry->iter)) {
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
    fprintf(file, "{\n");

    for (size_t i = 0; i < map->size; i++) {
        fuco_map_entry_t *entry = map->data[i];

        while (entry != NULL) {
            fprintf(file, "  ");

            write_key_func(entry->key, file);

            fuco_map_iter_t *iter = &entry->iter;

            if (FUCO_MAP_IS_SINGLE_ITER(iter)) {
                fprintf(file, ": ");
                write_value_func(iter->value, file);
                fprintf(file, "\n");
            } else {
                fprintf(file, ": {\n");

                while (iter != NULL) {
                    fprintf(file, "    ");
                    write_value_func(iter->value, file);
                    fprintf(file, "\n");

                    iter = iter->next;
                }

                fprintf(file, "  }\n");
            }

            entry = entry->next;
        }
    }

    fprintf(file, "}\n");
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

fuco_map_entry_t *fuco_map_lookup_entry_by_hash(fuco_map_t *map, void *key, 
                                                fuco_hashvalue_t hash) {
    assert(key != NULL);
    
    if (map->cap == 0) {
        return NULL;
    }

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

fuco_map_iter_t *fuco_map_lookup(fuco_map_t *map, void *key) {
    assert(key != NULL);

    if (map->cap == 0) {
        return NULL;
    }

    fuco_hashvalue_t hash = map->hash_func(key);
    fuco_map_entry_t *entry = fuco_map_lookup_entry_by_hash(map, key, hash);

    if (entry == NULL) {
        return NULL;
    }

    return &entry->iter;
}

int fuco_map_insert(fuco_map_t *map, void *key, void *value) {
    fuco_hashvalue_t hash = map->hash_func(key);

    if (fuco_map_lookup_entry_by_hash(map, key, hash) != NULL) {
        return 1;
    }

    fuco_map_maybe_rehash(map);

    size_t idx = hash % map->cap;
    map->data[idx] = fuco_map_entry_new(key, value, hash, 
                                        map->data[idx], true);

    map->size++;

    return 0;
}

int fuco_map_multi_insert(fuco_map_t *map, void *key, void *value) {
    fuco_hashvalue_t hash = map->hash_func(key);
    fuco_map_entry_t *entry = fuco_map_lookup_entry_by_hash(map, key, hash);

    if (entry == NULL) {
        fuco_map_maybe_rehash(map);

        size_t idx = hash % map->cap;
        map->data[idx] = fuco_map_entry_new(key, value, hash, 
                                            map->data[idx], false);
        
        map->size++;
    } else {
        if (FUCO_MAP_IS_SINGLE_ITER(&entry->iter)) {
            return 1;
        }

        fuco_map_iter_t *iter = malloc(sizeof(fuco_map_iter_t));
        *iter = entry->iter;

        entry->iter.value = value;
        entry->iter.next = iter;

        /* Note: multiple entries with same key count as 1 towards size */
    }

    return 0;
}
