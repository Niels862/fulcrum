#include <stdio.h>
#include "tokenizer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    FUCO_UNUSED(argc), FUCO_UNUSED(argv);

    fuco_tokenizer_t tokenizer;
    fuco_tokenizer_init(&tokenizer);

    fuco_tokenizer_add_source_filename(&tokenizer, fuco_strdup("tests/main.fc"));
    fuco_tokenizer_open_next_source(&tokenizer);
    fuco_next_token(&tokenizer);

    fuco_tokenizer_destruct(&tokenizer);

    return 0;
}
