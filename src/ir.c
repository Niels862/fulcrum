#include "ir.h"
#include <stdlib.h>

fuco_ir_node_t *fuco_ir_node_instr_new(fuco_opcode_t opcode) {
    fuco_ir_node_t *node = malloc(sizeof(fuco_ir_node_t));

    node->attrs = 0;
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
    if (node->attrs & FUCO_IR_LABEL) {
        fprintf(stream, ".L%ld:\n", node->imm.label);
    } else {
        fprintf(stream, "  %s (...)\n", 
                instr_descriptors[node->opcode].mnemonic);
    }
}

void fuco_ir_object_init(fuco_ir_object_t *object, fuco_ir_label_t label) {
    object->label = label;
    object->begin = object->end = NULL;
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
    ir->objects = malloc(FUCO_IR_OBJECTS_INIT_SIZE * sizeof(fuco_ir_node_t *));
    ir->size = 0;
    ir->cap = FUCO_IR_OBJECTS_INIT_SIZE;
    ir->label = 0;
}

void fuco_ir_destruct(fuco_ir_t *ir) {
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_object_destruct(&ir->objects[i]);
    }
    free(ir->objects);
}

fuco_ir_object_t *fuco_ir_add_object(fuco_ir_t *ir, fuco_ir_label_t label) {
    if (ir->size >= ir->cap) {
        ir->objects = realloc(ir->objects, 
                              2 * ir->cap * sizeof(fuco_ir_object_t));
        ir->cap *= 2;
    }
    
    fuco_ir_object_t *object = &ir->objects[ir->size];
    fuco_ir_object_init(object, label);

    ir->size++;

    return object;
}

void fuco_ir_add_instr(fuco_ir_object_t *object, fuco_opcode_t opcode) {
    fuco_ir_node_t *node = fuco_ir_node_instr_new(opcode);
    if (object->begin == NULL) {
        object->begin = object->end = node;
    } else {
        object->end->next = node;
        object->end = node;
    }
}
