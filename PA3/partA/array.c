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
    
    if(a_size < 0 || a_size > 10)
    {  
       printf("Error: invalid queue size entered\n");
       return NULL;
    }
    else if(len != 255)
    {
       printf("Error: invalid queue length entered\n");
       return NULL;
    }
    
    Queue *s = calloc(1,sizeof(*s));
    if(!s){printf("Error allocating for Queue structure\n");return NULL;}
    s->arr = calloc(a_size,sizeof(char*));
    if(!s->arr){printf("Error allocating for array in structure\n");return NULL;}
    for(int i=0; i<a_size; i++){
       s->arr[i] = calloc(len,sizeof(char));
    }
    
    
    s->capacity = a_size;
    s->maxLen = len;
    printf("Capacity set to: %d\n",s->capacity);
    
    //if(!(s->arr))
    //{
    //   printf("Error creating array, failed in calloc.\n");
    //   return NULL;
    //}
    //s->arr[0] = 'A';
    //printf("type of variable for arr: %ld\n", sizeof(s->arr));

    s->size = s->in = s->counter = 0;
    s->out = 0; 
    printf("Current out in init: %d\n",s->out);
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
    
    return s;
}

int isFull(Queue *s){
    
    if (s->in == s->out && s->size > 9) return -1;
    return 0;
}

int isEmpty(Queue *s){
    
    if (s->in == s->out && s->size < 1) return -1;
    return 0;
}

//custom string copiers//
void push_str(Queue *s, char *str){
    //cycle through char array to push on queue 
    int i;
    for(i = 0; i < s->maxLen && str[i] != '\0'; i++){
        //printf("I'm trying. %c...\n",s->arr[s->in][i%2]);
        s->arr[s->in][i] = str[i];
    }
    //for( ; i < s->maxLen; i++){
    //    s->arr[s->in][i] = '\0';
    //} 
    //printf("New in saved:\n%s\n", str);
    //return str;
}

void* pop_str(Queue *s, char* str){
    //char str[s->maxLen];
    int i;
    for(i = 0; i < s->maxLen && s->arr[s->out][i] != '\0'; i++){
       str[i] = s->arr[s->out][i];
       //if( s->arr[i] == '\0') break; //return str;
       s->arr[s->out][i] = '\0';
    }
    return str;
}

// place element in allocated queue
void* queue_push (Queue *s, char *str) {
    //printf("made it in push.\tcurrent: %d\n",s->capacity);
    
    sem_wait(&s->space_ready);
      sem_wait(&s->mute);
      
      //printf("s in before: %d\n",s->in);
      s->counter++;
      s->size++;
      //s->arr[s->in] = str;
      push_str(s,str);
      //printf("\nHERE:\t%p\n\n",s->arr[s->in]);
      printf("New in saved:\n%s\n", s->arr[s->in]);
      s->in++;
      s->in %= s->capacity;
      
      //printf("s in after: %d\n",s->in);
      
      sem_post(&s->mute);
    sem_post(&s->items_ready);
    
    //printf("I made it out of push\n");
    return 0;
}

// remove element from queue
void* queue_pop (Queue *s, char* str) {
    //printf("Current in to pop: %d\n", (s->in));
    
    sem_wait(&s->items_ready);
      sem_wait(&s->mute);
      
      //printf("s out before: %d\n",s->out);
      s->size--;
      printf("Removing\n%s\nfrom queue\n", s->arr[s->out]);
      pop_str(s,str);
      //free(s->arr[s->out]);
      //s->arr[s->out] = NULL;
      s->out++;
      s->out %= s->capacity;
      //printf("s out after: %d\n",s->out);
      
      sem_post(&s->mute);
    sem_post(&s->space_ready);
    
    return 0;
}

// free all the queue's resources
int queue_free (Queue *s) {
    printf("Produced %d things.\n",s->counter);
    
    //printf("Destroying semaphores and checking for error.\n");
    if(sem_destroy(&s->space_ready))
    {
        perror("Error trying to destroy space_ready semaphore\n");
	return QUEUE_FAIL;
    }
    
    if(sem_destroy(&s->items_ready))
    {
        perror("Error trying to destroy items_ready semaphore\n");
	return QUEUE_FAIL;
    }
    
    //printf("Destroying mutex and checking for error.\n");
    if(sem_destroy(&s->mute))
    {
        perror("Error trying to destroy Cmutex\n");
	return QUEUE_FAIL;
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
    
    return 0;
}

