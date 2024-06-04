#ifndef FUCO_PARSER_H
#define FUCO_PARSER_H

#include "tokenizer.h"
#include "tree.h"

typedef struct {
    fuco_tokenizer_t tokenizer;
} fuco_parser_t;

void fuco_parser_init(fuco_parser_t *parser);

void fuco_parser_destruct(fuco_parser_t *parser);

fuco_node_t *fuco_parser_parse_filebody(fuco_parser_t *parser);

fuco_node_t *fuco_parser_parse_function_declaration(fuco_parser_t *parser);

#endif
