#ifndef FUCO_COMPILER_H
#define FUCO_COMPILER_H

#include "tokenizer.h"
#include "lexer.h"
#include "parser.h"
#include "symbol.h"
#include "instruction.h"
#include "tree.h"

typedef struct {
    fuco_lexer_t lexer;

    /* To be replaced */
    fuco_tokenizer_t tokenizer;
    
    fuco_parser_t parser;
    fuco_symboltable_t table;
    fuco_scope_t global;
    fuco_ir_t ir;
    fuco_bytecode_t bytecode;
    fuco_node_t *root;
    char *filename;
} fuco_compiler_t;

void fuco_compiler_init(fuco_compiler_t *compiler, char *filename);

void fuco_compiler_destruct(fuco_compiler_t *compiler);

int fuco_compiler_run(fuco_compiler_t *compiler);

#endif
