#include <stdio.h>
#include "parser.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    FUCO_UNUSED(argc), FUCO_UNUSED(argv);

    fuco_tokenizer_t tokenizer;
    fuco_tokenizer_init(&tokenizer);

    fuco_tokenizer_add_source_filename(&tokenizer, fuco_strdup("tests/main.fc"));

    fuco_node_t *node = fuco_parse_filebody(&tokenizer);
    if (node != NULL) {
        fuco_node_pretty_write(node, stderr);
        fprintf(stderr, "\n");

        fuco_node_free(node);
    }

    fuco_tokenizer_destruct(&tokenizer);

    return 0;
}
