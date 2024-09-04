#include "compiler.h"
#include "lexer.h"
#include "tokenlist.h"
#include "utils.h"

void fuco_compiler_init(fuco_compiler_t *compiler, char *filename) {
    fuco_lexer_init(&compiler->lexer);
    fuco_parser_init(&compiler->parser);
    fuco_symboltable_init(&compiler->table);
    fuco_ir_init(&compiler->ir);
    fuco_bytecode_init(&compiler->bytecode);
    compiler->root = NULL;
    compiler->filename = filename;
}

void fuco_compiler_destruct(fuco_compiler_t *compiler) {
    fuco_lexer_destruct(&compiler->lexer);
    fuco_parser_destruct(&compiler->parser);
    fuco_symboltable_destruct(&compiler->table);
    fuco_ir_destruct(&compiler->ir);
    fuco_bytecode_destruct(&compiler->bytecode);

    if (compiler->root != NULL) {
        fuco_node_free(compiler->root);
    }
}

int fuco_compiler_run(fuco_compiler_t *compiler) {
    fuco_lexer_add_job(&compiler->lexer, compiler->filename);

    fuco_tstream_t tstream = fuco_lexer_lex(&compiler->lexer);
    
    if (tstream == NULL) {
        return 1;
    }
    
    compiler->parser.tstream = tstream;
    fuco_parser_setup_instrs(&compiler->parser);

    compiler->root = fuco_parse_filebody(&compiler->parser);
    if (compiler->root == NULL) {
        return 1;
    }

    fuco_node_setup_scopes(compiler->root, NULL);

    fuco_scope_t *global = fuco_node_get_scope(compiler->root, NULL);

    fuco_symboltable_setup(&compiler->table, global);

    if (fuco_node_gather_datatypes(compiler->root, &compiler->table, NULL)) {
        return 1;
    }

    if (fuco_node_gather_functions(compiler->root, &compiler->table, NULL)) {
        return 1;
    }

    if (fuco_node_resolve_local(compiler->root, &compiler->table, NULL, NULL)) {
        return 1;
    }

    fuco_symbol_t *entry;
    if ((entry = fuco_scope_lookup(global, "main", NULL, false)) == NULL) {
        fuco_syntax_error(NULL, "entry point '%s' was not defined", "main");
        return 1;
    }

    compiler->ir.label = compiler->table.size;
    fuco_ir_create_startup_object(&compiler->ir, entry->id);
    fuco_node_generate_ir(compiler->root, &compiler->ir, 0);
    
    fuco_ir_assemble(&compiler->ir, &compiler->bytecode);

    if (compiler->bytecode.instrs == NULL) {
        return 1;
    }
    
    fuco_node_unparse_write(compiler->root, stderr);
    fprintf(stderr, "\n");

    fuco_node_pretty_write(compiler->root, stderr);

    fuco_symboltable_write(&compiler->table, stderr);

    fuco_ir_write(&compiler->ir, stderr);

    fuco_bytecode_write(&compiler->bytecode, stderr);

    return 0;
}
