#include <stdio.h>
#include "parser.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    FUCO_UNUSED(argc), FUCO_UNUSED(argv);

    fuco_parser_t parser;
    fuco_parser_init(&parser);

    fuco_node_t *node = fuco_parser_parse_filebody(&parser);
    if (node == NULL) {
        fprintf(stderr, "parsing error occurred");
    } else {
        fuco_node_pretty_write(node, stderr);
        fprintf(stderr, "\n");

        fuco_node_free(node);
    }

    fuco_parser_destruct(&parser);

    return 0;
}
