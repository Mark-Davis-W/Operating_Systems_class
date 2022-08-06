#ifndef STACK_H
#define STACK_H

#define STACK_SIZE 10                       // max elements in stack

typedef struct {
    int array[STACK_SIZE];                  // storage array for integers
    int top;                                // array index indicating where the top is
} stack;

int  stack_init(stack *s);                  // init the stack
int  stack_push(stack *s, int element);     // place element on the top of the stack
int  stack_pop (stack *s, int *element);    // remove element from the top of the stack
void stack_free(stack *s);                  // free the stack's resources

#endif
