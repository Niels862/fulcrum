#include "tree.h"
#include "utils.h"
#include "instruction.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

fuco_node_descriptor_t node_descriptors[] = {
    { FUCO_NODE_EMPTY, 0, FUCO_LAYOUT_EMPTY_N, "empty" },
    { FUCO_NODE_FILEBODY, 0, FUCO_LAYOUT_VARIADIC, "filebody" },
    { FUCO_NODE_BODY, 0, FUCO_LAYOUT_VARIADIC, "body" },
    { FUCO_NODE_FUNCTION, 0, FUCO_LAYOUT_FUNCTION_N, "function" },
    { FUCO_NODE_PARAM_LIST, 0, FUCO_LAYOUT_VARIADIC, "param-list" },
    { FUCO_NODE_PARAM, 0, FUCO_LAYOUT_PARAM_N, "param" },
    { FUCO_NODE_CALL, 0, FUCO_LAYOUT_CALL_N, "call" },
    { FUCO_NODE_ARG_LIST, 0, FUCO_LAYOUT_VARIADIC, "arg-list" },
    { FUCO_NODE_VARIABLE, 0, FUCO_LAYOUT_VARIABLE_N, "variable" },
    { FUCO_NODE_INTEGER, 0, FUCO_LAYOUT_INTEGER_N, "integer" },
    { FUCO_NODE_RETURN, 0, FUCO_LAYOUT_RETURN_N, "return" },
    { FUCO_NODE_TYPE_IDENTIFIER, 0, FUCO_LAYOUT_TYPE_IDENTIFIER_N, "type-id" },
};

fuco_node_t *fuco_node_base_new(fuco_nodetype_t type, size_t allocated, 
                                size_t count) {
    fuco_node_t *node = malloc(FUCO_NODE_SIZE(allocated));
    
    node->type = type;
    node->token = NULL;
    node->symbol = NULL;
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

fuco_node_t *fuco_node_transform(fuco_node_t *node, fuco_nodetype_t type) {
    assert(node_descriptors[node->type].layout != FUCO_LAYOUT_VARIADIC);
    assert(node_descriptors[type].layout != FUCO_LAYOUT_VARIADIC);

    node->type = type;

    return fuco_node_set_count(node, node_descriptors[type].layout);
}

fuco_node_t *fuco_node_variadic_transform(fuco_node_t *node, 
                                          fuco_nodetype_t type, 
                                          size_t *allocated) {
    assert(node_descriptors[type].layout == FUCO_LAYOUT_VARIADIC);
    
    if (node->count > FUCO_VARIADIC_NODE_INIT_SIZE) {
        *allocated = node->count;
    } else {
        *allocated = FUCO_VARIADIC_NODE_INIT_SIZE;
    }

    node->type = type;

    return fuco_node_set_count(node, *allocated);;
}

fuco_node_t *fuco_node_set_count(fuco_node_t *node, size_t count) {
    if (count > node->count) {
        node = realloc(node, FUCO_NODE_SIZE(count));

        for (size_t i = node->count; i < count; i++) {
            node->children[i] = NULL;
        }
    }

    node->count = count;

    return node;
}

void fuco_node_free(fuco_node_t *node) {
    for (size_t i = 0; i < node->count; i++) {
        if (node->children[i] != NULL) {
            fuco_node_free(node->children[i]);
        }
    }

    free(node);
}

fuco_node_t *fuco_node_add_child(fuco_node_t *node, fuco_node_t *child, 
                                 size_t *allocated) {    
    assert(node_descriptors[node->type].layout == FUCO_LAYOUT_VARIADIC);
    fuco_node_validate(child);

    if (node->count >= *allocated) {
        *allocated *= 2;

        node = realloc(node, FUCO_NODE_SIZE(*allocated));
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

void fuco_node_write(fuco_node_t *node, FILE *file) {
    fprintf(file, "[(");
    fuco_token_write(node->token, file);
    fprintf(file, ")");

    for (size_t i = 0; i < node->count; i++) {
        fprintf(file, " ");
        fuco_node_write(node->children[i], file);
    }
    
    fprintf(file, "]");
}

void fuco_node_pretty_write(fuco_node_t *node, FILE *file) {
    static bool is_last[256];
    static size_t depth = 0;

    assert(depth < 256);

    for (size_t i = 1; i < depth; i++) {
        if (is_last[i - 1]) {
            fprintf(file, "    ");
        } else {
            fprintf(file, "│   ");
        }
    }

    if (depth > 0) {
        if (is_last[depth - 1]) {
            fprintf(file, "└───");
        } else {
            fprintf(file, "├───");
        }
    }

    if (node == NULL) {
        fprintf(stderr, "(null)\n");
    } else {
        char *label = node_descriptors[node->type].label;
        if (*label != '\0') {
            fprintf(file, "%s", label);
        }

        if (node->token != NULL) {
            fprintf(file, ": ");
            fuco_token_write(node->token, file);
        }

        if (node->symbol != NULL) {
            fprintf(file, " (id=%d)", node->symbol->id);
        }
        
        fprintf(file, "\n");

        depth++;

        for (size_t i = 0; i < node->count; i++) {
            is_last[depth - 1] = i == node->count - 1;
            fuco_node_pretty_write(node->children[i], file);
        }

        depth--;
    }
}

void fuco_node_validate(fuco_node_t *node) {
    assert(node != NULL);
    assert(node_descriptors[node->type].layout == FUCO_LAYOUT_VARIADIC 
           || (size_t)node_descriptors[node->type].layout == node->count);
        
    for (size_t i = 0; i < node->count; i++) {
        assert(node->children[i] != NULL);
    }
}

int fuco_node_resolve_global(fuco_node_t *node, 
                                      fuco_symboltable_t *table, 
                                      fuco_scope_t *scope) {    
    switch (node->type) {
        case FUCO_NODE_EMPTY:
        case FUCO_NODE_BODY:
        case FUCO_NODE_PARAM_LIST:
        case FUCO_NODE_PARAM:
        case FUCO_NODE_CALL:
        case FUCO_NODE_ARG_LIST:
        case FUCO_NODE_VARIABLE:
        case FUCO_NODE_INTEGER:
        case FUCO_NODE_RETURN:
        case FUCO_NODE_TYPE_IDENTIFIER:
            break;

        case FUCO_NODE_FILEBODY:
            for (size_t i = 0; i < node->count; i++) {
                fuco_node_t *child = node->children[i];
                if (fuco_node_resolve_global(child, table, scope)) {
                    return 1;
                }
            }
            break;

        case FUCO_NODE_FUNCTION:
            node->symbol = fuco_symboltable_insert(table, scope, 
                                                   node->token, node, 
                                                   FUCO_SYMBOL_FUNCTION);
            if (node->symbol == NULL) {
                return 1;
            }
            break;
    }

    return 0;
}

int fuco_node_resolve_local_propagate(fuco_node_t *node, 
                                      fuco_symboltable_t *table, 
                                      fuco_scope_t *scope) {
    for (size_t i = 0; i < node->count; i++) {
        if (fuco_node_resolve_local(node->children[i], table, scope)) {
            return 1;
        }
    }

    return 0;
}

int fuco_node_resolve_local_function(fuco_node_t *node, 
                                     fuco_symboltable_t *table, 
                                     fuco_scope_t *scope) {
    int res = 0;

    fuco_function_def_t *def = NULL;
    node->symbol->value = def;
    
    fuco_scope_t next;
    fuco_scope_init(&next, scope);
        
    res = fuco_node_resolve_local_propagate(node, table, &next);

    fuco_scope_destruct(&next);

    return res;
}

int fuco_node_resolve_local(fuco_node_t *node, fuco_symboltable_t *table, 
                            fuco_scope_t *scope) {
    switch (node->type) {
        case FUCO_NODE_EMPTY:
        case FUCO_NODE_INTEGER:
            break;

        case FUCO_NODE_FILEBODY:
        case FUCO_NODE_BODY:
        case FUCO_NODE_ARG_LIST:
        case FUCO_NODE_RETURN:
            if (fuco_node_resolve_local_propagate(node, table, scope)) {
                return 1;
            }
            break;

        case FUCO_NODE_FUNCTION:
            if (fuco_node_resolve_local_function(node, table, scope)) {
                return 1;
            }
            break;

        case FUCO_NODE_PARAM_LIST:
            for (size_t i = 0; i < node->count; i++) {
                if (fuco_node_resolve_local(node->children[i], table, scope)) {
                    return 1;
                }
            }
            break;

        case FUCO_NODE_PARAM:
            node->symbol = fuco_symboltable_insert(table, scope, 
                                                   node->token, node, 
                                                   FUCO_SYMBOL_VARIABLE);
            if (node->symbol == NULL) {
                return 1;
            }
            
            fuco_node_resolve_local_propagate(node, table, scope);
            break;

        case FUCO_NODE_CALL:
        case FUCO_NODE_VARIABLE:
        case FUCO_NODE_TYPE_IDENTIFIER:
            fuco_node_resolve_local_propagate(node, table, scope);

            node->symbol = fuco_scope_lookup_token(scope, node->token);
            if (node->symbol == NULL) {
                return 1;
            }
            break;
    }
    
    return 0;
}

void fuco_node_generate_ir_propagate(fuco_node_t *node, fuco_ir_t *ir, 
                                     size_t obj) {
    for (size_t i = 0; i < node->count; i++) {
        fuco_node_generate_ir(node->children[i], ir, obj);
    }
}

void fuco_node_generate_ir(fuco_node_t *node, fuco_ir_t *ir, 
                           size_t obj) {
    fuco_node_t *next = NULL;

    switch (node->type) {
        case FUCO_NODE_EMPTY:
        case FUCO_NODE_PARAM_LIST:
        case FUCO_NODE_PARAM:
        case FUCO_NODE_TYPE_IDENTIFIER:
            break;

        case FUCO_NODE_FILEBODY:
        case FUCO_NODE_BODY:
            fuco_node_generate_ir_propagate(node, ir, obj);
            break;

        case FUCO_NODE_FUNCTION:
            node->symbol->obj = fuco_ir_add_object(ir, node->symbol->id, 
                                                   node);

            next = node->children[FUCO_LAYOUT_FUNCTION_BODY];
            
            fuco_node_generate_ir(next, ir, node->symbol->obj);
            break;

        case FUCO_NODE_CALL:
            next = node->children[FUCO_LAYOUT_CALL_ARGS];

            fuco_node_generate_ir(next, ir, obj);

            fuco_ir_add_instr_imm48_label(ir, obj, FUCO_OPCODE_CALL, 
                                          node->symbol->id);
            break;

        case FUCO_NODE_ARG_LIST:
            /* Arguments are pushed in reverse order */
            for (size_t i = node->count; i > 0; i--) {
                fuco_node_generate_ir(node->children[i - 1], ir, obj);
            }
            break;

        case FUCO_NODE_VARIABLE:
            fuco_ir_add_instr_imm48_label(ir, obj, FUCO_OPCODE_RLOADQ, 
                                          node->symbol->id);
            break;

        case FUCO_NODE_INTEGER:
            fuco_ir_add_instr_imm48(ir, obj, FUCO_OPCODE_PUSHQ, 
                                   *(uint64_t *)node->token->data);
            break;

        case FUCO_NODE_RETURN:
            fuco_node_generate_ir_propagate(node, ir, obj);
            fuco_ir_add_instr(ir, obj, FUCO_OPCODE_RETQ);
            break;
    }
}

void fuco_node_setup_offsets(fuco_node_t *node, uint64_t *defs) {
    int64_t offset = -3 * sizeof(uint64_t);

    fuco_node_t *params = node->children[FUCO_LAYOUT_FUNCTION_PARAMS];

    for (size_t i = 0; i < params->count; i++) {
        fuco_node_t *param = params->children[i];

        defs[param->symbol->id] = offset; /* TODO safer */
        offset -= 8;
    }
}
