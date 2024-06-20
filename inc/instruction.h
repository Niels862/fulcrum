#ifndef FUCO_INSTRUCTION_H
#define FUCO_INSTRUCTION_H

#include <stdio.h>
#include <stdint.h>

typedef uint64_t fuco_instr_t;

typedef uint32_t fuco_pointer_t;

typedef enum {
    FUCO_OPCODE_NOP = 0,
    FUCO_OPCODE_CALL,
    FUCO_OPCODE_RETD,
    FUCO_OPCODE_PUSHD,
    FUCO_OPCODE_EXIT
} fuco_opcode_t;

#define FUCO_GET_OPCODE(instr) ((instr) & 0xFFFF)

#define FUCO_SET_OPCODE(instr, opcode) ((instr) |= (opcode))

#define FUCO_GET_IMM48(instr) ((instr) >> 16)

#define FUCO_SET_IMM48(instr, imm48) ((instr) |= ((imm48) << 16))

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

#define FUCO_BYTECODE_INIT_SIZE 1024

typedef struct {
    fuco_instr_t *instrs;
    size_t size;
    size_t cap;
} fuco_bytecode_t;

extern fuco_instr_descriptor_t instr_descriptors[];

void fuco_instr_write(fuco_instr_t instr, FILE *stream);

void fuco_bytecode_init(fuco_bytecode_t *bytecode);

void fuco_bytecode_destruct(fuco_bytecode_t *bytecode);

void fuco_bytecode_write(fuco_bytecode_t *bytecode, FILE *stream);

void fuco_bytecode_add_instr(fuco_bytecode_t *bytecode, fuco_instr_t instr);

#endif
