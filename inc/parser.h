#ifndef FUCO_PARSER_H
#define FUCO_PARSER_H

#include "tokenizer.h"
#include "tree.h"

fuco_node_t *fuco_parse_filebody(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_function_declaration(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_param_list(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_param(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_braced_block(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_body_statement(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_return(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_expression(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_value(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_call(fuco_tokenizer_t *tokenizer, fuco_node_t *node, 
                             size_t *allocated);

#endif
