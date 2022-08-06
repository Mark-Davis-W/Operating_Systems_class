/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    for (int i = 0; i < MAXPROCESSES; i++) {
        // printf("looking at process= %d\t",i);
        // printf("looking at pc: %ld\t",q[i].pc);
        // printf("looking at page: %ld\n",q[i].pc/PAGESIZE);
        // printf("\nI was called!!\t\tprocess= %d\n",i);

        int curpage = q[i].pc / PAGESIZE;
        
        for (int j = 0; j < MAXPROCPAGES; j++) {
            
            (j == curpage) ? pagein(i,j) : pageout(i,j);
        }
    }
} 
