#include "parser.h"
#include "utils.h"
#include <stdlib.h>
#include <assert.h>

void fuco_parser_init(fuco_parser_t *parser) {
    parser->tstream = NULL;
    fuco_map_init(&parser->instrs, fuco_hash_string, fuco_equal_string, 
                  NULL, NULL);
}

void fuco_parser_destruct(fuco_parser_t *parser) {
    fuco_map_destruct(&parser->instrs);
}

void fuco_parser_setup_instrs(fuco_parser_t *parser) {
    static fuco_node_instr_t instrs[] = {
        { .opcode = FUCO_OPCODE_IADD },
        { .opcode = FUCO_OPCODE_ISUB },
        { .opcode = FUCO_OPCODE_IMUL },
        { .opcode = FUCO_OPCODE_IDIV },
        { .opcode = FUCO_OPCODE_IMOD },
    };

    for (size_t i = 0; i < sizeof(instrs) / sizeof(*instrs); i++) {
        instrs[i].in1 = instrs[i].in2 = instrs[i].ret = FUCO_SYMID_INT;
        instrs->count = 2;

        char *mnemonic = fuco_opcode_get_mnemonic(instrs[i].opcode);
        void **result = fuco_map_insert(&parser->instrs, mnemonic, &instrs[i]);

        assert(result == NULL);
    }
}

void fuco_parser_advance(fuco_parser_t *parser) {
    if (parser->tstream->type != FUCO_TOKEN_END_OF_SOURCE) {
        parser->tstream++;
    }
}

void fuco_parser_move(fuco_parser_t *parser, fuco_node_t *node) {
    assert(node->token == NULL);

    node->token = parser->tstream;
}

bool fuco_parser_accept(fuco_parser_t *parser, fuco_tokentype_t type, 
                        fuco_node_t *node) {
    if (parser->tstream->type != type) {
        return false;
    }

    if (node != NULL) {
        fuco_parser_move(parser, node);
    }

    fuco_parser_advance(parser);

    return true;
}

bool fuco_parser_expect(fuco_parser_t *parser, fuco_tokentype_t type, 
                        fuco_node_t *node) {
    if (parser->tstream->type != type) {
        fuco_syntax_error(&parser->tstream->source, "expected %s, but got %s", 
                          fuco_tokentype_string(type), 
                          fuco_token_string(parser->tstream));

        return false;
    }
    
    if (node != NULL) {
        fuco_parser_move(parser, node);
    }

    fuco_parser_advance(parser);

    return true;
}

int fuco_parser_lookup_instr(fuco_parser_t *parser, fuco_node_t *node) {
    assert(parser->tstream->type == FUCO_TOKEN_IDENTIFIER);
    assert(node->type == FUCO_NODE_INSTR);

    char *mnemonic = parser->tstream->lexeme;
    void **result = fuco_map_lookup(&parser->instrs, mnemonic);

    if (result == NULL) {
        fuco_syntax_error(&parser->tstream->source, 
                          "unrecognized instruction: '%s'", mnemonic);
        return 1;
    }

    fuco_node_instr_t *mapped_instr = *result;

    assert(mapped_instr != NULL);

    node->instr = *mapped_instr;

    printf("(%d)\n", node->instr.opcode);

    return 0;
}

bool fuco_parser_check(fuco_parser_t *parser, fuco_tokentype_t type) {
    return parser->tstream->type == type;
}

fuco_node_t *fuco_parse_filebody(fuco_parser_t *parser) {
    size_t allocated;
    fuco_node_t *node = fuco_node_variadic_new(FUCO_NODE_FILEBODY, &allocated);

    fuco_parser_expect(parser, FUCO_TOKEN_START_OF_SOURCE, NULL);

    while (!fuco_parser_accept(parser, FUCO_TOKEN_END_OF_SOURCE, NULL)) {
        if (!fuco_parser_accept(parser, FUCO_TOKEN_END_OF_FILE, NULL)) {
            fuco_node_t *sub = fuco_parse_function_declaration(parser);
            if (sub == NULL) {
                fuco_node_free(node);
                return NULL;
            }

            node = fuco_node_add_child(node, sub, &allocated);
        }
    }

    return node;
}

fuco_node_t *fuco_parse_function_declaration(fuco_parser_t *parser) {
    if (!fuco_parser_expect(parser, FUCO_TOKEN_DEF, NULL)) {
        return NULL;
    }

    fuco_node_t *node = fuco_node_new(FUCO_NODE_FUNCTION);
    fuco_node_t *params = NULL, *body = NULL, *ret_type = NULL;

    if (!fuco_parser_expect(parser, FUCO_TOKEN_IDENTIFIER, node)
        || (params = fuco_parse_param_list(parser)) == NULL
        || !fuco_parser_expect(parser, FUCO_TOKEN_ARROW, NULL)
        || (ret_type = fuco_parse_type(parser)) == NULL
        || (body = fuco_parse_braced_block(parser)) == NULL) {
        fuco_node_free(node);

        if (params != NULL) {
            fuco_node_free(params);
        }

        if (body != NULL) {
            fuco_node_free(body);
        }

        if (ret_type != NULL) {
            fuco_node_free(ret_type);
        }

        return NULL;
    }

    fuco_node_set_child(node, params, FUCO_LAYOUT_FUNCTION_PARAMS);
    fuco_node_set_child(node, ret_type, FUCO_LAYOUT_FUNCTION_RET_TYPE);
    fuco_node_set_child(node, body, FUCO_LAYOUT_FUNCTION_BODY);

    return node;
}

fuco_node_t *fuco_parse_param_list(fuco_parser_t *parser) {
    size_t allocated;
    fuco_node_t *node = fuco_node_variadic_new(FUCO_NODE_PARAM_LIST, 
                                               &allocated);
    fuco_node_t *param = NULL;
    
    if (!fuco_parser_expect(parser, FUCO_TOKEN_BRACKET_OPEN, NULL)) {
        fuco_node_free(node);
        return NULL;
    }

    if (!fuco_parser_check(parser, FUCO_TOKEN_BRACKET_CLOSE)) {
        do {
            if ((param = fuco_parse_param(parser)) == NULL) {
                fuco_node_free(node);
                return NULL;
            }

            node = fuco_node_add_child(node, param, &allocated);

            if (!fuco_parser_accept(parser, FUCO_TOKEN_COMMA, NULL)) {
                break;
            }
        } while (true);
    }

    if (!fuco_parser_expect(parser, FUCO_TOKEN_BRACKET_CLOSE, NULL)) {
        fuco_node_free(node);
        return NULL;
    }

    return node;
}

fuco_node_t *fuco_parse_param(fuco_parser_t *parser) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_PARAM);
    fuco_node_t *type = NULL;

    if (!fuco_parser_expect(parser, FUCO_TOKEN_IDENTIFIER, node)
        || !fuco_parser_expect(parser, FUCO_TOKEN_COLON, NULL)
        || (type = fuco_parse_type(parser)) == NULL) {
        fuco_node_free(node);
        return NULL;
    }

    fuco_node_set_child(node, type, FUCO_LAYOUT_PARAM_TYPE);

    return node;
}

fuco_node_t *fuco_parse_braced_block(fuco_parser_t *parser) {
    if (!fuco_parser_expect(parser, FUCO_TOKEN_BRACE_OPEN, NULL)) {
        return NULL;
    }

    size_t allocated;
    fuco_node_t *node = fuco_node_variadic_new(FUCO_NODE_BODY, &allocated);

    while (!fuco_parser_accept(parser, FUCO_TOKEN_BRACE_CLOSE, NULL)) {
        fuco_node_t *sub = fuco_parse_body_statement(parser);
        
        if (sub == NULL) {        
            fuco_node_free(node);

            return NULL;
        }

        node = fuco_node_add_child(node, sub, &allocated);
    }
    
    return node;
}

fuco_node_t *fuco_parse_body_statement(fuco_parser_t *parser) {
    fuco_node_t *node;

    switch (parser->tstream->type) {
        case FUCO_TOKEN_RETURN:
            node = fuco_parse_return(parser);
            break;

        default:
            node = NULL;
            break;
    }

    return node;
}

fuco_node_t *fuco_parse_return(fuco_parser_t *parser) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_RETURN);
    fuco_node_t *value;
    
    if (!fuco_parser_expect(parser, FUCO_TOKEN_RETURN, node)
        || (value = fuco_parse_expression(parser)) == NULL) {
        fuco_node_free(node);
        return NULL;
    }

    fuco_node_set_child(node, value, FUCO_LAYOUT_RETURN_VALUE);

    if (!fuco_parser_expect(parser, FUCO_TOKEN_SEMICOLON, NULL)) {
        fuco_node_free(node);
        return NULL;
    }
    
    return node;
}

fuco_node_t *fuco_parse_expression(fuco_parser_t *parser) {
    return fuco_parse_value(parser);
}

fuco_node_t *fuco_parse_value(fuco_parser_t *parser) {
    fuco_node_t *node = NULL;

    switch (parser->tstream->type) {
        case FUCO_TOKEN_INTEGER:
            node = fuco_node_new(FUCO_NODE_INTEGER);
            fuco_parser_move(parser, node);
            fuco_parser_advance(parser);
            break;

        case FUCO_TOKEN_IDENTIFIER:
            node = fuco_node_new(FUCO_NODE_VARIABLE);
            fuco_parser_move(parser, node);
            fuco_parser_advance(parser);

            if (fuco_parser_check(parser, FUCO_TOKEN_BRACKET_OPEN)) {
                fuco_node_t *args = fuco_parse_args(parser);

                if (args == NULL) {
                    fuco_node_free(node);
                    return NULL;
                }

                node = fuco_node_transform(node, FUCO_NODE_CALL);

                fuco_node_set_child(node, args, FUCO_LAYOUT_CALL_ARGS);
            }
            break;

        case FUCO_TOKEN_PERCENT:
            fuco_parser_advance(parser);

            node = fuco_node_new(FUCO_NODE_INSTR);

            if (fuco_parser_lookup_instr(parser, node)) {
                fuco_node_free(node);
                return NULL;
            }

            fuco_parser_move(parser, node);
            fuco_parser_advance(parser);

            fuco_node_t *args = fuco_parse_args(parser);

            if (args == NULL) {
                fuco_node_free(node);
                return NULL;
            }

            fuco_node_set_child(node, args, FUCO_LAYOUT_INSTR_ARGS);
            break;

        default:
            fuco_syntax_error(&parser->tstream->source, 
                              "expected value, but got %s", 
                              fuco_tokentype_string(parser->tstream->type));
            break;
    }

    return node;
}

fuco_node_t *fuco_parse_args(fuco_parser_t *parser) {
    size_t allocated;
    fuco_node_t *node = fuco_node_variadic_new(FUCO_NODE_ARG_LIST, &allocated);
    fuco_node_t *arg;

    if (!fuco_parser_expect(parser, FUCO_TOKEN_BRACKET_OPEN, NULL)) {
        fuco_node_free(node);
        return NULL;
    }

    if (!fuco_parser_check(parser, FUCO_TOKEN_BRACKET_CLOSE)) {
        do {
            if ((arg = fuco_parse_value(parser)) == NULL) {
                fuco_node_free(node);
                return NULL;
            }

            node = fuco_node_add_child(node, arg, &allocated);

            if (!fuco_parser_accept(parser, FUCO_TOKEN_COMMA, NULL)) {
                break;
            }
        } while (true);
    }

    if (!fuco_parser_expect(parser, FUCO_TOKEN_BRACKET_CLOSE, NULL)) {
        fuco_node_free(node);
        return NULL;
    }

    return node;
}

fuco_node_t *fuco_parse_type(fuco_parser_t *parser) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_TYPE_IDENTIFIER);

    if (!fuco_parser_expect(parser, FUCO_TOKEN_IDENTIFIER, node)) {
        fuco_node_free(node);
        return NULL;
    }

    return node;
}
