#include "compiler.h"
#include "parser.h"
#include "utils.h"

void fuco_compiler_init(fuco_compiler_t *compiler, char *file) {
    fuco_tokenizer_init(&compiler->tokenizer);
    fuco_symboltable_init(&compiler->table);
    fuco_scope_init(&compiler->global, NULL);
    fuco_ir_init(&compiler->ir);
    fuco_bytecode_init(&compiler->bytecode);
    compiler->root = NULL;
    compiler->file = file;
}

void fuco_compiler_destruct(fuco_compiler_t *compiler) {
    fuco_tokenizer_destruct(&compiler->tokenizer);
    fuco_symboltable_destruct(&compiler->table);
    fuco_scope_destruct(&compiler->global);
    fuco_ir_destruct(&compiler->ir);
    fuco_bytecode_destruct(&compiler->bytecode);

    if (compiler->root != NULL) {
        fuco_node_free(compiler->root);
    }
}

int fuco_compiler_run(fuco_compiler_t *compiler) {
    fuco_tokenizer_add_source_filename(&compiler->tokenizer, 
                                       fuco_strdup(compiler->file));

    compiler->root = fuco_parse_filebody(&compiler->tokenizer);
    if (compiler->root == NULL) {
        return 1;
    }

    if (fuco_node_resolve_global(compiler->root, &compiler->table, 
                                 &compiler->global)) {
        return 1;
    }

    fuco_symbol_t *entry;
    if ((entry = fuco_scope_lookup(&compiler->global, "main",  
                                   NULL, false)) == NULL) {
        fuco_syntax_error(NULL, "entry point '%s' was not defined", "main");
        return 1;
    }

    if (fuco_node_resolve_local(compiler->root, &compiler->table, 
                                &compiler->global)) {
        return 1;
    }

    compiler->ir.label = compiler->table.size;
    fuco_ir_create_startup_object(&compiler->ir, entry->id);
    fuco_node_generate_ir(compiler->root, &compiler->ir, NULL);
    
    fuco_ir_assemble(&compiler->ir, &compiler->bytecode);

    if (compiler->bytecode.instrs == NULL) {
        return 1;
    }
    
    fuco_node_pretty_write(compiler->root, stderr);

    fuco_symboltable_write(&compiler->table, stderr);

    fuco_ir_write(&compiler->ir, stderr);

    fuco_bytecode_write(&compiler->bytecode, stderr);

    return 0;
}
