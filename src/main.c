#include <stdio.h>
#include "parser.h"
#include "symbol.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    FUCO_UNUSED(argc), FUCO_UNUSED(argv);

    fuco_tokenizer_t tokenizer;
    fuco_tokenizer_init(&tokenizer);

    fuco_tokenizer_add_source_filename(&tokenizer, fuco_strdup("tests/main.fc"));

    fuco_node_t *node = fuco_parse_filebody(&tokenizer);
    
    fuco_symboltable_t table;
    fuco_symboltable_init(&table);
    
    fuco_scope_t global;
    fuco_scope_init(&global, NULL);

    fuco_ir_t ir;
    fuco_ir_init(&ir);

    if (node != NULL) {
        if (fuco_node_resolve_symbols_global(node, &table, &global) == 0) {
            fuco_symbol_t *entry = fuco_scope_lookup(&global, "main");
            if (entry == NULL) {
                fuco_syntax_error(NULL, "entry point '%s' was not defined", 
                                  "main");
            } else {
                /* Labels start where symbol IDs finished */
                ir.label = table.size;
                fuco_node_generate_ir(node, &ir, NULL);
                
                fuco_node_pretty_write(node, stderr);
                fuco_symboltable_write(&table, stderr);
                for (size_t i = 0; i < ir.size; i++) {
                    fuco_ir_object_write(&ir.objects[i], stderr);
                }
            }
        }

        fuco_node_free(node);
    }

    fuco_tokenizer_destruct(&tokenizer);
    fuco_symboltable_destruct(&table);
    fuco_scope_destruct(&global);
    fuco_ir_destruct(&ir);

    return 0;
}
