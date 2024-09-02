#include "parser.h"
#include "utils.h"
#include "strutils.h"
#include <stdlib.h>
#include <assert.h>

fuco_operator_specification_t fuco_operator_specs[] = {
    { .associativity = FUCO_ASSOCIATIVE_LEFT,
      { FUCO_TOKEN_EQ, FUCO_TOKEN_NE } 
    }, 
    { .associativity = FUCO_ASSOCIATIVE_LEFT,
      { FUCO_TOKEN_GT, FUCO_TOKEN_GE, FUCO_TOKEN_LT, FUCO_TOKEN_LE } 
    },
    { .associativity = FUCO_ASSOCIATIVE_LEFT, 
      { FUCO_TOKEN_PLUS, FUCO_TOKEN_MINUS } 
    },
    { .associativity = FUCO_ASSOCIATIVE_LEFT, 
      { FUCO_TOKEN_ASTERISK, FUCO_TOKEN_SLASH, FUCO_TOKEN_PERCENT } 
    }
};

void fuco_parser_init(fuco_parser_t *parser) {
    parser->tstream = NULL;
    fuco_map_init(&parser->instrs, fuco_string_hash, fuco_string_equal, 
                  NULL, NULL);
}

void fuco_parser_destruct(fuco_parser_t *parser) {
    fuco_map_destruct(&parser->instrs);
}

void fuco_parser_setup_instrs(fuco_parser_t *parser) {
    static fuco_opcode_t instrs[] = {
        FUCO_OPCODE_IADD,
        FUCO_OPCODE_ISUB,
        FUCO_OPCODE_IMUL,
        FUCO_OPCODE_IDIV,
        FUCO_OPCODE_IMOD,
        FUCO_OPCODE_IEQ,
        FUCO_OPCODE_INE,
        FUCO_OPCODE_ILT,
        FUCO_OPCODE_ILE,
        FUCO_OPCODE_IGT,
        FUCO_OPCODE_IGE
    };

    for (size_t i = 0; i < sizeof(instrs) / sizeof(*instrs); i++) {
        char *mnemonic = fuco_opcode_get_mnemonic(instrs[i]);
        void **result = fuco_map_insert(&parser->instrs, mnemonic, &instrs[i]);

        assert(result == NULL);
    }
}

fuco_token_t *fuco_parser_advance(fuco_parser_t *parser) {
    fuco_token_t *last = parser->tstream;

    if (parser->tstream->type != FUCO_TOKEN_END_OF_SOURCE) {
        parser->tstream++;
    }

    return last;
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
                          fuco_token_static_string(parser->tstream));

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

    char *mnemonic =  fuco_token_string(parser->tstream);
    void **result = fuco_map_lookup(&parser->instrs, mnemonic);

    if (result == NULL) {
        fuco_syntax_error(&parser->tstream->source, 
                          "unrecognized instruction: '%s'", mnemonic);
        return 1;
    }

    fuco_opcode_t *mapped_instr = *result;

    assert(mapped_instr != NULL);

    node->opcode = *mapped_instr;

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
    bool success = true;

    bool inlined = fuco_parser_accept(parser, FUCO_TOKEN_INLINE, NULL);
    FUCO_UNUSED(inlined);

    if (fuco_parser_accept(parser, FUCO_TOKEN_SQBRACKET_OPEN, NULL)) {
        fuco_parser_move(parser, node);
        fuco_parser_advance(parser);

        success = fuco_parser_expect(parser, FUCO_TOKEN_SQBRACKET_CLOSE, NULL);
    } else {
        success = fuco_parser_expect(parser, FUCO_TOKEN_IDENTIFIER, node);
    }

    success = success 
              && (params = fuco_parse_param_list(parser)) != NULL
              && fuco_parser_expect(parser, FUCO_TOKEN_ARROW, NULL)
              && (ret_type = fuco_parse_type(parser)) != NULL
              && (body = fuco_parse_braced_block(parser)) != NULL;

    if (!success) {
        fuco_node_free(node);
        fuco_node_free(params);
        fuco_node_free(body);
        fuco_node_free(ret_type);

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

        case FUCO_TOKEN_IF:
            node = fuco_parse_if_else(parser);
            break;

        case FUCO_TOKEN_WHILE:
            node = fuco_parse_while(parser);
            break;

        default:
            fuco_syntax_error(&parser->tstream->source, 
                              "expected statement, but got %s", 
                              fuco_token_static_string(parser->tstream));

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

fuco_node_t *fuco_parse_if_else(fuco_parser_t *parser) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_IF_ELSE);
    fuco_node_t *cond = NULL, *true_body = NULL, *false_body = NULL;
    
    bool success = fuco_parser_expect(parser, FUCO_TOKEN_IF, node)
                   && (cond = fuco_parse_expression(parser)) != NULL
                   && (true_body = fuco_parse_braced_block(parser)) != NULL;
    
    if (success) {
        if (fuco_parser_accept(parser, FUCO_TOKEN_ELSE, NULL)) {
            success = (false_body = fuco_parse_braced_block(parser)) != NULL;
        } else {
            false_body = &fuco_node_empty;
        }
    }

    if (!success) {
        fuco_node_free(node);
        fuco_node_free(node);
        fuco_node_free(true_body);

        return NULL;
    }

    fuco_node_set_child(node, cond, FUCO_LAYOUT_IF_ELSE_COND);
    fuco_node_set_child(node, true_body, FUCO_LAYOUT_IF_ELSE_TRUE_BODY);
    fuco_node_set_child(node, false_body, FUCO_LAYOUT_IF_ELSE_FALSE_BODY);

    return node;
}

fuco_node_t *fuco_parse_while(fuco_parser_t *parser) {
    fuco_node_t *node = fuco_node_new(FUCO_NODE_WHILE);
    fuco_node_t *cond = NULL, *body = NULL;

    if (!fuco_parser_expect(parser, FUCO_TOKEN_WHILE, node)
        || (cond = fuco_parse_expression(parser)) == NULL
        || (body = fuco_parse_braced_block(parser)) == NULL) {
        fuco_node_free(node);
        fuco_node_free(node);
        fuco_node_free(node);

        return NULL;
    }

    fuco_node_set_child(node, cond, FUCO_LAYOUT_WHILE_COND);
    fuco_node_set_child(node, body, FUCO_LAYOUT_WHILE_BODY);

    return node;
}

fuco_node_t *fuco_parse_expression(fuco_parser_t *parser) {
    return fuco_parse_operator(parser, 0);
}

fuco_node_t *fuco_parse_operator(fuco_parser_t *parser, size_t level) {
    if (level >= FUCO_ARRAY_SIZE(fuco_operator_specs)) {
        return fuco_parse_value(parser);
    }

    if (fuco_operator_specs[level].associativity == FUCO_ASSOCIATIVE_LEFT) {
        return fuco_parse_operator_left(parser, level);
    }

    return fuco_parse_operator_right(parser, level);
}

fuco_node_t *fuco_parse_operator_left(fuco_parser_t *parser, size_t level) {
    fuco_node_t *left, *right;

    if ((left = fuco_parse_operator(parser, level + 1)) == NULL) {
        return NULL;
    }

    fuco_operator_specification_t *spec = &fuco_operator_specs[level];
    
    while (true) {
        fuco_token_t *operator = NULL;

        for (size_t i = 0; i < FUCO_MAX_OPERATORS_PER_LEVEL; i++) {
            if (fuco_parser_check(parser, spec->operators[i])) {
                operator = fuco_parser_advance(parser);
                break;
            }
        }

        if (operator != NULL) {
            if ((right = fuco_parse_operator(parser, level + 1)) == NULL) {
                fuco_node_free(left);
                return NULL;
            }
            
            left = fuco_node_call_new(2, left, right);
            left->token = operator;
        } else {
            return left;
        }
    }
}

fuco_node_t *fuco_parse_operator_right(fuco_parser_t *parser, size_t level) {
    FUCO_UNUSED(level);

    FUCO_NOT_IMPLEMENTED();

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

        case FUCO_TOKEN_BRACKET_OPEN:
            fuco_parser_advance(parser);

            if ((node = fuco_parse_expression(parser)) == NULL) {
                return NULL;
            }

            if (!fuco_parser_expect(parser, FUCO_TOKEN_BRACKET_CLOSE, NULL)) {
                fuco_node_free(node);
                return NULL;
            }
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
            if ((arg = fuco_parse_expression(parser)) == NULL) {
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
