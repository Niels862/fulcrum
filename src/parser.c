#include "parser.h"
#include "utils.h"
#include <stdlib.h>

fuco_node_t *fuco_parse_filebody(fuco_tokenizer_t *tokenizer) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_LIST, FUCO_LAYOUT_VARIADIC);

    while (true) {
        while (tokenizer->curr.type == FUCO_TOKEN_EOF) {
            if (fuco_queue_is_empty(&tokenizer->sources)) {
                return node;
            }

            fuco_tokenizer_open_next_source(tokenizer);
        }

        fuco_node_t *sub = fuco_parse_function_declaration(tokenizer);
        if (sub == NULL) {
            fuco_node_free(node);
            return NULL;
        }

        node = fuco_node_add_child(node, sub);
    }
}

fuco_node_t *fuco_parse_function_declaration(fuco_tokenizer_t *tokenizer) {
    if (!fuco_tokenizer_expect(tokenizer, FUCO_TOKEN_DEF)) {
        return NULL;
    }
    fuco_tokenizer_discard(tokenizer);

    fuco_node_t *node = fuco_node_new(FUCO_NODE_FUNCTION, 
                                      FUCO_LAYOUT_FUNCTION_N);
    
    if (!fuco_tokenizer_expect(tokenizer, FUCO_TOKEN_IDENTIFIER)) {
        free(node);
        return NULL;
    }
    fuco_tokenizer_move(tokenizer, node);    
    
    return node;
}
