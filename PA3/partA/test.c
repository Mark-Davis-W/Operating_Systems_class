#include "array.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define ARR_LEN 255
#define ARR_SIZE 10

/*When trying to do different amounts of consumers & producers it hangs
because they are blocking till more is added but there is
no more. Need a struct atomic semaphore to count so consumers
can check to see if there are still producers active and waiting*/

int P_TH_SZ = 2;
int C_TH_SZ = 1;
int loops = 10;

void* producer(void *arg) {
    printf("producer() created\n"); 
    Queue *s = (Queue*)arg;
    //create string array with nulls for use
    char str[ARR_LEN] = {'\0'};
    
    // push random string until full 
    for(int j=loops; j>0; j--){
        //str = calloc(1,sizeof(char) * s->maxLen);
        //str = '\0';
	str[0] = 'A';
	//str[253] = '9';
	str[254] = '\0'; 
	for(int k=1; k<20; k++){ str[k] = 'A' + (rand() % 26);}
	printf("I'm attempting %s.\n",str);
	queue_push(s, str);
	//if(i) break; 
    }
    printf("done checking this producer.\n");
    pthread_exit(NULL);
}

void* consumer(void *arg) {
    
    printf("consumer() created\n");
    Queue *s = (Queue*)arg;
    int j = loops * P_TH_SZ;
    char* i = calloc(s->maxLen, sizeof(char));
    // pop and print until empty
    //for(int j=0; j<100; j++){
    while(j){
	queue_pop(s,i);
        printf("done checking this consumer.\nGot: %s.\n",i);
        j--;
    }
    free(i);
    pthread_exit(NULL);
}


int main(int argc, char **argv) {
   struct timeval start;
   struct timeval end;
   //struct timeval final;
   gettimeofday(&start,NULL);
   
   if(argc == 2){
       loops = atoi(argv[1]);
       printf("Strings: %d.\n",atoi(argv[1]));
    }else if(argc == 3){
       loops = atoi(argv[1]);
       P_TH_SZ = atoi(argv[2]);
       printf("Strings: %d.\tProducers: %d.\n",atoi(argv[1]),atoi(argv[2]));
    }else if(argc == 4){
       loops = atoi(argv[1]);
       P_TH_SZ = atoi(argv[2]);
       C_TH_SZ = atoi(argv[3]);
       printf("Strings: %d.\tProducers: %d.\tConsumers: %d.\n",atoi(argv[1]),atoi(argv[2]),atoi(argv[3]));
    }
    
    
    //printf("I got here!!!\n");
    pthread_t p_thr_id[P_TH_SZ];
    pthread_t c_thr_id[C_TH_SZ];
    int i;
    
    // init stack, exit if failed
    Queue *my_queue = queue_init(ARR_SIZE, ARR_LEN);
    
    if (my_queue == NULL){
        printf("I'm failing in the beginning.\n");
        exit(-1);
    }
    //printf("my_queue capacity: %d\n",my_queue->capacity);
    
    for(i=0; i<P_TH_SZ; i++){
        if(pthread_create(&p_thr_id[i], NULL, &producer, my_queue) != 0) {
	    perror("Producer thread create failed");
	}   
    }
    
    for(i=0; i<C_TH_SZ; i++){
        if(pthread_create(&c_thr_id[i], NULL, &consumer, my_queue) != 0) {
	    perror("Consumer thread create failed");
	}
    }
    
    for(i=0; i<P_TH_SZ; i++){
        pthread_join(p_thr_id[i],NULL);
    }    
    for(i=0; i<C_TH_SZ; i++){
        pthread_join(c_thr_id[i],NULL);
    }
    
    //int p = producer(my_queue);
    //int c = consumer(my_queue);
    
    //if(p){printf("Producer was stopped early.\n");}
    //else {printf("Producer completed fully!\n");}
    
    //if(c){printf("Consumer was stopped early.\n");}
    //else {printf("Consumer completed fully!\n");}
    
    // free the whole structure
    queue_free(my_queue);
    printf("Made it to the end!\n");
    
    gettimeofday(&end,NULL);
    double final = (end.tv_sec - start.tv_sec) * 1000.0;
    final += (end.tv_usec - start.tv_usec) / 1000.0;
    printf("Elapsed time: %f ms.\n", final);
     
    exit(0);
}

