#ifndef FUCO_IR_H
#define FUCO_IR_H

#include "instruction.h"
#include <stdint.h>

typedef uint64_t fuco_ir_label_t;

typedef enum {
    FUCO_IR_INSTR = 0x1, /* INSTR and LABEL are exclusive */
    FUCO_IR_LABEL = 0x2,
    FUCO_IR_REFERENCING_LABEL = 0x4
} fuco_ir_nodetype_t;

typedef struct fuco_ir_node_t {
    fuco_ir_nodetype_t attrs;
    
    fuco_opcode_t opcode;
    union {
        fuco_ir_label_t label;
        uint64_t data;
    } imm;

    struct fuco_ir_node_t *next;
} fuco_ir_node_t;

#define FUCO_IR_OBJECTS_INIT_SIZE 16

typedef struct {
    fuco_ir_node_t *objects;
    size_t size;
    size_t cap;
    fuco_ir_label_t label;
} fuco_ir_t;

fuco_ir_node_t *fuco_ir_instr_new();

fuco_ir_node_t *fuco_ir_label_new();

void fuco_ir_join(fuco_ir_node_t *pre, fuco_ir_node_t *post);

void fuco_ir_node_free(fuco_ir_node_t *node);

void fuco_ir_init(fuco_ir_t *ir);

void fuco_ir_destruct(fuco_ir_t *ir);

#endif
