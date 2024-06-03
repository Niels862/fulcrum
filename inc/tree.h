#ifndef FUCO_TREE_H
#define FUCO_TREE_H

#include "tokenizer.h"

typedef enum {
    FUCO_NODE_LIST,
    FUCO_NODE_FUNCTION,
    FUCO_NODE_VARIABLE
} fuco_nodetype_t;

/* Defines position and counts of child nodes. Each type starts with 
   FUCO_... = 0 and ends with FUCO_..._N to denote count. */
typedef enum {
    FUCO_LAYOUT_VARIADIC = -1,

    FUCO_LAYOUT_FUNCTION_TYPE = 0,
    FUCO_LAYOUT_FUNCTION_PARAMS,
    FUCO_LAYOUT_FUNCTION_BODY,
    FUCO_LAYOUT_FUNCTION_N
} fuco_node_layout_t;

#define FUCO_VARIADIC_NODE_INIT_SIZE 4

typedef struct fuco_node_t {
    fuco_nodetype_t type;
    fuco_token_t token;
    fuco_node_layout_t layout;
    size_t allocated;
    size_t used;
    struct fuco_node_t *children[];
} fuco_node_t;

fuco_node_t *fuco_node_new(fuco_nodetype_t type, fuco_node_layout_t layout);

void fuco_node_free(fuco_node_t *node);

/* May realloc, result should not be discarded */
fuco_node_t *fuco_node_add_child(fuco_node_t *node, fuco_node_t *child);

void fuco_node_set_child(fuco_node_t *node, fuco_node_t *child, 
                         fuco_node_layout_t index);

void fuco_node_validate(fuco_node_t *node);

#endif
