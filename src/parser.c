#include "parser.h"
#include "utils.h"
#include <stdlib.h>

fuco_node_t *fuco_parse_filebody(fuco_tokenizer_t *tokenizer) {
    size_t allocated;
    fuco_node_t *node = fuco_node_variadic_new(FUCO_NODE_LIST, &allocated);

    while (true) {
        while (tokenizer->curr.type == FUCO_TOKEN_EOF) {
            if (fuco_queue_is_empty(&tokenizer->sources)) {
                return node;
            }

            if (fuco_tokenizer_open_next_source(tokenizer)) {
                fuco_node_free(node);
                return NULL;
            }
        }

        fuco_node_t *sub = fuco_parse_function_declaration(tokenizer);
        if (sub == NULL) {
            fuco_node_free(node);
            return NULL;
        }

        node = fuco_node_add_child(node, sub, &allocated);
    }
}

fuco_node_t *fuco_parse_function_declaration(fuco_tokenizer_t *tokenizer) {
    if (!fuco_tokenizer_discard_if(tokenizer, FUCO_TOKEN_DEF)) {
        return NULL;
    }

    fuco_node_t *node = fuco_node_new(FUCO_NODE_FUNCTION);
    fuco_node_t *body;

    if (!fuco_tokenizer_move_if(tokenizer, FUCO_TOKEN_IDENTIFIER, node)
        || !fuco_tokenizer_discard_if(tokenizer, FUCO_TOKEN_BRACKET_OPEN)
        || !fuco_tokenizer_discard_if(tokenizer, FUCO_TOKEN_BRACKET_CLOSE)
        || (body = fuco_parse_braced_block(tokenizer)) == NULL) {

        fuco_node_free(node);
        return NULL;
    }

    fuco_node_set_child(node, fuco_node_new(FUCO_NODE_EMPTY), 
                        FUCO_LAYOUT_FUNCTION_TYPE);
    fuco_node_set_child(node, fuco_node_new(FUCO_NODE_EMPTY), 
                        FUCO_LAYOUT_FUNCTION_PARAMS);
    fuco_node_set_child(node, body, FUCO_LAYOUT_FUNCTION_BODY);

    return node;
}

fuco_node_t *fuco_parse_braced_block(fuco_tokenizer_t *tokenizer) {
    if (!fuco_tokenizer_discard_if(tokenizer, FUCO_TOKEN_BRACE_OPEN)) {
        return NULL;
    }

    size_t allocated;
    fuco_node_t *node = fuco_node_variadic_new(FUCO_NODE_LIST, &allocated);

    while (tokenizer->curr.type != FUCO_TOKEN_BRACE_CLOSE 
           && tokenizer->curr.type != FUCO_TOKEN_EOF) {
        fuco_node_t *sub = fuco_parse_body_statement(tokenizer);
        
        if (sub == NULL) {        
            fuco_node_free(node);

            return NULL;
        }

        node = fuco_node_add_child(node, sub, &allocated);
    }

    if (!fuco_tokenizer_discard_if(tokenizer, FUCO_TOKEN_BRACE_CLOSE)) {
        fuco_node_free(node);
        return NULL;
    }
    
    return node;
}

fuco_node_t *fuco_parse_body_statement(fuco_tokenizer_t *tokenizer) {
    fuco_node_t *node;

    switch (tokenizer->curr.type) {
        case FUCO_TOKEN_RETURN:
            node = fuco_parse_return(tokenizer);
            break;

        default:
            node = NULL;
            break;
    }

    return node;
}

fuco_node_t *fuco_parse_return(fuco_tokenizer_t *tokenizer) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_RETURN);
    fuco_node_t *value;
    
    if (!fuco_tokenizer_move_if(tokenizer, FUCO_TOKEN_RETURN, node)
        || (value = fuco_parse_expression(tokenizer)) == NULL) {
        fuco_node_free(node);
        return NULL;
    }

    fuco_node_set_child(node, value, FUCO_LAYOUT_RETURN_VALUE);

    if (!fuco_tokenizer_discard_if(tokenizer, FUCO_TOKEN_SEMICOLON)) {
        fuco_node_free(node);
        return NULL;
    }
    
    return node;
}

fuco_node_t *fuco_parse_expression(fuco_tokenizer_t *tokenizer) {
    return fuco_parse_value(tokenizer);
}

fuco_node_t *fuco_parse_value(fuco_tokenizer_t *tokenizer) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_INTEGER);

    if (!fuco_tokenizer_move_if(tokenizer, FUCO_TOKEN_INTEGER, node)) {
        fuco_node_free(node);
        return NULL;
    }

    return node;
}
