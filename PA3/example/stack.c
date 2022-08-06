#include "stack.h"

int stack_init(stack *s) {                  // init the stack
    s->top = 0;
    return 0;
}

int stack_push(stack *s, int element) {     // place element on the top of the stack
    if (s->top >= STACK_SIZE) return -1;
    s->array[s->top++] = element;
    return 0;
}

int stack_pop(stack *s, int *element) {     // remove element from the top of the stack
    if (s->top < 1) return -1;
    *element = s->array[--s->top];
    return 0;
}

void stack_free(stack *s) {                 // free the stack's resources
}


