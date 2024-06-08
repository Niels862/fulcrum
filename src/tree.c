#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

fuco_node_descriptor_t node_descriptors[] = {
    { FUCO_NODE_EMPTY, 0, FUCO_LAYOUT_EMPTY_N, "empty" },
    { FUCO_NODE_LIST, 0, FUCO_LAYOUT_VARIADIC, "list" },
    { FUCO_NODE_FUNCTION, 0, FUCO_LAYOUT_FUNCTION_N, "function" },
    { FUCO_NODE_VARIABLE, 0, FUCO_LAYOUT_VARIABLE_N, "variable" },
    { FUCO_NODE_INTEGER, 0, FUCO_LAYOUT_INTEGER_N, "integer" },
    { FUCO_NODE_RETURN, 0, FUCO_LAYOUT_RETURN_N, "return" },
};

fuco_node_t *fuco_node_base_new(fuco_nodetype_t type, size_t allocated, 
                                size_t count) {
    size_t size = sizeof(fuco_node_t) + allocated * sizeof(fuco_node_t *);
    fuco_node_t *node = malloc(size);
    
    node->type = type;
    fuco_token_init(&node->token);
    node->count = count;

    if (allocated > 0) {
        memset(node->children, 0, allocated * sizeof(fuco_node_t *));
    }

    return node;
}

fuco_node_t *fuco_node_new(fuco_nodetype_t type) {
    assert(node_descriptors[type].layout != FUCO_LAYOUT_VARIADIC);

    size_t count = node_descriptors[type].layout;

    return fuco_node_base_new(type, count, count);
}

fuco_node_t *fuco_node_variadic_new(fuco_nodetype_t type, size_t *allocated) {
    assert(node_descriptors[type].layout == FUCO_LAYOUT_VARIADIC);

    *allocated = FUCO_VARIADIC_NODE_INIT_SIZE;

    return fuco_node_base_new(type, *allocated, 0);
}

void fuco_node_free(fuco_node_t *node) {
    for (size_t i = 0; i < node->count; i++) {
        if (node->children[i] != NULL) {
            fuco_node_free(node->children[i]);
        }
    }

    fuco_token_destruct(&node->token);
    free(node);
}

fuco_node_t *fuco_node_add_child(fuco_node_t *node, fuco_node_t *child, 
                                 size_t *allocated) {    
    assert(node_descriptors[node->type].layout == FUCO_LAYOUT_VARIADIC);
    fuco_node_validate(child);

    if (node->count >= *allocated) {
        *allocated *= 2;

        size_t size = sizeof(fuco_node_t) + *allocated * sizeof(fuco_node_t *);
        node = realloc(node, size);
    }

    node->children[node->count] = child;
    node->count++;

    return node;
}

void fuco_node_set_child(fuco_node_t *node, fuco_node_t *child, 
                         fuco_node_layout_t index) {
    assert((size_t)index < node->count);
    assert(node_descriptors[node->type].layout != FUCO_LAYOUT_VARIADIC);
    assert(node->children[index] == NULL);
    fuco_node_validate(child);

    node->children[index] = child;
}

void fuco_node_write(fuco_node_t *node, FILE *stream) {
    fprintf(stream, "[(");
    fuco_token_write(&node->token, stream);
    fprintf(stream, ")");

    for (size_t i = 0; i < node->count; i++) {
        fprintf(stream, " ");
        fuco_node_write(node->children[i], stream);
    }
    
    fprintf(stream, "]");
}

void fuco_node_pretty_write(fuco_node_t *node, FILE *stream) {
    static bool is_last[256];
    static size_t depth = 0;

    assert(depth < 256);

    for (size_t i = 1; i < depth; i++) {
        if (is_last[i - 1]) {
            fprintf(stream, "    ");
        } else {
            fprintf(stream, "│   ");
        }
    }

    if (depth > 0) {
        if (is_last[depth - 1]) {
            fprintf(stream, "└───");
        } else {
            fprintf(stream, "├───");
        }
    }

    if (node == NULL) {
        fprintf(stderr, "(null)\n");
    } else {
        char *label = node_descriptors[node->type].label;
        if (*label != '\0') {
            fprintf(stream, "%s", label);
        }

        if (node->token.type != FUCO_TOKEN_EMPTY) {
            fprintf(stream, ": ");
            fuco_token_write(&node->token, stream);
        }

        fprintf(stream, "\n");

        depth++;

        for (size_t i = 0; i < node->count; i++) {
            is_last[depth - 1] = i == node->count - 1;
            fuco_node_pretty_write(node->children[i], stream);
        }

        depth--;
    }
}

void fuco_node_validate(fuco_node_t *node) {
    assert(node != NULL);
    assert(node_descriptors[node->type].layout == FUCO_LAYOUT_VARIADIC 
           || (size_t)node_descriptors[node->type].layout == node->count);
    
    return;
    
    for (size_t i = 0; i < node->count; i++) {
        assert(node->children[i] != NULL);
    }
}
