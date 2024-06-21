#include <stdio.h>
#include "parser.h"
#include "symbol.h"
#include "interpreter.h"
#include "utils.h"
#include "compiler.h"

int main(int argc, char *argv[]) {
    FUCO_UNUSED(argc), FUCO_UNUSED(argv);

    fuco_compiler_t compiler;
    fuco_compiler_init(&compiler, "tests/main.fc");

    fuco_instr_t *instrs = fuco_compiler_run(&compiler);

    fuco_compiler_destruct(&compiler);

    if (instrs != NULL) {
        fuco_interpret(instrs);
    }

    return 0;
}
