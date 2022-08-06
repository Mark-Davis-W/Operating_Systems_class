
/* 
 * File: multi-lookup.c
 * Author: Mark Davis
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2021/10/23
 * Description:
 *	This file incorporates my dynamically allocated shared queue 
 *	array with other mutexes and variables for a multi-thread resolver.
 */

#include "multi-lookup.h"

/* --- CHECK IF REQUESTORS ARE DONE ---*/
void req_done(Handle* h, int* i){
    sem_wait(&h->req_count_mute);
      *i = h->req_count;
    sem_post(&h->req_count_mute);
    //return;
}

/* --- INITIALIZE ALL VARIABLES --- */
Handle* handle_init(FILE* s, FILE* r, int i, char** arg){
    //create space on heap for struct
    Handle* h = calloc(1, sizeof(Handle));
    
    if(!h){printf("Error allocating for HANDLING struct.\n");return NULL;}
    
    // Create array size for shared hostname buffer
    h->str = queue_init(ARRAY_SIZE,MAX_NAME_LENGTH);
    if(!h->str){printf("Error allocating for BUFFER queue.\n");return NULL;}
    
    // Create array for filenames for producers
    h->files = queue_init(MAX_INPUT_FILES,MAX_NAME_LENGTH);
    if(!h->files){printf("Error allocating for FILES queue.\n");return NULL;}
    
    // start checking at index 5 --> first filename
    for(int j=0; j<i; j++){
	queue_push(h->files,arg[j+5]);
    }
    
    // set how many requestors from command line args
    h->req_count = atoi(arg[1]);
    
    // save opened file ptr to struct
    h->service = s;
    h->result = r;
    
    // mutex to protect checking file queue
    if(sem_init(&h->req_mute,0,1))
    {
        perror("Error trying to initialize req mutex.");
        return NULL;
    }
    
    // mutex to limit requestor log access
    if(sem_init(&h->req_write,0,1))
    {
        perror("Error trying to initialize req write mutex.");
        return NULL;
    }
    
    // shared mutex to protect requestor count set earlier
    if(sem_init(&h->req_count_mute,0,1))
    {
        perror("Error trying to initialize req_count_mute mutex.");
        return NULL;
    }
    
    // mutex to limit resolver log access
    if(sem_init(&h->res_write,0,1))
    {
        perror("Error trying to initialize res mutex.");
        return NULL;
    }
    
    // shared mutex to limit printing to terminal
    if(sem_init(&h->prt_mute,0,1))
    {
	perror("Error trying to initialize stdout mutex.");
	return NULL;
    }
    
    return h;
}

/* --- REQUESTOR FUNCTION --- */
void producer(void* arg) {
    //printf("producer() created\n"); 
    
    Handle* h = (Handle*)arg;
    
    // initialize variables needed
    int files_read = 0;
    char input_file[MAX_NAME_LENGTH] = {'\0'};
    char host_name[MAX_NAME_LENGTH] = {'\0'};
    
    while(1){
	
	// keep checking if queue has a file to process
	// if not break constant loop and get out
	// protected in function
          if(isEmpty(h->files)){ 
		break;
	  }
	
	// already protected on queue side
	// should break eariler if empty
	queue_pop(h->files,input_file);
	
	FILE* open_file = fopen(input_file, "r");
	if(!open_file){
	    //Bad filename
	    sem_wait(&h->prt_mute);
	      //fprintf(stderr,"Error : Invalid input file < %s >.\n",input_file);
	    sem_post(&h->prt_mute);
	    continue;
	}
        // save return of getting a line from file
	int x = fscanf(open_file, "%255s", host_name);
	while(x != EOF){
	    // checking for empty lines
	    if(x < 0) continue;
             //printf("HERE\n");
	    
	    // protect file writing
	    sem_wait(&h->req_write);
		fprintf(h->service,"%s\n", host_name);
	    sem_post(&h->req_write);
	    
	    queue_push(h->str,host_name);
	    
	    //recalled at end for next loop checking
	    x = fscanf(open_file, "%255s", host_name);
	}
	
	// increment serviced files and close it 
	files_read++;
	fclose(open_file);
	
	// clear array to reuse
	memset(input_file, '\0', sizeof(input_file));
    }
    
    // protect & decrement how many requestors are active
    sem_wait(&h->req_count_mute);
       h->req_count--;
    sem_post(&h->req_count_mute);
    
    // done running, protect terminal write
    sem_wait(&h->prt_mute);
      fprintf(stdout, "Thread %ld serviced %d files.\n", pthread_self(), files_read);
    sem_post(&h->prt_mute);
    
}

/* --- RESOLVER FUNCTION --- */
void consumer(void* arg) {
    //printf("consumer() created\n");
    Handle* h = (Handle*)arg;
    
    // initalize variables needed
    int host_count = 0;
    int active_prod = 1;
    char host_name[MAX_NAME_LENGTH] = {'\0'};
    char ip_addr[MAX_IP_LENGTH] = {'\0'};
    
    // keep looping while there are active producers
    while(1){
	
	if(isEmpty(h->str) && !(active_prod)) break;
	
	queue_pop(h->str,host_name);
	
	if(host_name[0] == '\0') break;
	
	if(dnslookup(host_name,ip_addr,sizeof(ip_addr)) == UTIL_FAILURE){
	    sem_wait(&h->prt_mute);
              //fprintf(stderr, "Error : Invalid host name < %s >.\n", host_name);
	    sem_post(&h->prt_mute);
	    strncpy(ip_addr, "NOT_RESOLVED", sizeof(ip_addr));
	}
	
	if(ip_addr[0] == 'U'){
	    strncpy(ip_addr, "NOT_RESOLVED", sizeof(ip_addr));
	}
	
	sem_wait(&h->res_write);
	  fprintf(h->result, "%s, %s\n", host_name, ip_addr);
	sem_post(&h->res_write);
	
	// check if requestors still active
	req_done(h, &active_prod);
	
	host_count++;
	
	memset(host_name, '\0', sizeof(host_name));
    }
    
    sem_wait(&h->prt_mute);
      fprintf(stdout, "Thread %ld resolved %d hostnames.\n", pthread_self(), host_count);
    sem_post(&h->prt_mute);
    
}

/* --- CHECK VALID COMMAND LINE --- */
void check_input(int c, char** s){
    //check if args are at minimum -> stdout
    if(c < 6){
	printf("ERROR : Too few arguments provided.\n");
	printf("%s <# requestors> <# resolvers> <requestor log> <resolver log> [<data file(s)> ... ]\n",s[0]);
	exit(EXIT_FAILURE);
    }
    //check if too many data files given -> stderr
    if(c > 105){
          fprintf(stderr,"ERROR : Too many input files, max is %d.\n",MAX_INPUT_FILES);
          exit(EXIT_FAILURE);
      }
    //check if args are out of range -> stderr
    if(atoi(s[1]) < 1){
	fprintf(stderr,"ERROR : Minimum number of requestor threads is 1.\n");
	exit(EXIT_FAILURE);
    }
    if(atoi(s[1]) > MAX_REQ_THREADS){
	fprintf(stderr,"ERROR : Maximum number of requestor threads is %d.\n",MAX_REQ_THREADS);
	exit(EXIT_FAILURE);
    }
    if(atoi(s[2]) < 1){
	fprintf(stderr,"ERROR : Minimum number of resolver threads is 1.\n");
	exit(EXIT_FAILURE);
    }
    if(atoi(s[2]) > MAX_RES_THREADS){
	fprintf(stderr,"ERROR : Maximum number of resolver threads is %d.\n",MAX_RES_THREADS);
	exit(EXIT_FAILURE);
    }
    
}

/* --- CLEAR ALL MEMORY --- */
int handle_destroy(Handle* h){
    int i = 0;
    i = queue_free(h->files);
    i = queue_free(h->str);
    
    
    if(sem_destroy(&h->req_mute))
    {
	perror("Error trying to destroy req_mute mutex");
	i = -1;
    }
    
    if(sem_destroy(&h->req_write))
    {
	perror("Error trying to destroy req_write mutex");
	i = -1;
    }
    
    if(sem_destroy(&h->req_count_mute))
    {
	perror("Error trying to destroy req_count_mute mutex");
	i = -1;
    }
    
    if(sem_destroy(&h->res_write))
    {
	perror("Error trying to destroy res_write mutex");
	i = -1;
    }
    
    if(sem_destroy(&h->prt_mute))
    {
	perror("Error trying to destroy prt_mute mutex");
	i = -1;
    }
    
    if(fclose(h->service))
    {
	perror("Error trying to close file");
	i = -1;
    }

    if(fclose(h->result))
    {
	perror("Error trying to close file");
	i = -1;
    }
    
    free(h);
    h = NULL;
    return i;
}



int main(int argc, char *argv[]) {
    struct timeval start;
    struct timeval end;
    gettimeofday(&start,NULL);
    
    check_input(argc,argv);
    
    int num_of_req = atoi(argv[1]);
    int num_of_res = atoi(argv[2]);
    int num_of_files = (argc - 5);
    
    // Using 'w' not 'w+' because only want
    // to be able to write '+' can read also
    FILE* service_out_log = fopen(argv[3], "w");
    if(!service_out_log){perror("Error in opening/creating service log."); exit(EXIT_FAILURE);}
    
    FILE* results_out_log = fopen(argv[4], "w");
    if(!results_out_log){perror("Error in opening/creating results log."); exit(EXIT_FAILURE);}
    
    pthread_t p_thr_id[num_of_req];
    pthread_t c_thr_id[num_of_res];
    
    Handle* h = handle_init(service_out_log,results_out_log,num_of_files,argv);
    if(!h){perror("Error in initialization of Handle struct."); exit(EXIT_FAILURE);}
    
    int i;
    for(i=0; i<num_of_req; i++){
        if(pthread_create(&p_thr_id[i], NULL, (void*)&producer, h) != 0) {
	    perror("Requestor thread create failed");
	}   
    }
    
    for(i=0; i<num_of_res; i++){
        if(pthread_create(&c_thr_id[i], NULL,(void*)&consumer, h) != 0) {
	    perror("Resolver thread create failed");
	}
    }
    
    for(i=0; i<num_of_req; i++){
        if(pthread_join(p_thr_id[i],NULL) != 0){
	    perror("Requestor thread join failed");
	}
    }    
    for(i=0; i<num_of_res; i++){
        if(pthread_join(c_thr_id[i],NULL) != 0){
	    perror("Resolver thread join failed");
	}
    }
    
    //printf("Made it to the end!\n");
    
    //Cleanup all memory
    int err = 0;
    err = handle_destroy(h);
    
    // get curr time & calc runtime
    gettimeofday(&end,NULL);
    double final = (end.tv_sec - start.tv_sec) * 1000;
    final = (final + ((end.tv_usec - start.tv_usec) / 1000.0)) / 1000.0;
    printf("%s: total time is %f seconds.\n",argv[0], final);
    
    if(err)
    {
	perror("Error in cleanup of memory");
	exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS);
}

