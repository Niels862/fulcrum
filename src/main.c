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

    if (node != NULL) {
        if (fuco_node_resolve_symbols_global(node, &table, &global) == 0) {
            fuco_node_pretty_write(node, stderr);
            fprintf(stderr, "\n");

            fuco_symboltable_write(&table, stderr);
        }

        fuco_node_free(node);
    }

    fuco_tokenizer_destruct(&tokenizer);
    fuco_symboltable_destruct(&table);
    fuco_scope_destruct(&global);

    return 0;
}
