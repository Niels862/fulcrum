#include "ir.h"
#include "utils.h"
#include "tree.h"
#include <stdlib.h>
#include <assert.h>

fuco_ir_node_t *fuco_ir_node_new(fuco_opcode_t opcode, fuco_ir_attr_t attrs) {
    fuco_ir_node_t *node = malloc(sizeof(fuco_ir_node_t));

    node->attrs = attrs;
    node->opcode = opcode;
    node->next = NULL;

    return node;
}

void fuco_ir_node_free(fuco_ir_node_t *node) {
    while (node != NULL) {
        fuco_ir_node_t *next = node->next;
        free(node);
        node = next;
    }
}

void fuco_ir_node_write(fuco_ir_node_t *node, FILE *stream) {
    if (node->attrs & FUCO_IR_INSTR) {
        fprintf(stream, "  %s", instr_descriptors[node->opcode].mnemonic);
        if (node->attrs & FUCO_IR_INCLUDES_DATA) {
            if (node->attrs & FUCO_IR_REFERENCES_LABEL) {
                fprintf(stream, " .L%ld", node->imm.label);
            } else {
                fprintf(stream, " %ld", node->imm.data);
            }
        }
        fprintf(stream, "\n");
    } else {
        fprintf(stream, ".L%ld:\n", node->imm.label);
    }
}

void fuco_ir_object_init(fuco_ir_object_t *object, fuco_ir_label_t label, 
                         fuco_node_t *def) {
    object->label = label;
    object->begin = object->end = NULL;
    object->def = def;
}

void fuco_ir_object_destruct(fuco_ir_object_t *object) {
    fuco_ir_node_free(object->begin);
}

void fuco_ir_object_write(fuco_ir_object_t *object, FILE *stream) {
    fprintf(stream, "object %ld: {\n", object->label);

    fuco_ir_node_t *node = object->begin;
    if (node == NULL) {
        fprintf(stream, "  (empty)\n");
    } else {
        while (node != NULL) {
            fprintf(stream, "  ");
            fuco_ir_node_write(node, stream);
            node = node->next;
        }
    }

    fprintf(stream, "}\n");
}

void fuco_ir_init(fuco_ir_t *ir) {
    ir->objects = malloc(FUCO_IR_OBJECTS_INIT_SIZE * sizeof(fuco_ir_node_t));
    ir->size = 0;
    ir->cap = FUCO_IR_OBJECTS_INIT_SIZE;
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

void fuco_ir_write(fuco_ir_t *ir, FILE *stream) {
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_write(&ir->objects[i], stream);
    }
}

fuco_ir_object_t *fuco_ir_add_object(fuco_ir_t *ir, fuco_ir_label_t label, 
                                     fuco_node_t *def) {
    if (ir->size >= ir->cap) {
        ir->cap *= 2;
        ir->objects = realloc(ir->objects, 
                              ir->cap * sizeof(fuco_ir_object_t));
    }
    
    fuco_ir_object_t *object = &ir->objects[ir->size];
    fuco_ir_object_init(object, label, def);
    fuco_ir_add_label(object, label);

    ir->size++;

    return object;
}

void fuco_ir_add_node(fuco_ir_object_t *object, fuco_ir_node_t *node) {
    if (object->begin == NULL) {
        object->begin = object->end = node;
    } else {
        object->end->next = node;
        object->end = node;
    }
}

void fuco_ir_add_instr(fuco_ir_object_t *object, fuco_opcode_t opcode) {
    assert(instr_descriptors[opcode].layout == FUCO_INSTR_LAYOUT_NO_IMM);
    
    fuco_ir_node_t *node = fuco_ir_node_new(opcode, FUCO_IR_INSTR);
    fuco_ir_add_node(object, node);
}

void fuco_ir_add_label(fuco_ir_object_t *object, fuco_ir_label_t label) {
    fuco_ir_node_t *node = fuco_ir_node_new(FUCO_OPCODE_NOP, FUCO_IR_LABEL);
    
    node->imm.label = label;

    fuco_ir_add_node(object, node); 
}

void fuco_ir_add_instr_imm48(fuco_ir_object_t *object, fuco_opcode_t opcode, 
                             uint64_t data) {
    assert(instr_descriptors[opcode].layout == FUCO_INSTR_LAYOUT_IMM48);

    fuco_ir_node_t *node = fuco_ir_node_new(opcode, FUCO_IR_INSTR);

    node->attrs |= FUCO_IR_INCLUDES_DATA;
    node->imm.data = data;

    fuco_ir_add_node(object, node);
}

void fuco_ir_add_instr_imm48_label(fuco_ir_object_t *object, 
                                   fuco_opcode_t opcode, 
                                   fuco_ir_label_t label) {
    assert(instr_descriptors[opcode].layout == FUCO_INSTR_LAYOUT_IMM48);

    fuco_ir_node_t *node = fuco_ir_node_new(opcode, FUCO_IR_INSTR);

    node->attrs |= FUCO_IR_INCLUDES_DATA | FUCO_IR_REFERENCES_LABEL;
    node->imm.label = label;

    fuco_ir_add_node(object, node);
}

void fuco_ir_create_startup_object(fuco_ir_t *ir, fuco_ir_label_t entry) {
    assert(ir->size == 0);

    fuco_ir_object_t *object = fuco_ir_add_object(ir, FUCO_LABEL_STARTUP, NULL);

    fuco_ir_add_instr_imm48_label(object, FUCO_OPCODE_CALL, entry);
    fuco_ir_add_instr(object, FUCO_OPCODE_EXIT);
}

void fuco_ir_assemble(fuco_ir_t *ir, fuco_bytecode_t *bytecode) {
    assert(ir->size > 0);
    assert(ir->objects[0].label == FUCO_LABEL_STARTUP);

    uint64_t *defs = malloc(ir->label * sizeof(uint64_t));
    for (size_t i = 0; i < ir->label; i++) {
        defs[i] = FUCO_LABEL_DEF_INVALID;
    }

    for (size_t i = 0; i < ir->size; i++) {
        fuco_node_t *def = ir->objects[i].def;
        if (def != NULL) {
            fuco_node_setup_offsets(def, defs);
        }
    }

    /* First pass: define labels */
    size_t p = 0;
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_t *object = &ir->objects[i];
        fuco_ir_node_t *node = object->begin;

        while (node != NULL) {
            if (node->attrs & FUCO_IR_INSTR) {
                p++;
            } else {
                assert(node->imm.label < ir->label);
                assert(defs[node->imm.label] == FUCO_LABEL_DEF_INVALID);

                defs[node->imm.label] = p;
            }

            node = node->next;
        }
    }

    for (size_t i = 0; i < ir->label; i++) {
        fprintf(stderr, "%ld -> %ld\n", i, defs[i]);
    }

    /* Second pass: write bytecode */
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_t *object = &ir->objects[i];
        fuco_ir_node_t *node = object->begin;

        while (node != NULL) {
            if (node->attrs & FUCO_IR_INSTR) {
                fuco_instr_t instr = 0;
                FUCO_SET_OPCODE(instr, node->opcode);

                uint64_t imm;
                if (node->attrs & FUCO_IR_INCLUDES_DATA) {
                    if (node->attrs & FUCO_IR_REFERENCES_LABEL) {
                        imm = defs[node->imm.label];

                        assert(imm != FUCO_LABEL_DEF_INVALID);
                    } else {
                        imm = node->imm.data;
                    }
                } else {
                    imm = 0;
                }

                switch (instr_descriptors[node->opcode].layout) {
                    case FUCO_INSTR_LAYOUT_NO_IMM:
                        break;

                    case FUCO_INSTR_LAYOUT_IMM48:
                        FUCO_SET_IMM48(instr, imm);
                        break;
                }

                fuco_bytecode_add_instr(bytecode, instr);
            }

            node = node->next;
        }
    }

    free(defs);
}
