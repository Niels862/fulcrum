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

    if (fuco_compiler_run(&compiler) == 0) {
        fuco_interpret(compiler.bytecode.instrs);
    }

    fuco_compiler_destruct(&compiler);

    return 0;
}
