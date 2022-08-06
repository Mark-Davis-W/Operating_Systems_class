#include "stack.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    stack my_stack;
    int i;

    if (stack_init(&my_stack) < 0) exit(-1);                // init stack, exit if failed
    while (stack_push(&my_stack, rand() % 10) == 0);        // push random numbers until full
    while (stack_pop(&my_stack, &i) == 0) printf("%d ", i); // pop and print until empty
    stack_free(&my_stack);                                  // destroy stack
    exit(0);
}


