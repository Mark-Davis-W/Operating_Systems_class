
/* 
 * File: array.h
 * Author: Mark Davis
 * Project: CSCI 3753 Programming Assignment 3 Part A
 * Create Date: 2021/10/16
 * Description:
 *      This has the function declarartions for my dynamically 
 *      allocated queue array that is thread safe.
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <semaphore.h>

typedef struct{
    // array index indicating where the next out & in is
    int out;
    int in;
    // semaphore & mutex to keep track and lockout
    sem_t mute;
    sem_t empty_mute;
    sem_t space_ready;
    sem_t items_ready;
    // capacity of array needed
    int capacity;
    // current size of items in array
    int size;
    int maxLen;
    int counter;
    // storage array for strings
    char **arr;
} Queue;

// init the stack
Queue* queue_init (int a_size, int len);

// helper to copy and return from array
void push_str(Queue* s, char* str);

// helper to copy strings
void pop_str(Queue* s, char* str);

// place element on the top of the stack
void queue_push (Queue* s, char* str);     

// remove element from the top of the stack
void queue_pop (Queue* s, char* str);

// free the stack's resources
int queue_free (Queue *s);                  

int isEmpty(Queue *s);

int isFull(Queue *s);

#endif
