#ifndef FUCO_TSTACK_H
#define FUCO_TSTACK_H

#include <stdlib.h>

/*
 * Stack macros to work with stacks of arbitrary types 
 * Stack should have the following fields: 
 * - data of type T*
 * - size of type size_t
 * - cap of type size_t
 * */

#define FUCO_TSTACK_INIT_SIZE 8

#define FUCO_TSTACK_SIZEOF(s) sizeof(*((s)->data))

#define FUCO_TSTACK_INIT(s) \
        ( \
            (s)->data = malloc(FUCO_TSTACK_INIT_SIZE * FUCO_TSTACK_SIZEOF(s)), \
            (s)->size = 0, \
            (s)->cap = FUCO_TSTACK_INIT_SIZE \
        )

#define FUCO_TSTACK_RESIZE(s) \
        ( \
            (s)->data = realloc((s)->data, \
                                2 * (s)->cap * FUCO_TSTACK_SIZEOF(S)), \
            (s)->cap *= 2 \
        )

#endif
