#ifndef FUCO_INSTRUCTION_H
#define FUCO_INSTRUCTION_H

#include "defs.h"
#include <stdio.h>
#include <stdint.h>

typedef uint64_t fuco_instr_t;

typedef uint32_t fuco_pointer_t;

typedef enum {
    FUCO_OPCODE_NOP,
    FUCO_OPCODE_CALL,
    FUCO_OPCODE_RETQ,
    FUCO_OPCODE_PUSHQ,
    FUCO_OPCODE_LOADQ,
    FUCO_OPCODE_RLOADQ,
    FUCO_OPCODE_IADD,
    FUCO_OPCODE_ISUB,
    FUCO_OPCODE_IMUL,
    FUCO_OPCODE_IDIV,
    FUCO_OPCODE_IMOD,
    FUCO_OPCODE_EXIT,

    FUCO_OPCODES_N
} fuco_opcode_t;

#define FUCO_GET_OPCODE(instr) ((instr) & 0xFFFF)

#define FUCO_SET_OPCODE(instr, opcode) ((instr) |= (opcode))

#define FUCO_GET_IMM48(instr) ((instr) >> 16)

#define FUCO_SET_IMM48(instr, imm48) ((instr) |= ((imm48) << 16))

#define FUCO_SEX_IMM48(imm48) (((int64_t)(imm48)) << 16) >> 16

#define FUCO_INSTR_FORMAT "%016lx"

typedef enum {
    FUCO_INSTR_LAYOUT_NO_IMM,
    FUCO_INSTR_LAYOUT_IMM48
} fuco_instr_layout_t;

typedef struct {
    fuco_opcode_t opcode;
    char *string;
} fuco_mnemonic_t;

#define FUCO_BYTECODE_INIT_SIZE 1024

typedef struct {
    fuco_instr_t *instrs;
    size_t size;
    size_t cap;
} fuco_bytecode_t;

void fuco_instr_write(fuco_instr_t instr, FILE *file);

char *fuco_opcode_get_mnemonic(fuco_opcode_t opcode);

fuco_instr_layout_t fuco_opcode_get_layout(fuco_opcode_t opcode);

size_t fuco_opcode_get_arity(fuco_opcode_t opcode);

fuco_node_t *fuco_opcode_get_argtype(fuco_opcode_t opcode, 
                                     fuco_symboltable_t *table, size_t arg);

fuco_node_t *fuco_opcode_get_rettype(fuco_opcode_t opcode, 
                                     fuco_symboltable_t *table);

void fuco_bytecode_init(fuco_bytecode_t *bytecode);

void fuco_bytecode_destruct(fuco_bytecode_t *bytecode);

void fuco_bytecode_write(fuco_bytecode_t *bytecode, FILE *file);

void fuco_bytecode_add_instr(fuco_bytecode_t *bytecode, fuco_instr_t instr);

#endif
