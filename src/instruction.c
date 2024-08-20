#include "instruction.h"
#include "utils.h"
#include <stdlib.h>

char *fuco_opcode_get_mnemonic(fuco_opcode_t opcode) {
    switch (opcode) {
        case FUCO_OPCODE_NOP:
            return "nop";

        case FUCO_OPCODE_CALL:
            return "call";

        case FUCO_OPCODE_RETQ:
            return "retq";

        case FUCO_OPCODE_PUSHQ:
            return "pushq";

        case FUCO_OPCODE_LOADQ:
            return "loadq";

        case FUCO_OPCODE_RLOADQ:
            return "rloadq";

        case FUCO_OPCODE_IADD:
            return "iadd";

        case FUCO_OPCODE_ISUB:
            return "isub";

        case FUCO_OPCODE_IMUL:
            return "imul";

        case FUCO_OPCODE_IDIV:
            return "idiv";

        case FUCO_OPCODE_IMOD:
            return "imod";

        case FUCO_OPCODE_EXIT:
            return "exit";

        case FUCO_OPCODES_N:
            break;
    }

    FUCO_UNREACHED();
}

fuco_instr_layout_t fuco_opcode_get_layout(fuco_opcode_t opcode) {
    switch (opcode) {
        case FUCO_OPCODE_NOP:
        case FUCO_OPCODE_IADD:
        case FUCO_OPCODE_ISUB:
        case FUCO_OPCODE_IMUL:
        case FUCO_OPCODE_IDIV:
        case FUCO_OPCODE_IMOD:
        case FUCO_OPCODE_EXIT:
            return FUCO_INSTR_LAYOUT_NO_IMM;

        case FUCO_OPCODE_CALL:
        case FUCO_OPCODE_RETQ:
        case FUCO_OPCODE_PUSHQ:
        case FUCO_OPCODE_LOADQ:
        case FUCO_OPCODE_RLOADQ:
            return FUCO_INSTR_LAYOUT_IMM48;

        case FUCO_OPCODES_N:
            break;
    }

    FUCO_UNREACHED();
}

void fuco_instr_write(fuco_instr_t instr, FILE *file) {
    fuco_opcode_t opcode = FUCO_GET_OPCODE(instr);
    uint64_t imm48 = FUCO_GET_IMM48(instr);

    if (opcode < FUCO_OPCODES_N) {
        char *mnemonic = fuco_opcode_get_mnemonic(opcode);
        
        switch (fuco_opcode_get_layout(opcode)) {
            case FUCO_INSTR_LAYOUT_NO_IMM:
                fprintf(file, "%s\n", mnemonic);
                break;

            case FUCO_INSTR_LAYOUT_IMM48:
                fprintf(file, "%s %ld\n", mnemonic, 
                        FUCO_SEX_IMM48(imm48));
                break;

            default:
                fprintf(file, "?\n");
        }
    } else {
        fprintf(file, "? (" FUCO_INSTR_FORMAT ")\n", instr);
    }
}

void fuco_bytecode_init(fuco_bytecode_t *bytecode) {
    bytecode->instrs = NULL;
    bytecode->size = 0;
    bytecode->cap = 0;
}

void fuco_bytecode_destruct(fuco_bytecode_t *bytecode) {
    if (bytecode->instrs != NULL) {
        free(bytecode->instrs);
    }
}

void fuco_bytecode_write(fuco_bytecode_t *bytecode, FILE *file) {
    for (size_t i = 0; i < bytecode->size; i++) {
        fprintf(file, FUCO_INSTR_FORMAT ": ", bytecode->instrs[i]);
        fuco_instr_write(bytecode->instrs[i], file);
    }
}

void fuco_bytecode_add_instr(fuco_bytecode_t *bytecode, fuco_instr_t instr) {
    if (bytecode->size >= bytecode->cap) {
        if (bytecode->cap == 0) {
            bytecode->cap = FUCO_BYTECODE_INIT_SIZE;
        } else {
            bytecode->cap *= 2;
        }
        bytecode->instrs = realloc(bytecode->instrs, bytecode->cap);
    }

    bytecode->instrs[bytecode->size] = instr;
    bytecode->size++;
}
