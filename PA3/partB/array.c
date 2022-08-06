
/* 
 * File: array.c
 * Author: Mark Davis
 * Project: CSCI 3753 Programming Assignment 3 Part A
 * Create Date: 2021/10/16
 * Description:
 *      This has the implementations for my dynamically 
 *	allocated queue array that is thread safe.
 */

#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include <string.h>

#define QUEUE_SUCC 0
#define QUEUE_FAIL -1

// init the queue
Queue* queue_init (int a_size, int len) {
    
    Queue *s = calloc(1,sizeof(Queue));
    if(!s){printf("Error allocating for Queue structure\n");return NULL;}
    s->arr = calloc(a_size,sizeof(char*));
    
    if(!s->arr){printf("Error allocating for array in structure\n");return NULL;}
    
    //printf("Address of base array allocated: %p\n",s->arr);
    for(int i=0; i<a_size; i++){
       s->arr[i] = calloc(len,sizeof(char));
       //printf("Addresses for array allocated: %p\n",s->arr[i]);
    }
    
    s->capacity = a_size;
    s->maxLen = len;
    //printf("Capacity set to: %d\n",s->capacity);
    
    s->size = s->in = s->counter = 0;
    s->out = 0; 
    //printf("Current out in init: %d\n",s->out);
    //printf("Inside init, in addr: %d\tout addr: %d\n",s->in,s->out);
    //printf("looking at first index in array: %d\n",(s->arr[s->out]));
    
    if(sem_init(&s->mute,0,1))
    {
        perror("Error trying to initialize mutex.");
	return NULL;
    }
    
    if(sem_init(&s->space_ready,0,s->capacity))
    {
        perror("Error trying to initialize reader sem.");
	return NULL;
    }
    
    if(sem_init(&s->items_ready,0,0))
    {
        perror("Error trying to initialize writer sem.");
	return NULL;
    }
    
    if(sem_init(&s->empty_mute,0,1))
    {
	perror("Error trying to initialize empty mutex.");
	return NULL;
    }
    
    return s;
}

int isFull(Queue *s){
    sem_wait(&s->empty_mute);
    if (s->in == s->out && s->size > s->capacity)
        { sem_post(&s->empty_mute); return -1; }
    sem_post(&s->empty_mute);
    
    return 0;
}

int isEmpty(Queue *s){
    sem_wait(&s->empty_mute);
      if (s->in == s->out && s->size < 1)
        { sem_post(&s->empty_mute); return -1; }
    sem_post(&s->empty_mute);
    
    return 0;
}

//custom string copiers//
void push_str(Queue *s, char *str){
    //cycle through char array to push on queue 
    int i;
    for(i = 0; i < s->maxLen && str[i] != '\0'; i++){
        //printf("I'm trying. %c...\n",s->arr[s->in][i%2]);
	s->arr[s->in][i] = '\0';
        s->arr[s->in][i] = str[i];
	str[i] = '\0';
    }
}

void pop_str(Queue *s, char* str){
    //char str[s->maxLen];
    int i;
    
    for(i = 0; i < s->maxLen && s->arr[s->out][i] != '\0'; i++){
        str[i] = '\0';
	str[i] = s->arr[s->out][i];
	s->arr[s->out][i] = '\0';
    }
}

// place element in allocated queue
void queue_push (Queue *s, char *str) {
    //printf("made it in push.\tcurrent: %d\n",s->capacity);
    
    sem_wait(&s->space_ready);
      sem_wait(&s->mute);
      
      //printf("s in before: %d\n",s->in);
      s->counter++;
      s->size++;
      push_str(s,str);
      //printf("\nHERE:\t%p\n\n",s->arr[s->in]);
      //printf("New in saved:\n%s\n", s->arr[s->in]);
      s->in++;
      s->in %= s->capacity;
      
      //printf("s in after: %d\n",s->in);
      
      sem_post(&s->mute);
    sem_post(&s->items_ready);
    
    //printf("I made it out of push\n");
    return;
}

// remove element from queue
void queue_pop (Queue *s, char* str) {
    //printf("Current in to pop: %d\n", (s->in));
    
    sem_wait(&s->items_ready);
      sem_wait(&s->mute);
      
      //printf("s out before: %d\n",s->out);
      s->size--;
      //printf("Removing\n%s\nfrom queue\n", s->arr[s->out]);
      pop_str(s,str);
      s->out++;
      s->out %= s->capacity;
      //printf("s out after: %d\n",s->out);
      
      sem_post(&s->mute);
    sem_post(&s->space_ready);
    
    return;
}

// free all the queue's resources
int queue_free (Queue *s) {
    //printf("Produced %d things.\n",s->counter);
    
    int k = 0;
    //printf("Destroying semaphores and checking for error.\n");
    if(sem_destroy(&s->space_ready))
    {
        perror("Error trying to destroy space_ready semaphore");
	k = QUEUE_FAIL;
    }
    
    if(sem_destroy(&s->items_ready))
    {
        perror("Error trying to destroy items_ready semaphore");
	k = QUEUE_FAIL;
    }
    
    //printf("Destroying mutexes and checking for error.\n");
    if(sem_destroy(&s->mute))
    {
        perror("Error trying to destroy base mutex");
	k = QUEUE_FAIL;
    }
    
    if(sem_destroy(&s->empty_mute))
    {
        perror("Error trying to destroy empty mutex");
	k = QUEUE_FAIL;
    }
    
    for(int i=0; i<s->capacity; i++){
        free(s->arr[i]);
        s->arr[i] = NULL;
    }
    
    free(s->arr);
    s->arr = NULL;
    //Lastly free the struct
    free(s);
    s = NULL;
    
    return k;
}

