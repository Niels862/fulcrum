#include "tree.h"
#include "utils.h"
#include "strutils.h"
#include "instruction.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

fuco_node_t fuco_node_empty = {
    .type = FUCO_NODE_EMPTY,
    .token = NULL,
    .symbol = NULL,
    .count = 0
};

fuco_node_layout_t fuco_nodetype_get_layout(fuco_nodetype_t type) {
    switch (type) {
        case FUCO_NODE_EMPTY:
            return FUCO_LAYOUT_EMPTY_N;
        
        case FUCO_NODE_FILEBODY:
            return FUCO_LAYOUT_VARIADIC;

        case FUCO_NODE_BODY:
            return FUCO_LAYOUT_VARIADIC;

        case FUCO_NODE_FUNCTION:
            return FUCO_LAYOUT_FUNCTION_N;

        case FUCO_NODE_PARAM_LIST:
            return FUCO_LAYOUT_VARIADIC;
        
        case FUCO_NODE_PARAM:
            return FUCO_LAYOUT_PARAM_N;

        case FUCO_NODE_CALL:
            return FUCO_LAYOUT_CALL_N;

        case FUCO_NODE_INSTR:
            return FUCO_LAYOUT_INSTR_N;

        case FUCO_NODE_ARG_LIST:
            return FUCO_LAYOUT_VARIADIC;

        case FUCO_NODE_VARIABLE:
            return FUCO_LAYOUT_VARIABLE_N;

        case FUCO_NODE_INTEGER:
            return FUCO_LAYOUT_INTEGER_N;

        case FUCO_NODE_RETURN:
            return FUCO_LAYOUT_RETURN_N;

        case FUCO_NODE_IF_ELSE:
            return FUCO_LAYOUT_IF_ELSE_N;

        case FUCO_NODE_WHILE:
            return FUCO_LAYOUT_WHILE_N;

        case FUCO_NODE_TYPE_IDENTIFIER: 
            return FUCO_LAYOUT_TYPE_IDENTIFIER_N;
    }

    FUCO_UNREACHED();
}

char *fuco_nodetype_get_label(fuco_nodetype_t type) {
    switch (type) {
        case FUCO_NODE_EMPTY:
            return "empty";
        
        case FUCO_NODE_FILEBODY:
            return "filebody";

        case FUCO_NODE_BODY:
            return "body";

        case FUCO_NODE_FUNCTION:
            return "function";

        case FUCO_NODE_PARAM_LIST:
            return "param-list";
        
        case FUCO_NODE_PARAM:
            return "param";

        case FUCO_NODE_CALL:
            return "call";

        case FUCO_NODE_INSTR:
            return "instr";

        case FUCO_NODE_ARG_LIST:
            return "arg-list";

        case FUCO_NODE_VARIABLE:
            return "variable";

        case FUCO_NODE_INTEGER:
            return "integer";

        case FUCO_NODE_RETURN:
            return "return";

        case FUCO_NODE_IF_ELSE:
            return "if-else";

        case FUCO_NODE_WHILE:
            return "while";

        case FUCO_NODE_TYPE_IDENTIFIER: 
            return "type-identifier";
    }

    FUCO_UNREACHED();
}

fuco_node_t *fuco_node_base_new(fuco_nodetype_t type, size_t allocated, 
                                size_t count) {
    fuco_node_t *node = malloc(FUCO_NODE_SIZE(allocated));
    
    node->type = type;
    node->token = NULL;
    node->symbol = NULL;
    node->data.datatype = NULL;
    node->count = count;

    if (allocated > 0) {
        memset(node->children, 0, allocated * sizeof(fuco_node_t *));
    }

    return node;
}

fuco_node_t *fuco_node_new(fuco_nodetype_t type) {
    assert(fuco_nodetype_get_layout(type) != FUCO_LAYOUT_VARIADIC);

    size_t count = fuco_nodetype_get_layout(type);

    return fuco_node_base_new(type, count, count);
}

fuco_node_t *fuco_node_variadic_new(fuco_nodetype_t type, size_t *allocated) {
    assert(fuco_nodetype_get_layout(type) == FUCO_LAYOUT_VARIADIC);

    *allocated = FUCO_VARIADIC_NODE_INIT_SIZE;

    return fuco_node_base_new(type, *allocated, 0);
}

fuco_node_t *fuco_node_call_new(size_t args_n, ...) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_CALL);

    size_t allocated;
    fuco_node_t *sub = fuco_node_variadic_new(FUCO_NODE_ARG_LIST, &allocated);

    va_list args;
    va_start(args, args_n);

    for (size_t i = 0; i < args_n; i++) {
        sub = fuco_node_add_child(sub, va_arg(args, fuco_node_t *), &allocated);
    }

    va_end(args);

    fuco_node_set_child(node, sub, FUCO_LAYOUT_CALL_ARGS);

    return node;
}

fuco_node_t *fuco_node_transform(fuco_node_t *node, fuco_nodetype_t type) {
    assert(fuco_nodetype_get_layout(node->type) != FUCO_LAYOUT_VARIADIC);
    assert(fuco_nodetype_get_layout(type) != FUCO_LAYOUT_VARIADIC);

    node->type = type;

    return fuco_node_set_count(node, fuco_nodetype_get_layout(type));
}

fuco_node_t *fuco_node_variadic_transform(fuco_node_t *node, 
                                          fuco_nodetype_t type, 
                                          size_t *allocated) {
    assert(fuco_nodetype_get_layout(type) == FUCO_LAYOUT_VARIADIC);
    
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
    switch (node->type) {
        case FUCO_NODE_FILEBODY:
        case FUCO_NODE_FUNCTION: /* TODO: scoped body? */
            if (node->data.scope != NULL) {
                fuco_scope_destruct(node->data.scope);
                free(node->data.scope);
            }
            break;

        default:
            break;
    }

    for (size_t i = 0; i < node->count; i++) {
        if (node->children[i] != NULL) {
            fuco_node_free(node->children[i]);
        }
    }

    if (node != &fuco_node_empty) {
        free(node);
    }
}

fuco_node_t *fuco_node_add_child(fuco_node_t *node, fuco_node_t *child, 
                                 size_t *allocated) {    
    assert(fuco_nodetype_get_layout(node->type) == FUCO_LAYOUT_VARIADIC);
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
    assert(fuco_nodetype_get_layout(node->type) != FUCO_LAYOUT_VARIADIC);
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
        char *label = fuco_nodetype_get_label(node->type);
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

        if (fuco_node_has_type(node)) {
            fprintf(file, " :: ");

            if (node->data.datatype == NULL) {
                fprintf(stderr, "(nil)");
            } else {
                fuco_node_unparse_write(node->data.datatype, file);
            }
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

void fuco_node_unparse_write(fuco_node_t *node, FILE *file) {
    fuco_node_t *sub;
    
    switch (node->type) {
        case FUCO_NODE_EMPTY:
            FUCO_UNREACHED();

        case FUCO_NODE_FILEBODY:
            for (size_t i = 0; i < node->count; i++) {
                fuco_node_unparse_write(node->children[i], file);
                fprintf(file, " ");
            }
            break;

        case FUCO_NODE_BODY:
            fprintf(file, "{ ");
            for (size_t i = 0; i < node->count; i++) {
                fuco_node_unparse_write(node->children[i], file);
                fprintf(file, " ");
            }
            fprintf(file, "}");
            break;

        case FUCO_NODE_FUNCTION:
            fprintf(file, "def %s", fuco_token_string(node->token));

            sub = node->children[FUCO_LAYOUT_FUNCTION_PARAMS];
            fuco_node_unparse_write(sub, file);

            fprintf(file, " -> ");

            sub = node->children[FUCO_LAYOUT_FUNCTION_RET_TYPE];
            fuco_node_unparse_write(sub, file);

            fprintf(file, " ");

            sub = node->children[FUCO_LAYOUT_FUNCTION_BODY];
            fuco_node_unparse_write(sub, file);
            break;

        case FUCO_NODE_PARAM_LIST:
        case FUCO_NODE_ARG_LIST:    
            fprintf(file, "(");
            for (size_t i = 0; i < node->count; i++) {
                if (i > 0) {
                    fprintf(file, ", ");
                }

                fuco_node_unparse_write(node->children[i], file);
            }
            fprintf(file, ")");
            break;

        case FUCO_NODE_PARAM:
            fprintf(file, "%s: ", fuco_token_string(node->token));
            
            sub = node->children[FUCO_LAYOUT_PARAM_TYPE];
            fuco_node_unparse_write(sub, file);
            break;

        case FUCO_NODE_CALL:
        case FUCO_NODE_INSTR:
            if (node->type == FUCO_NODE_INSTR) {
                fprintf(file, "%%");
            }

            fprintf(file, "%s", fuco_token_string(node->token));

            sub = node->children[FUCO_LAYOUT_CALL_ARGS];
            fuco_node_unparse_write(sub, file);
            break;

        case FUCO_NODE_VARIABLE:
        case FUCO_NODE_TYPE_IDENTIFIER:
            fprintf(file, "%s", fuco_token_string(node->token));
            break;

        case FUCO_NODE_INTEGER:
            fprintf(file, "%ld", *(uint64_t *)node->token->data);
            break;

        case FUCO_NODE_RETURN:
            fprintf(file, "return ");

            sub = node->children[FUCO_LAYOUT_RETURN_VALUE];
            fuco_node_unparse_write(sub, file);
            fprintf(file, ";");
            break;

        case FUCO_NODE_IF_ELSE:
            fprintf(file, "if ");

            sub = node->children[FUCO_LAYOUT_IF_ELSE_COND];
            fuco_node_unparse_write(sub, file);
            fprintf(file, " ");

            sub = node->children[FUCO_LAYOUT_IF_ELSE_TRUE_BODY];
            fuco_node_unparse_write(sub, file);
            
            sub = node->children[FUCO_LAYOUT_IF_ELSE_FALSE_BODY];
            if (sub->type != FUCO_NODE_EMPTY) {
                fprintf(file, " else ");
                fuco_node_unparse_write(sub, file);
            }
            break;

        case FUCO_NODE_WHILE:
            fprintf(file, "while ");

            sub = node->children[FUCO_LAYOUT_WHILE_COND];
            fuco_node_unparse_write(sub, file);
            fprintf(file, " ");
            
            sub = node->children[FUCO_LAYOUT_WHILE_BODY];
            fuco_node_unparse_write(sub, file);
            break;
    }
}

void fuco_node_validate(fuco_node_t *node) {
    assert(node != NULL);
    assert(fuco_nodetype_get_layout(node->type) == FUCO_LAYOUT_VARIADIC 
           || (size_t)fuco_nodetype_get_layout(node->type) == node->count);
        
    for (size_t i = 0; i < node->count; i++) {
        assert(node->children[i] != NULL);
    }
}

bool fuco_node_has_type(fuco_node_t *node) {
    switch (node->type) {
        case FUCO_NODE_EMPTY:
        case FUCO_NODE_FILEBODY:
        case FUCO_NODE_BODY:
        case FUCO_NODE_FUNCTION:
        case FUCO_NODE_PARAM_LIST:
        case FUCO_NODE_PARAM:
        case FUCO_NODE_ARG_LIST:
        case FUCO_NODE_RETURN:
        case FUCO_NODE_IF_ELSE:
        case FUCO_NODE_WHILE:
        case FUCO_NODE_TYPE_IDENTIFIER:
            return false;

        case FUCO_NODE_CALL:
        case FUCO_NODE_INSTR:
        case FUCO_NODE_VARIABLE:
        case FUCO_NODE_INTEGER:
            return true;
    }

    FUCO_UNREACHED();
}

fuco_typematch_t fuco_node_type_match(fuco_node_t *node, fuco_node_t *other) {
    switch (node->type) {
        case FUCO_NODE_TYPE_IDENTIFIER:
            assert(other->type == FUCO_NODE_TYPE_IDENTIFIER);

            if (node->symbol->id == other->symbol->id) {
                return FUCO_TYPEMATCH_MATCH;
            } else {
                return FUCO_TYPEMATCH_NOMATCH;
            }

        default:
            FUCO_UNREACHED();
    }
}

/* TODO: better hash function. Use mixing */
fuco_hashvalue_t fuco_node_hash(void *data) {
    fuco_node_t *node = data;

    fuco_hashvalue_t hash = (node->symbol == NULL ? 0 : node->symbol->id)
                            ^ node->type;

    for (size_t i = 0; i < node->count; i++) {
        hash ^= fuco_node_hash(node->children[i]);
    }

    return hash;
}

bool fuco_node_type_equal(void *node, void *other) {
    return fuco_node_type_match(node, other) == FUCO_TYPEMATCH_MATCH;
}

void fuco_node_setup_scopes(fuco_node_t *node, fuco_scope_t *scope) {
    fuco_scope_t *next;

    switch (node->type) {
        case FUCO_NODE_FILEBODY:
        case FUCO_NODE_FUNCTION:
            next = node->data.scope = malloc(sizeof(fuco_scope_t));
            fuco_scope_init(node->data.scope, scope);
            break;

        default:
            next = scope;
            break;
    }

    for (size_t i = 0; i < node->count; i++) {
        fuco_node_setup_scopes(node->children[i], next);
    }
}

fuco_scope_t *fuco_node_get_scope(fuco_node_t *node, fuco_scope_t *outer) {
    fuco_scope_t *scope = NULL;
    
    switch (node->type) {
        case FUCO_NODE_FILEBODY:
        case FUCO_NODE_FUNCTION:
            scope = node->data.scope;
            break;

        default:
            scope = outer;
            break;
    }

    assert(scope != NULL);

    return scope;
}

int fuco_node_gather_datatypes(fuco_node_t *node, fuco_symboltable_t *table, 
                                fuco_scope_t *outer) {
    fuco_scope_t *scope = fuco_node_get_scope(node, outer);

    switch (node->type) {
        default:
            break;
    }

    for (size_t i = 0; i < node->count; i++) {
        if (fuco_node_gather_datatypes(node->children[i], table, scope)) {
            return 1;
        }
    }

    return 0;
}

int fuco_node_resolve_type(fuco_node_t *node, fuco_symboltable_t *table, 
                            fuco_scope_t *outer) {
    /* Types should not have nested scopes, tested in assertion */
    fuco_scope_t *scope = fuco_node_get_scope(node, outer);
    assert(outer == scope);

    FUCO_UNUSED(table), FUCO_UNUSED(outer);

    switch (node->type) {
        case FUCO_NODE_TYPE_IDENTIFIER:
            node->symbol = fuco_scope_lookup_token(scope, node->token);
            if (node->symbol == NULL) {
                return 1;
            }

            if (node->symbol->type != FUCO_SYMBOL_TYPE) {
                fuco_syntax_error(&node->token->source, "expected type");
                return 1;
            }
            break;
        
        default:
            FUCO_UNREACHED();
    }

    return 0;
}

int fuco_node_gather_functions(fuco_node_t *node, fuco_symboltable_t *table, 
                                fuco_scope_t *outer) {
    fuco_scope_t *scope = fuco_node_get_scope(node, outer);
    fuco_node_t *params, *type, *rettype;

    switch (node->type) {
        case FUCO_NODE_FUNCTION:
            node->symbol = fuco_symboltable_insert(table, outer, node->token, 
                                                   node, FUCO_SYMBOL_FUNCTION);
            if (node->symbol == NULL) {
                return 1;
            }

            params = node->children[FUCO_LAYOUT_FUNCTION_PARAMS];
            if (fuco_node_gather_functions(params, table, scope)) {
                return 1;
            }

            rettype = node->children[FUCO_LAYOUT_FUNCTION_RET_TYPE];
            if (fuco_node_resolve_type(rettype, table, outer)) {
                return 1;
            }
            break;

        case FUCO_NODE_PARAM:
            node->symbol = fuco_symboltable_insert(table, scope, 
                                                   node->token, node, 
                                                   FUCO_SYMBOL_VARIABLE);
            if (node->symbol == NULL) {
                return 1;
            }
            
            type = node->children[FUCO_LAYOUT_PARAM_TYPE];
            if (fuco_node_resolve_type(type, table, scope)) {
                return 1;
            }

            node->data.datatype = node->children[FUCO_LAYOUT_PARAM_TYPE];
            break;
            
        default:     
            for (size_t i = 0; i < node->count; i++) {
                if (fuco_node_gather_functions(node->children[i], 
                                                table, scope)) {
                    return 1;
                }
            }
            break;
    }

    return 0;
}

int fuco_node_coerce_type(fuco_node_t **pnode, fuco_node_t *type, 
                          fuco_scope_t *scope) {
    FUCO_UNUSED(scope);
    
    fuco_node_t *node = *pnode;
    assert(node->data.datatype != NULL);

    fuco_typematch_t match = fuco_node_type_match(node->data.datatype, type);
    if (match != FUCO_TYPEMATCH_MATCH) {
        return 1;
    }

    return 0;
}


int fuco_node_resolve_local_propagate(fuco_node_t *node, 
                                      fuco_symboltable_t *table, 
                                      fuco_scope_t *scope, fuco_node_t *ctx) {
    for (size_t i = 0; i < node->count; i++) {
        if (fuco_node_resolve_local(node->children[i], table, scope, ctx)) {
            return 1;
        }
    }

    return 0;
}

int fuco_node_resolve_local_function(fuco_node_t *node, 
                                     fuco_symboltable_t *table, 
                                     fuco_scope_t *scope) {
    assert(node->type == FUCO_NODE_FUNCTION);
    
    fuco_function_def_t *def = NULL;
    node->symbol->value = def;
    
    fuco_node_t *next = node->children[FUCO_LAYOUT_FUNCTION_BODY];
    return fuco_node_resolve_local_propagate(next, table, scope, node);
}

int fuco_node_resolve_local_call(fuco_node_t *node, fuco_symboltable_t *table, 
                                 fuco_scope_t *scope, fuco_node_t *ctx) {
    
    assert(node->type == FUCO_NODE_CALL);
    
    if (fuco_node_resolve_local_propagate(node, table, scope, ctx)) {
        return 1;
    }

    /* FUTURE: enable subscopes to extend overloads instead of only 
       considering most recent overloads */
    fuco_symbol_t *symbol = fuco_scope_lookup_token(scope, node->token);

    if (symbol == NULL) {
        return 1;
    }

    switch (symbol->type) {
        case FUCO_SYMBOL_FUNCTION:
            break;

        case FUCO_SYMBOL_TYPE:
            symbol = symbol->link;

            if (symbol == NULL) {
                fuco_syntax_error(&node->token->source, 
                                  "no conversions present for '%s'", 
                                  fuco_token_string(node->token));
                return 1;
            }
            break;

        case FUCO_SYMBOL_NULL:
            FUCO_UNREACHED();

        case FUCO_SYMBOL_VARIABLE:
            /* FUTURE: calling variables / indirect functions */
            FUCO_NOT_IMPLEMENTED();
    }

    /* TODO message */
    if (symbol->type != FUCO_SYMBOL_FUNCTION) {
        fuco_syntax_error(&node->token->source, "expected function");
        return 1;
    }

    fuco_symbol_t *candidates[FUCO_TYPEMATCH_N];
    bool multiple[FUCO_TYPEMATCH_N];
    for (size_t i = 0; i < FUCO_TYPEMATCH_N; i++) {
        candidates[i] = NULL;
        multiple[i] = false;
    }

    fuco_node_t *args = node->children[FUCO_LAYOUT_CALL_ARGS];

    while (symbol != NULL) {
        fuco_node_t *func = symbol->def;
        fuco_node_t *params = func->children[FUCO_LAYOUT_FUNCTION_PARAMS];
        fuco_typematch_t match = FUCO_TYPEMATCH_MATCH, arg_match;

        fuco_node_t *arg_type, *param, *param_type;

        if (args->count == params->count) {
            for (size_t i = 0; i < args->count; i++) {
                arg_type = args->children[i]->data.datatype;
                param = params->children[i];
                param_type = param->children[FUCO_LAYOUT_PARAM_TYPE];

                assert(arg_type != NULL);
                assert(param_type != NULL);

                arg_match = fuco_node_type_match(arg_type, param_type);
                
                match = FUCO_MIN(match, arg_match);

                /* early termination if not a match */
                if (match == FUCO_TYPEMATCH_NOMATCH) {
                    break;
                }
            }

            if (candidates[match] != NULL) {
                multiple[match] = true;
            }

            candidates[match] = symbol;
        }

        symbol = symbol->link;
    }
    
    /* searches for best matching ('highest') overload */
    for (size_t i = FUCO_TYPEMATCH_N; i > FUCO_TYPEMATCH_NOMATCH + 1; i--) {
        size_t idx = i - 1;
        
        if (candidates[idx] != NULL) {
            if (multiple[idx]) {
                fuco_syntax_error(&node->token->source, 
                                  "multiple candidates for call to '%s'", 
                                  fuco_token_string(node->token));
                return 1;
            } else {
                node->symbol = candidates[idx];
                break;
            }
        }
    }

    if (node->symbol == NULL) {
        fuco_syntax_error(&node->token->source, 
                          "no matching candidate for call to '%s'", 
                          fuco_token_string(node->token));
        return 1;
    }

    fuco_node_t *def = node->symbol->def;
    node->data.datatype = def->children[FUCO_LAYOUT_FUNCTION_RET_TYPE];

    return 0;
}

int fuco_node_resolve_instr(fuco_node_t *node, fuco_symboltable_t *table,
                            fuco_scope_t *scope, fuco_node_t *ctx) {
    assert(node->type == FUCO_NODE_INSTR);
    assert(node->opcode != FUCO_OPCODE_NOP);

    if (fuco_node_resolve_local_propagate(node, table, scope, ctx)) {
        return 1;
    }

    fuco_node_t *args = node->children[FUCO_LAYOUT_INSTR_ARGS];

    size_t arity = fuco_opcode_get_arity(node->opcode);
    for (size_t i = 0; i < arity; i++) {
        fuco_node_t *type = fuco_opcode_get_argtype(node->opcode, table, i);

        if (fuco_node_coerce_type(&args->children[i], type, scope)) {
            return 1;
        }
    }

    node->data.datatype = fuco_opcode_get_rettype(node->opcode, table);;

    return 0;
}

int fuco_node_resolve_local(fuco_node_t *node, fuco_symboltable_t *table, 
                            fuco_scope_t *outer, fuco_node_t *ctx) {        
    fuco_scope_t *scope = fuco_node_get_scope(node, outer);
    
    fuco_node_t *type;

    switch (node->type) {
        case FUCO_NODE_EMPTY:
        case FUCO_NODE_PARAM_LIST:
        case FUCO_NODE_PARAM:
            break;

        case FUCO_NODE_FILEBODY:
        case FUCO_NODE_BODY:
        case FUCO_NODE_ARG_LIST:
            if (fuco_node_resolve_local_propagate(node, table, scope, ctx)) {
                return 1;
            }
            break;

        case FUCO_NODE_FUNCTION:
            if (fuco_node_resolve_local_function(node, table, scope)) {
                return 1;
            }
            break;

        case FUCO_NODE_CALL:
            if (fuco_node_resolve_local_call(node, table, scope, ctx)) {
                return 1;
            }
            break;

        case FUCO_NODE_INSTR:
            if (fuco_node_resolve_instr(node, table, scope, ctx)) {
                return 1;
            }
            break;

        case FUCO_NODE_VARIABLE:
            node->symbol = fuco_scope_lookup_token(scope, node->token);
            if (node->symbol == NULL) {
                return 1;
            }

            if (node->symbol->type != FUCO_SYMBOL_VARIABLE) {
                fuco_syntax_error(&node->token->source, "expected variable");
                return 1;
            }

            node->data.datatype = node->symbol->def->data.datatype;
            break;

        case FUCO_NODE_INTEGER:
            node->data.datatype = fuco_symboltable_get_type(table, 
                                                            FUCO_SYMID_INT);
            break;

        case FUCO_NODE_RETURN:
            if (fuco_node_resolve_local_propagate(node, table, scope, ctx)) {
                return 1;
            }

            type = ctx->children[FUCO_LAYOUT_FUNCTION_RET_TYPE];
            if (fuco_node_coerce_type(&node->children[FUCO_LAYOUT_RETURN_VALUE], 
                                      type, scope)) {
                return 1;
            }
            break;

        case FUCO_NODE_IF_ELSE:
            /* TODO */
            break;

        case FUCO_NODE_WHILE:
            /* TODO */
            break;

        case FUCO_NODE_TYPE_IDENTIFIER:
            node->symbol = fuco_scope_lookup_token(scope, node->token);
            if (node->symbol == NULL) {
                return 1;
            }

            if (node->symbol->type != FUCO_SYMBOL_TYPE) {
                fuco_syntax_error(&node->token->source, "expected type");
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

        case FUCO_NODE_INSTR:
            next = node->children[FUCO_LAYOUT_INSTR_ARGS];
            fuco_node_generate_ir(next, ir, obj);

            fuco_ir_add_instr(ir, obj, node->opcode);
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
            fuco_ir_add_instr_imm48_label(ir, obj, FUCO_OPCODE_RETQ, 
                                          ir->objects[obj].paramsize_label);
            break;

        case FUCO_NODE_IF_ELSE:
            /* TODO */
            break;

        case FUCO_NODE_WHILE:
            /* TODO */
            break;
    }
}

 /* FIXME improve */
size_t fuco_node_setup_offsets(fuco_node_t *node, uint64_t *defs) {
    int64_t offset = 0;

    fuco_node_t *params = node->children[FUCO_LAYOUT_FUNCTION_PARAMS];

    for (size_t i = 0; i < params->count; i++) {
        fuco_node_t *param = params->children[i];

        defs[param->symbol->id] = -3 * sizeof(uint64_t) - offset;
        offset += 8;
    }

    return offset;
}
