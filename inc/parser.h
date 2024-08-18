#ifndef FUCO_PARSER_H
#define FUCO_PARSER_H

#include "tokenlist.h"
#include "tree.h"

typedef struct {
    fuco_tstream_t tstream;
    fuco_map_t instrs;
} fuco_parser_t;

void fuco_parser_init(fuco_parser_t *parser);

void fuco_parser_destruct(fuco_parser_t *parser);

void fuco_parser_setup_instrs(fuco_parser_t *parser);

void fuco_parser_advance(fuco_parser_t *parser);

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

fuco_node_t *fuco_parse_expression(fuco_parser_t *parser);

fuco_node_t *fuco_parse_value(fuco_parser_t *parser);

fuco_node_t *fuco_parse_args(fuco_parser_t *parser);

fuco_node_t *fuco_parse_type(fuco_parser_t *parser);

#endif
