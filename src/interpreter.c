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

uint64_t fuco_program_qpop(fuco_program_t *program) {
    program->sp -= sizeof(uint64_t);
    return *(uint64_t *)(program->stack + program->sp);
}

void fuco_program_qpush(fuco_program_t *program, uint64_t data) {
    *(uint64_t *)(program->stack + program->sp) = data;
    program->sp += sizeof(uint64_t);
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

void fuco_program_write_stack(fuco_program_t *program, FILE *file) {
    fprintf(file, "SP: %ld\n", program->sp);
    for (size_t i = 0; i < program->sp; i += 16) {
        fprintf(file, "%04ld ", i);
        for (size_t j = 0; j < 16 && i + j < program->sp; j++) {
            fprintf(file, "%02x ", program->stack[i + j]);
        }
        fprintf(file, "\n");
    }
}

int32_t fuco_interpret(fuco_instr_t *instrs) {  
    fuco_program_t program;
    fuco_program_init(&program, instrs, 4096);

    uint64_t retaddr, retbp;
    uint64_t retq;
    int64_t exit_code = -1;
    
    uint64_t x1, x2;

    bool running = true;

    FUCO_UNUSED(retaddr), FUCO_UNUSED(retbp);

    fprintf(stderr, "Start of execution...\n");

    while (running) {
        fuco_instr_t instr = program.instrs[program.ip];
        fuco_opcode_t opcode = instr & 0xFFFF;

        uint64_t imm48 = instr >> 16;
        int64_t simm48 = FUCO_SEX_IMM48(imm48);
        uint64_t immq = imm48;

        switch (opcode) {
            case FUCO_OPCODE_NOP:
                break;

            case FUCO_OPCODE_CALL:
                fuco_program_qpush(&program, program.ip);
                fuco_program_qpush(&program, program.bp);
                program.bp = program.sp;
                program.ip = imm48 - 1;
                break;

            case FUCO_OPCODE_RETQ:
                retq = fuco_program_qpop(&program);
                program.sp = program.bp;
                program.bp = fuco_program_qpop(&program);
                program.ip = fuco_program_qpop(&program);
                program.sp -= imm48;
                fuco_program_qpush(&program, retq);
                break;

            case FUCO_OPCODE_PUSHQ:
                fuco_program_qpush(&program, immq);
                break;

            case FUCO_OPCODE_LOADQ:
                immq = *(uint64_t *)(program.stack + simm48);
                fuco_program_qpush(&program, immq);
                break;

            case FUCO_OPCODE_RLOADQ:
                immq = *(uint64_t *)(program.stack + program.bp + simm48);
                fuco_program_qpush(&program, immq);
                break;

            case FUCO_OPCODE_IADD:
                x1 = fuco_program_qpop(&program);
                x2 = fuco_program_qpop(&program);
                fuco_program_qpush(&program, x1 + x2);
                break;

            case FUCO_OPCODE_ISUB:
                x1 = fuco_program_qpop(&program);
                x2 = fuco_program_qpop(&program);
                fuco_program_qpush(&program, x1 - x2);
                break;

            case FUCO_OPCODE_IMUL:
                x1 = fuco_program_qpop(&program);
                x2 = fuco_program_qpop(&program);
                fuco_program_qpush(&program, x1 * x2);
                break;

            case FUCO_OPCODE_IDIV: /* TODO: 0 div */
                x1 = fuco_program_qpop(&program);
                x2 = fuco_program_qpop(&program);
                fuco_program_qpush(&program, x1 / x2);
                break;

            case FUCO_OPCODE_IMOD: /* TODO: arithmetic exceptions */
                x1 = fuco_program_qpop(&program);
                x2 = fuco_program_qpop(&program);
                fuco_program_qpush(&program, x1 % x2);
                break;

            case FUCO_OPCODE_EXIT:
                exit_code = fuco_program_qpop(&program);
                running = false;
                break;

            case FUCO_OPCODES_N:
                break;
#if 0
            default:
                fprintf(stderr, "Unhandled instruction at %ld: " 
                        FUCO_INSTR_FORMAT "\n", program.ip, instr);
                return exit_code; /* TODO fix */
#endif
        }

        program.ip++;
    }

    fprintf(stderr, "Program finished with exit code %ld\n", exit_code);

    fuco_program_destruct(&program);

    return exit_code;
}
