#include "ir.h"
#include <stdlib.h>

fuco_ir_node_t *fuco_ir_instr_new() {
    fuco_ir_node_t *node = malloc(sizeof(fuco_ir_node_t));

    return node;
}

fuco_ir_node_t *fuco_ir_label_new() {
    static fuco_ir_label_t label = 1;
    
    fuco_ir_node_t *node = malloc(sizeof(fuco_ir_node_t));
    
    node->attrs = FUCO_IR_LABEL;
    node->imm.label = label;
    label++;

    return node;
}

void fuco_ir_join(fuco_ir_node_t *pre, fuco_ir_node_t *post) {
    fuco_ir_node_t *end = pre;
    
    while (end->next != NULL) {
        end = end->next;
    }
    end->next = post;


}

void fuco_ir_node_free(fuco_ir_node_t *node) {
    while (node != NULL) {
        fuco_ir_node_t *next = node->next;
        free(node);
        node = next;
    }
}

void fuco_ir_init(fuco_ir_t *ir) {
    ir->objects = malloc(FUCO_IR_OBJECTS_INIT_SIZE * sizeof(fuco_ir_node_t *));
    ir->size = 0;
    ir->cap = FUCO_IR_OBJECTS_INIT_SIZE;
    ir->label = 0;
}

void fuco_ir_destruct(fuco_ir_t *ir) {
    for (size_t i = 0; i < ir->size; i++) {
        fuco_ir_node_free(&ir->objects[i]);
    }
    free(ir->objects);
}
