#ifndef FUCO_TREE_H
#define FUCO_TREE_H

#include "tokenizer.h"
#include "symbol.h"
#include "ir.h"

typedef enum {
    FUCO_NODE_EMPTY,
    FUCO_NODE_FILEBODY,
    FUCO_NODE_BODY,
    FUCO_NODE_FUNCTION,
    FUCO_NODE_VARIABLE,
    FUCO_NODE_INTEGER,
    FUCO_NODE_RETURN,
} fuco_nodetype_t;

typedef enum {
    _
} fuco_node_attr_t;

/* Defines position and counts of child nodes. Each type starts with 
   FUCO_... = 0 and ends with FUCO_..._N to denote count. */
typedef enum {
    FUCO_LAYOUT_VARIADIC = -1,

    FUCO_LAYOUT_EMPTY_N = 0,

    FUCO_LAYOUT_FUNCTION_TYPE = 0,
    FUCO_LAYOUT_FUNCTION_PARAMS,
    FUCO_LAYOUT_FUNCTION_BODY,
    FUCO_LAYOUT_FUNCTION_N,

    FUCO_LAYOUT_VARIABLE_N = 0,

    FUCO_LAYOUT_INTEGER_N = 0,

    FUCO_LAYOUT_RETURN_VALUE = 0,
    FUCO_LAYOUT_RETURN_N
} fuco_node_layout_t;

#define FUCO_VARIADIC_NODE_INIT_SIZE 4

typedef struct {
    fuco_nodetype_t type;
    fuco_node_attr_t attr;
    fuco_node_layout_t layout;
    char *label;
} fuco_node_descriptor_t;

struct fuco_node_t {
    fuco_nodetype_t type;
    fuco_token_t token;
    fuco_symbol_t *symbol;
    size_t count;
    struct fuco_node_t *children[];
};

extern fuco_node_descriptor_t node_descriptors[];

fuco_node_t *fuco_node_base_new(fuco_nodetype_t type, size_t allocated, 
                                size_t count);

fuco_node_t *fuco_node_new(fuco_nodetype_t type);

fuco_node_t *fuco_node_variadic_new(fuco_nodetype_t type, size_t *allocated);

void fuco_node_free(fuco_node_t *node);

/* May realloc, result should not be discarded */
fuco_node_t *fuco_node_add_child(fuco_node_t *node, fuco_node_t *child, 
                                 size_t *allocated);

void fuco_node_set_child(fuco_node_t *node, fuco_node_t *child, 
                         fuco_node_layout_t index);

fuco_node_t *fuco_node_get_child(fuco_node_t *node, fuco_node_layout_t index, 
                                 fuco_nodetype_t type);

void fuco_node_write(fuco_node_t *node, FILE *stream);

void fuco_node_pretty_write(fuco_node_t *node, FILE *stream);

void fuco_node_validate(fuco_node_t *node);

int fuco_node_resolve_symbols_global(fuco_node_t *node, 
                                     fuco_symboltable_t *table, 
                                     fuco_scope_t *scope);

void fuco_node_generate_ir_propagate(fuco_node_t *node, fuco_ir_t *ir, 
                                     fuco_ir_object_t *object);

void fuco_node_generate_ir(fuco_node_t *node, fuco_ir_t *ir, 
                           fuco_ir_object_t *object);

#endif
