#ifndef FUCO_LEXER_H
#define FUCO_LEXER_H

#include "token.h"

#define FUCO_LEXER_TOKENLIST_INIT_SIZE 256

#define FUCO_LEXER_FILE_BUF_SIZE 1024

fuco_token_t *fuco_lexer_lex(char *filename);

#endif
