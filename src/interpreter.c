#include "interpreter.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

void fuco_program_pop(fuco_program_t *program, void *data, size_t size) {
    program->sp -= size;
    memcpy(data, program->stack + program->sp, size);
}

void fuco_program_push(fuco_program_t *program, void *data, size_t size) {
    memcpy(program->stack + program->sp, data, size);
    program->sp += size;
}

void fuco_program_init(fuco_program_t *program, fuco_instr_t *instrs, 
                       size_t stack_size) {
    program->ip = program->sp = program->bp = 0;
    program->instrs = instrs;
    program->stack = malloc(stack_size);
}

void fuco_program_destruct(fuco_program_t *program) {
    free(program->stack);
}

void fuco_program_write_stack(fuco_program_t *program, FILE *stream) {
    fprintf(stream, "SP: %d\n", program->sp);
    for (size_t i = 0; i < program->sp; i += 16) {
        fprintf(stream, "%04ld ", i);
        for (size_t j = 0; j < 16 && i + j < program->sp; j++) {
            fprintf(stream, "%02x ", program->stack[i + j]);
        }
        fprintf(stream, "\n");
    }
}

int32_t fuco_interpret(fuco_instr_t *instrs) {  
    fuco_program_t program;
    fuco_program_init(&program, instrs, 4096);

    uint32_t retaddr, retbp;
    uint32_t vald1, vald2, retd;
    int32_t exit_code = -1;

    bool running = true;

    FUCO_UNUSED(retaddr), FUCO_UNUSED(retbp);
    FUCO_UNUSED(vald1), FUCO_UNUSED(vald2);

    fprintf(stderr, "Start of execution...\n");

    while (running) {
        fuco_instr_t instr = program.instrs[program.ip];
        fuco_opcode_t opcode = instr & 0xFFFF;
        uint64_t imm48 = instr >> 16;
        uint32_t immd = imm48;

        switch (opcode) {
            case FUCO_OPCODE_NOP:
                break;

            case FUCO_OPCODE_CALL:
                fuco_program_push(&program, &program.ip, sizeof(program.ip));
                fuco_program_push(&program, &program.bp, sizeof(program.bp));
                program.bp = program.sp;
                program.ip = imm48 - 1;
                break;

            case FUCO_OPCODE_RETD:
                fuco_program_pop(&program, &retd, sizeof(retd));
                program.sp = program.bp;
                fuco_program_pop(&program, &program.bp, sizeof(program.bp));
                fuco_program_pop(&program, &program.ip, sizeof(program.ip));
                fuco_program_push(&program, &retd, sizeof(retd));
                break;

            case FUCO_OPCODE_PUSHD:
                fuco_program_push(&program, &immd, sizeof(uint32_t));
                break;

            case FUCO_OPCODE_EXIT:
                fuco_program_pop(&program, &exit_code, sizeof(exit_code));
                running = false;
                break;

            default:
                fprintf(stderr, "Unhandled instruction at %d: " 
                        FUCO_INSTR_FORMAT "\n", program.ip, instr);
                return exit_code;
        }

        program.ip++;
    }

    fprintf(stderr, "Program finished with exit code %d\n", exit_code);

    fuco_program_destruct(&program);

    return exit_code;
}
