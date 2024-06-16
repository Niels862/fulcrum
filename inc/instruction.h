#ifndef FUCO_INSTRUCTION_H
#define FUCO_INSTRUCTION_H

#include <stdio.h>
#include <stdint.h>

typedef uint64_t fuco_instr_t;

typedef uint64_t fuco_pointer_t;

typedef enum {
    FUCO_OPCODE_NOP = 0,
    FUCO_OPCODE_CALL,
    FUCO_OPCODE_RET,
    FUCO_OPCODE_PUSH4,
    FUCO_OPCODE_EXIT
} fuco_opcode_t;

#define FUCO_GET_OPCODE(instr) ((instr) & 0xFFFF)

#define FUCO_GET_IMM48(instr) ((instr) >> 16)

#define FUCO_INSTR_FORMAT "%016lx"

typedef enum {
    FUCO_INSTR_LAYOUT_NO_IMM,
    FUCO_INSTR_LAYOUT_IMM48
} fuco_instr_layout_t;

typedef struct {
    fuco_opcode_t opcode;
    char *mnemonic;
    fuco_instr_layout_t layout;
} fuco_instr_descriptor_t;

extern fuco_instr_descriptor_t instr_descriptors[];

void fuco_disassemble(fuco_instr_t instr, FILE *stream);

#endif
