#include <stdio.h>
#include "tokenizer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    FUCO_UNUSED(argc), FUCO_UNUSED(argv);

    fuco_tokenizer_t tokenizer;
    fuco_tokenizer_init(&tokenizer);

    char *filename = fuco_strdup("tests/main.fc");
    fuco_tokenizer_add_source_filename(&tokenizer, filename);
    
    int res;
    while (true) {
        if (tokenizer.last == FUCO_TOKEN_EOF) {
            res = fuco_tokenizer_open_next_source(&tokenizer);
            if (res < 0) {
                printf("done.\n");
                break;
            }
        }

        fuco_tokenizer_next_token(&tokenizer);
        
        fuco_token_write(&tokenizer.curr, stdout);
        printf("\n");
    
        fuco_tokenizer_discard(&tokenizer);
    }

    fuco_tokenizer_destruct(&tokenizer);

    return 0;
}
