
/* 
 * File: multi-lookup.h
 * Author: Mark Davis
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2021/10/23
 * Description:
 *      This has the function declarations for my thread safe 
 *      array with other mutexes & variables for a dns resolver.
 */

#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

/* Imported libraries */
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

/* User defined/local */
#include "util.h"
#include "array.h"

/* Predefined limits */
#define ARRAY_SIZE 10
#define MAX_INPUT_FILES 100
#define MAX_REQ_THREADS 10
#define MAX_RES_THREADS 10
#define MAX_NAME_LENGTH 255
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

typedef struct{
    Queue* str;
    FILE* service;
    FILE* result;
    sem_t req_mute;
    sem_t req_write;
    sem_t req_count_mute;
    sem_t res_write;
    sem_t prt_mute;
    int req_count;
    Queue* files;
} Handle;

void req_done(Handle* h, int* i);

Handle* handle_init(FILE* s, FILE* r,int i, char** arg);

void check_input(int c, char** s);

void consumer(void* arg);

void producer(void* arg);

int handle_destroy(Handle* h);

#endif
