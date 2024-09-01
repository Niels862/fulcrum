#ifndef FUCO_PARSER_H
#define FUCO_PARSER_H

#include "tokenlist.h"
#include "tree.h"

typedef enum {
    FUCO_ASSOCIATIVE_LEFT,
    FUCO_ASSOCIATIVE_RIGHT
} fuco_operator_associativity_t;

#define FUCO_MAX_OPERATORS_PER_LEVEL 4

typedef struct {
    fuco_operator_associativity_t associativity;
    fuco_tokentype_t operators[FUCO_MAX_OPERATORS_PER_LEVEL];
} fuco_operator_specification_t;

typedef struct {
    fuco_tstream_t tstream;
    fuco_map_t instrs;
} fuco_parser_t;

extern fuco_operator_specification_t fuco_operator_specs[];

void fuco_parser_init(fuco_parser_t *parser);

void fuco_parser_destruct(fuco_parser_t *parser);

void fuco_parser_setup_instrs(fuco_parser_t *parser);

fuco_token_t *fuco_parser_advance(fuco_parser_t *parser);

void fuco_parser_move(fuco_parser_t *parser, fuco_node_t *node);

bool fuco_parser_accept(fuco_parser_t *parser, fuco_tokentype_t type, 
                        fuco_node_t *node);

bool fuco_parser_expect(fuco_parser_t *parser, fuco_tokentype_t type, 
                        fuco_node_t *node);

int fuco_parser_lookup_instr(fuco_parser_t *parser, fuco_node_t *node);

fuco_node_t *fuco_parse_filebody(fuco_parser_t *parser);

fuco_node_t *fuco_parse_function_declaration(fuco_parser_t *parser);

fuco_node_t *fuco_parse_param_list(fuco_parser_t *parser);

fuco_node_t *fuco_parse_param(fuco_parser_t *parser);

fuco_node_t *fuco_parse_braced_block(fuco_parser_t *parser);

fuco_node_t *fuco_parse_body_statement(fuco_parser_t *parser);

fuco_node_t *fuco_parse_return(fuco_parser_t *parser);

fuco_node_t *fuco_parse_if_else(fuco_parser_t *parser);

fuco_node_t *fuco_parse_while(fuco_parser_t *parser);

fuco_node_t *fuco_parse_expression(fuco_parser_t *parser);

fuco_node_t *fuco_parse_operator(fuco_parser_t *parser, size_t level);

fuco_node_t *fuco_parse_operator_left(fuco_parser_t *parser, size_t level);

fuco_node_t *fuco_parse_operator_right(fuco_parser_t *parser, size_t level);

fuco_node_t *fuco_parse_value(fuco_parser_t *parser);

fuco_node_t *fuco_parse_args(fuco_parser_t *parser);

fuco_node_t *fuco_parse_type(fuco_parser_t *parser);

#endif
