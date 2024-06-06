#ifndef FUCO_PARSER_H
#define FUCO_PARSER_H

#include "tokenizer.h"
#include "tree.h"

fuco_node_t *fuco_parse_filebody(fuco_tokenizer_t *tokenizer);

fuco_node_t *fuco_parse_function_declaration(fuco_tokenizer_t *tokenizer);

#endif
