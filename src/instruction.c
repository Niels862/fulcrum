#include "instruction.h"

fuco_instr_descriptor_t instr_descriptors[] = {
    { FUCO_OPCODE_NOP, "nop", FUCO_INSTR_LAYOUT_NO_IMM }, 
    { FUCO_OPCODE_CALL, "call", FUCO_INSTR_LAYOUT_IMM48 }, 
    { FUCO_OPCODE_RET, "ret", FUCO_INSTR_LAYOUT_IMM48 }, 
    { FUCO_OPCODE_PUSH4, "push4", FUCO_INSTR_LAYOUT_IMM48 }, 
    { FUCO_OPCODE_EXIT, "exit", FUCO_INSTR_LAYOUT_IMM48 }
};

#define FUCO_N_INSTRUCTIONS \
        sizeof(instr_descriptors) / sizeof(*instr_descriptors)

void fuco_disassemble(fuco_instr_t instr, FILE *stream) {
    fuco_opcode_t opcode = FUCO_GET_OPCODE(instr);
    uint64_t imm48 = FUCO_GET_IMM48(instr);

    if (opcode < FUCO_N_INSTRUCTIONS) {
        fuco_instr_descriptor_t descriptor = instr_descriptors[opcode];

        switch (descriptor.layout) {
            case FUCO_INSTR_LAYOUT_NO_IMM:
                fprintf(stream, "%s\n", descriptor.mnemonic);
                break;

            case FUCO_INSTR_LAYOUT_IMM48:
                fprintf(stream, "%s %ld\n", descriptor.mnemonic, imm48);
                break;

            default:
                fprintf(stream, "?\n");
        }
    } else {
        fprintf(stream, "? (" FUCO_INSTR_FORMAT ")\n", instr);
    }
}
