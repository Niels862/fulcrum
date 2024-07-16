#ifndef FUCO_INTERPRETER_H
#define FUCO_INTERPRETER_H

#include "instruction.h"
#include <assert.h>
#include <stdbool.h>

typedef struct {
    char *stack;
    fuco_instr_t *instrs;
    uint64_t ip;
    uint64_t sp;
    uint64_t bp;
} fuco_program_t;

void fuco_program_pop(fuco_program_t *program, void *data, size_t size);

void fuco_program_push(fuco_program_t *program, void *data, size_t size);

void fuco_program_init(fuco_program_t *program, fuco_instr_t *instrs, 
                       size_t stack_size);

void fuco_program_destruct(fuco_program_t *program);

void fuco_program_write_stack(fuco_program_t *program, FILE *file);

int32_t fuco_interpret(fuco_instr_t *program);

#endif
