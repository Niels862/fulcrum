#ifndef FUCO_IR_H
#define FUCO_IR_H

#include "instruction.h"
#include "defs.h"
#include <stdint.h>

typedef uint64_t fuco_ir_label_t;

#define FUCO_LABEL_DEF_INVALID (uint64_t)-1

/* Can be equal as startup may not be used as label */
#define FUCO_LABEL_STARTUP (fuco_ir_label_t)0

#define FUCO_LABEL_INVALID (fuco_ir_label_t)0

typedef enum {
    FUCO_IR_LABEL = 0x0, /* Does nothing, more explicit */
    FUCO_IR_INSTR = 0x1,
    FUCO_IR_REFERENCES_LABEL = 0x2,
    FUCO_IR_INCLUDES_DATA = 0x4
} fuco_ir_attr_t;

typedef struct fuco_ir_unit_t {    
    fuco_opcode_t opcode;
    union {
        fuco_ir_label_t label;
        uint64_t data;
    } imm;
    fuco_ir_attr_t attrs;
} fuco_ir_unit_t;

#define FUCO_OBJECT_INIT_SIZE 16

/* Insertion at end */
typedef struct {
    fuco_ir_unit_t *units;
    size_t size;
    size_t cap;
    fuco_node_t *def;
    fuco_ir_label_t paramsize_label;
} fuco_ir_object_t;

#define FUCO_IR_OBJECTS_INIT_SIZE 16

typedef struct {
    fuco_ir_object_t *objects;
    size_t size;
    size_t cap;
    fuco_ir_label_t label;
} fuco_ir_t;

void fuco_ir_unit_write(fuco_ir_unit_t *unit, FILE *file);

void fuco_ir_object_init(fuco_ir_object_t *object, fuco_node_t *def);

void fuco_ir_object_destruct(fuco_ir_object_t *object);

void fuco_ir_object_write(fuco_ir_object_t *object, FILE *file);

void fuco_ir_init(fuco_ir_t *ir);

void fuco_ir_destruct(fuco_ir_t *ir);

void fuco_ir_write(fuco_ir_t *ir, FILE *file);

fuco_ir_label_t fuco_ir_next_label(fuco_ir_t *ir);

size_t fuco_ir_add_object(fuco_ir_t *ir, fuco_ir_label_t label, 
                          fuco_node_t *def);

fuco_ir_unit_t *fuco_ir_add_unit(fuco_ir_t *ir, size_t obj, 
                                 fuco_opcode_t opcode, fuco_ir_attr_t attrs);

void fuco_ir_add_instr(fuco_ir_t *ir, size_t obj, fuco_opcode_t opcode);

void fuco_ir_add_label(fuco_ir_t *ir, size_t obj, fuco_ir_label_t label);

void fuco_ir_add_instr_imm48(fuco_ir_t *ir, size_t obj, fuco_opcode_t opcode, 
                             uint64_t data);

void fuco_ir_add_instr_imm48_label(fuco_ir_t *ir, size_t obj, 
                                   fuco_opcode_t opcode, fuco_ir_label_t label);

void fuco_ir_create_startup_object(fuco_ir_t *ir, fuco_ir_label_t entry);

void fuco_ir_assemble(fuco_ir_t *ir, fuco_bytecode_t *bytecode);

#endif
