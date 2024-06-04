#include "parser.h"
#include "utils.h"

void fuco_parser_init(fuco_parser_t *parser) {
    fuco_tokenizer_init(&parser->tokenizer);

    fuco_tokenizer_add_source_filename(&parser->tokenizer, 
                                       fuco_strdup("tests/main.fc"));
}

void fuco_parser_destruct(fuco_parser_t *parser) {
    fuco_tokenizer_destruct(&parser->tokenizer);
}

fuco_node_t *fuco_parser_parse_filebody(fuco_parser_t *parser) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_LIST, FUCO_LAYOUT_VARIADIC);

    while (true) {
        while (parser->tokenizer.curr.type == FUCO_TOKEN_EOF) {
            if (fuco_queue_is_empty(&parser->tokenizer.sources)) {
                return node;
            }

            fuco_tokenizer_discard(&parser->tokenizer);
            fuco_tokenizer_open_next_source(&parser->tokenizer);
            fuco_tokenizer_next_token(&parser->tokenizer);
        }

        fuco_node_t *sub = fuco_parser_parse_function_declaration(parser);
        if (sub == NULL) {
            fuco_node_free(node);
            return NULL;
        }

        node = fuco_node_add_child(node, sub);
    }
}

fuco_node_t *fuco_parser_parse_function_declaration(fuco_parser_t *parser) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_FUNCTION, 
                                      FUCO_LAYOUT_FUNCTION_N);

    fuco_tokenizer_move(&parser->tokenizer, node);    
    fuco_tokenizer_next_token(&parser->tokenizer);

    return node;
}
