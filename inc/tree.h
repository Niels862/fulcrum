#ifndef FUCO_TREE_H
#define FUCO_TREE_H

#include "token.h"
#include "symbol.h"
#include "ir.h"

typedef enum {
    FUCO_NODE_EMPTY,
    FUCO_NODE_FILEBODY,
    FUCO_NODE_BODY,
    FUCO_NODE_FUNCTION,
    FUCO_NODE_PARAM_LIST,
    FUCO_NODE_PARAM,
    FUCO_NODE_CALL,
    FUCO_NODE_ARG_LIST,
    FUCO_NODE_VARIABLE,
    FUCO_NODE_INTEGER,
    FUCO_NODE_RETURN,
    FUCO_NODE_TYPE_IDENTIFIER,
} fuco_nodetype_t;

typedef enum {
    _
} fuco_node_attr_t;

/* Defines position and counts of child nodes. Each type starts with 
   FUCO_... = 0 and ends with FUCO_..._N to denote count. */
typedef enum {
    FUCO_LAYOUT_VARIADIC = -1,

    FUCO_LAYOUT_EMPTY_N = 0,

    FUCO_LAYOUT_FUNCTION_PARAMS = 0,
    FUCO_LAYOUT_FUNCTION_RET_TYPE,
    FUCO_LAYOUT_FUNCTION_BODY,
    FUCO_LAYOUT_FUNCTION_N,

    FUCO_LAYOUT_PARAM_TYPE = 0,
    FUCO_LAYOUT_PARAM_N,

    FUCO_LAYOUT_CALL_ARGS = 0,
    FUCO_LAYOUT_CALL_N,

    FUCO_LAYOUT_VARIABLE_N = 0,

    FUCO_LAYOUT_INTEGER_N = 0,

    FUCO_LAYOUT_RETURN_VALUE = 0,
    FUCO_LAYOUT_RETURN_N,

    FUCO_LAYOUT_TYPE_IDENTIFIER_N = 0
} fuco_node_layout_t;

#define FUCO_VARIADIC_NODE_INIT_SIZE 4

#define FUCO_NODE_SIZE(n) \
        sizeof(fuco_node_t) + (n) * sizeof(fuco_node_t *)

typedef struct {
    fuco_nodetype_t type;
    fuco_node_attr_t attr;
    fuco_node_layout_t layout;
    char *label;
} fuco_node_descriptor_t;

struct fuco_node_t {
    fuco_nodetype_t type;
    fuco_token_t *token;
    fuco_symbol_t *symbol;
    union {
        struct fuco_node_t *datatype;
        fuco_scope_t *scope;
    } data;
    size_t count;
    struct fuco_node_t *children[];
};

extern fuco_node_descriptor_t node_descriptors[];

fuco_node_t *fuco_node_base_new(fuco_nodetype_t type, size_t allocated, 
                                size_t count);

fuco_node_t *fuco_node_new(fuco_nodetype_t type);

fuco_node_t *fuco_node_variadic_new(fuco_nodetype_t type, size_t *allocated);

/* Transforms non-variadic node to (other) non-variadic node */
fuco_node_t *fuco_node_transform(fuco_node_t *node, fuco_nodetype_t type);

/* Transforms non-variadic node to variadic node */
fuco_node_t *fuco_node_variadic_transform(fuco_node_t *node, 
                                          fuco_nodetype_t type, 
                                          size_t *allocated);

fuco_node_t *fuco_node_set_count(fuco_node_t *node, size_t count);

void fuco_node_free(fuco_node_t *node);

/* May realloc, result should not be discarded */
fuco_node_t *fuco_node_add_child(fuco_node_t *node, fuco_node_t *child, 
                                 size_t *allocated);

void fuco_node_set_child(fuco_node_t *node, fuco_node_t *child, 
                         fuco_node_layout_t index);

void fuco_node_write(fuco_node_t *node, FILE *file);

void fuco_node_pretty_write(fuco_node_t *node, FILE *file);

void fuco_node_unparse_write(fuco_node_t *node, FILE *file);

void fuco_node_validate(fuco_node_t *node);

bool fuco_node_has_type(fuco_node_t *node);

bool fuco_node_type_match(fuco_node_t *node, fuco_node_t *other);

void fuco_node_setup_scopes(fuco_node_t *node, fuco_scope_t *scope);

fuco_scope_t *fuco_node_get_scope(fuco_node_t *node, fuco_scope_t *outer);

int fuco_node_resolve_datatypes(fuco_node_t *node, fuco_symboltable_t *table, 
                                fuco_scope_t *outer);

int fuco_node_resolve_types(fuco_node_t *node, fuco_symboltable_t *table, 
                            fuco_scope_t *outer);

int fuco_node_resolve_functions(fuco_node_t *node, fuco_symboltable_t *table, 
                                fuco_scope_t *outer);

int fuco_node_resolve_local_propagate(fuco_node_t *node, 
                                      fuco_symboltable_t *table, 
                                      fuco_scope_t *scope, fuco_node_t *ctx);

int fuco_node_resolve_local_function(fuco_node_t *node, 
                                     fuco_symboltable_t *table, 
                                     fuco_scope_t *scope);

int fuco_node_resolve_call(fuco_node_t *node, fuco_symboltable_t *table, 
                           fuco_scope_t *scope, fuco_node_t *ctx);

void fuco_node_resolve_local_id(fuco_node_t *node, fuco_symboltable_t *table, 
                                fuco_symbolid_t id);

int fuco_node_resolve_local(fuco_node_t *node, fuco_symboltable_t *table, 
                            fuco_scope_t *outer, fuco_node_t *ctx);

void fuco_node_generate_ir_propagate(fuco_node_t *node, fuco_ir_t *ir, 
                                     size_t obj);

void fuco_node_generate_ir(fuco_node_t *node, fuco_ir_t *ir, 
                           size_t obj);

void fuco_node_setup_offsets(fuco_node_t *node, uint64_t *defs);

#endif
