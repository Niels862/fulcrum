#include "instruction.h"
#include <stdlib.h>

fuco_instr_descriptor_t instr_descriptors[] = {
    { FUCO_OPCODE_NOP, "nop", FUCO_INSTR_LAYOUT_NO_IMM }, 
    { FUCO_OPCODE_CALL, "call", FUCO_INSTR_LAYOUT_IMM48 }, 
    { FUCO_OPCODE_RETQ, "retq", FUCO_INSTR_LAYOUT_NO_IMM }, 
    { FUCO_OPCODE_PUSHQ, "pushq", FUCO_INSTR_LAYOUT_IMM48 }, 
    { FUCO_OPCODE_LOADQ, "loadq", FUCO_INSTR_LAYOUT_IMM48 },
    { FUCO_OPCODE_RLOADQ, "rloadq", FUCO_INSTR_LAYOUT_IMM48 },
    { FUCO_OPCODE_EXIT, "exit", FUCO_INSTR_LAYOUT_NO_IMM }
};

#define FUCO_N_INSTRUCTIONS \
        sizeof(instr_descriptors) / sizeof(*instr_descriptors)

void fuco_instr_write(fuco_instr_t instr, FILE *file) {
    fuco_opcode_t opcode = FUCO_GET_OPCODE(instr);
    uint64_t imm48 = FUCO_GET_IMM48(instr);

    if (opcode < FUCO_N_INSTRUCTIONS) {
        fuco_instr_descriptor_t descriptor = instr_descriptors[opcode];

        switch (descriptor.layout) {
            case FUCO_INSTR_LAYOUT_NO_IMM:
                fprintf(file, "%s\n", descriptor.mnemonic);
                break;

            case FUCO_INSTR_LAYOUT_IMM48:
                fprintf(file, "%s %ld\n", descriptor.mnemonic, 
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
