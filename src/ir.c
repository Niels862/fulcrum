#include "ir.h"
#include "utils.h"
#include "tree.h"
#include <stdlib.h>
#include <assert.h>

void fuco_ir_unit_write(fuco_ir_unit_t *unit, FILE *file) {
    if (unit->attrs & FUCO_IR_INSTR) {
        fprintf(file, "  %s", fuco_opcode_get_mnemonic(unit->opcode));
        
        if (unit->attrs & FUCO_IR_INCLUDES_DATA) {
            if (unit->attrs & FUCO_IR_REFERENCES_LABEL) {
                fprintf(file, " .L%ld", unit->imm.label);
            } else {
                fprintf(file, " %ld", unit->imm.data);
            }
        }
        fprintf(file, "\n");
    } else {
        fprintf(file, ".L%ld:\n", unit->imm.label);
    }
}

void fuco_ir_object_init(fuco_ir_object_t *object, fuco_node_t *def) {
    object->cap = FUCO_OBJECT_INIT_SIZE;
    object->units = malloc(object->cap * sizeof(fuco_ir_unit_t));
    object->size = 0;
    object->def = def;
    object->paramsize_label = 0;
}

void fuco_ir_object_destruct(fuco_ir_object_t *object) {
    free(object->units);
}

void fuco_ir_object_write(fuco_ir_object_t *object, FILE *file) {
    fprintf(file, "object {\n");

    if (object->size == 0) {
        fprintf(file, "  (empty)\n");
    } else {
        for (size_t i = 0; i < object->size; i++) {
            fprintf(file, "  ");
            fuco_ir_unit_write(&object->units[i], file);
        }
    }

    fprintf(file, "}\n");
}

void fuco_ir_init(fuco_ir_t *ir) {
    ir->cap = FUCO_IR_OBJECTS_INIT_SIZE;
    ir->objects = malloc(ir->cap * sizeof(fuco_ir_object_t));
    ir->size = 0;
    ir->label = 0;
}

void fuco_ir_destruct(fuco_ir_t *ir) {
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_destruct(&ir->objects[i]);
    }

    if (ir->objects != NULL) {
        free(ir->objects);
    }
}

void fuco_ir_write(fuco_ir_t *ir, FILE *file) {
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_write(&ir->objects[i], file);
    }
}

fuco_ir_label_t fuco_ir_next_label(fuco_ir_t *ir) {
    fuco_ir_label_t label = ir->label;
    
    ir->label++;

    return label;
}

size_t fuco_ir_add_object(fuco_ir_t *ir, fuco_ir_label_t label, 
                          fuco_node_t *def) {
    if (ir->size >= ir->cap) {
        ir->cap *= 2;
        ir->objects = realloc(ir->objects, 
                              ir->cap * sizeof(fuco_ir_object_t));
    }
    
    size_t obj = ir->size;

    fuco_ir_object_init(&ir->objects[obj], def);
    fuco_ir_add_label(ir, obj, label);
    ir->objects[obj].paramsize_label = fuco_ir_next_label(ir);

    ir->size++;

    return obj;
}

fuco_ir_unit_t *fuco_ir_add_unit(fuco_ir_t *ir, size_t obj, 
                                 fuco_opcode_t opcode, fuco_ir_attr_t attrs) {
    fuco_ir_object_t *object = &ir->objects[obj];
    
    if (object->size >= object->cap) {
        object->cap *= 2;
        object->units = malloc(object->cap * sizeof(fuco_ir_unit_t));
    }

    fuco_ir_unit_t *unit = &object->units[object->size];

    unit->opcode = opcode;
    unit->attrs = attrs;

    object->size++;

    return unit;
}

void fuco_ir_add_instr(fuco_ir_t *ir, size_t obj, fuco_opcode_t opcode) {
    assert(fuco_opcode_get_layout(opcode) == FUCO_INSTR_LAYOUT_NO_IMM);
    
    fuco_ir_add_unit(ir, obj, opcode, FUCO_IR_INSTR);
}

void fuco_ir_add_label(fuco_ir_t *ir, size_t obj, fuco_ir_label_t label) {
    fuco_ir_unit_t *unit = fuco_ir_add_unit(ir, obj, FUCO_OPCODE_NOP, 
                                            FUCO_IR_LABEL); 
    
    unit->imm.label = label;
}

void fuco_ir_add_instr_imm48(fuco_ir_t *ir, size_t obj, fuco_opcode_t opcode, 
                             uint64_t data) {
    assert(fuco_opcode_get_layout(opcode) == FUCO_INSTR_LAYOUT_IMM48);

    fuco_ir_unit_t *unit = fuco_ir_add_unit(ir, obj, opcode, 
                                            FUCO_IR_INSTR); 

    unit->attrs |= FUCO_IR_INCLUDES_DATA;
    unit->imm.data = data;
}

void fuco_ir_add_instr_imm48_label(fuco_ir_t *ir, size_t obj, 
                                   fuco_opcode_t opcode, 
                                   fuco_ir_label_t label) {
    assert(fuco_opcode_get_layout(opcode) == FUCO_INSTR_LAYOUT_IMM48);

    fuco_ir_unit_t *unit = fuco_ir_add_unit(ir, obj, opcode, 
                                            FUCO_IR_INSTR); 

    unit->attrs |= FUCO_IR_INCLUDES_DATA | FUCO_IR_REFERENCES_LABEL;
    unit->imm.label = label;
}

void fuco_ir_create_startup_object(fuco_ir_t *ir, fuco_ir_label_t entry) {
    assert(ir->size == 0);

    size_t obj = fuco_ir_add_object(ir, FUCO_LABEL_STARTUP, NULL);

    fuco_ir_add_instr_imm48_label(ir, obj, FUCO_OPCODE_CALL, entry);
    fuco_ir_add_instr(ir, obj, FUCO_OPCODE_EXIT);
}

void fuco_ir_assemble(fuco_ir_t *ir, fuco_bytecode_t *bytecode) {
    assert(ir->size > 0);
    assert(ir->objects[0].def == NULL);

    uint64_t *defs = malloc(ir->label * sizeof(uint64_t));
    for (size_t i = 0; i < ir->label; i++) {
        defs[i] = FUCO_LABEL_DEF_INVALID;
    }

    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_t *obj = &ir->objects[i];

        if (obj->def != NULL) {
            size_t paramsize = fuco_node_setup_offsets(obj->def, defs);
            printf("set (%ld) to %ld\n", ir->objects[i].paramsize_label, paramsize);
            defs[obj->paramsize_label] = paramsize;
        }
    }

    /* First pass: define labels */
    size_t jump_location = 0;
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_t *object = &ir->objects[i];

        for (size_t j = 0; j < object->size; j++) {
            fuco_ir_unit_t *unit = &object->units[j];

            if (unit->attrs & FUCO_IR_INSTR) {
                jump_location++;
            } else {
                assert(unit->imm.label < ir->label);
                assert(defs[unit->imm.label] == FUCO_LABEL_DEF_INVALID);

                defs[unit->imm.label] = jump_location;
            }
        }
    }

    /* Second pass: write bytecode */
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_t *object = &ir->objects[i];

        for (size_t j = 0; j < object->size; j++) {
            fuco_ir_unit_t *unit = &object->units[j];

            if (unit->attrs & FUCO_IR_INSTR) {
                fuco_instr_t instr = 0;
                FUCO_SET_OPCODE(instr, unit->opcode);

                uint64_t imm;
                if (unit->attrs & FUCO_IR_INCLUDES_DATA) {
                    if (unit->attrs & FUCO_IR_REFERENCES_LABEL) {
                        imm = defs[unit->imm.label];

                        assert(imm != FUCO_LABEL_DEF_INVALID);
                    } else {
                        imm = unit->imm.data;
                    }
                } else {
                    imm = 0;
                }

                switch (fuco_opcode_get_layout(unit->opcode)) {
                    case FUCO_INSTR_LAYOUT_NO_IMM:
                        break;

                    case FUCO_INSTR_LAYOUT_IMM48:
                        FUCO_SET_IMM48(instr, imm);
                        break;
                }

                fuco_bytecode_add_instr(bytecode, instr);
            }
        }
    }

    free(defs);
}
