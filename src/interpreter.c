#include "interpreter.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

void fuco_interpret(fuco_instr_t *program) {
    fuco_pointer_t ip = 0;
    fuco_pointer_t bp = 0;
    fuco_pointer_t sp = 0;

    char *stack = malloc(4096);

    FUCO_UNUSED(ip), FUCO_UNUSED(bp), FUCO_UNUSED(sp), FUCO_UNUSED(stack);

    while (1) {
        fuco_instr_t instr = program[ip];
        fuco_opcode_t opcode = instr & 0xFFFF;
        uint64_t imm48 = instr >> 16;

        FUCO_UNUSED(imm48);

        switch (opcode) {
            case FUCO_OPCODE_NOP:
                break;

            case FUCO_OPCODE_CALL:
                /* ... */
                break;

            case FUCO_OPCODE_RET:
                /* ... */
                break;

            case FUCO_OPCODE_PUSH4:
                /* ... */
                break;

            case FUCO_OPCODE_EXIT:
                break;

            default:
                fprintf(stderr, "Unhandled instruction: " 
                        FUCO_INSTR_FORMAT "\n", instr);
                return;
        }

        ip++;
    }
}
