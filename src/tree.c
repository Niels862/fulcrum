#include "tree.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

fuco_node_t *fuco_node_new(fuco_nodetype_t type, fuco_node_layout_t layout) {
    size_t n;
    if (layout == FUCO_LAYOUT_VARIADIC) {
        n = FUCO_VARIADIC_NODE_INIT_SIZE;
    } else {
        n = layout;
    }

    size_t size = sizeof(fuco_node_t) + n * sizeof(fuco_node_t *);
    fuco_node_t *node = malloc(size);
    
    node->type = type;
    fuco_token_init(&node->token);
    node->layout = layout;
    node->allocated = n;
    node->used = 0;

    if (n > 0) {
        memset(node->children, 0, n * sizeof(fuco_node_t *));
    }

    return node;
}

void fuco_node_free(fuco_node_t *node) {
    for (size_t i = 0; i < node->used; i++) {
        fuco_node_free(node->children[i]);
    }

    fuco_token_destruct(&node->token);
    free(node);
}

fuco_node_t *fuco_node_add_child(fuco_node_t *node, fuco_node_t *child) {    
    assert(node->used <= node->allocated);
    assert(node->layout == FUCO_LAYOUT_VARIADIC);
    assert(child != NULL);

    if (node->used == node->allocated) {
        node->allocated *= 2;

        size_t size = sizeof(fuco_node_t) 
                      + node->allocated * sizeof(fuco_node_t *);
        node = realloc(node, size);
    }

    node->children[node->used] = child;
    node->used++;

    return node;
}

void fuco_node_set_child(fuco_node_t *node, fuco_node_t *child, 
                                 fuco_node_layout_t index) {
    assert(index < node->layout);
    assert(node->layout != FUCO_LAYOUT_VARIADIC);
    assert(child != NULL);

    node->children[index] = child;
}

void fuco_node_validate(fuco_node_t *node) {
    assert(node->token.type != FUCO_TOKEN_EMPTY);
    assert(node->used <= node->allocated);

    if (node->layout != FUCO_LAYOUT_VARIADIC) {
        assert((size_t)node->layout == node->used);
    }
    
    for (size_t i = 0; i < node->used; i++) {
        assert(node->children[i] != NULL);
    }
}
