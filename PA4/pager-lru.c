 /* Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <signal.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */
    
    /* Static vars */
    // static int initialized = 0;
    static unsigned long tick = 0; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    // if((q[0].active) && (tick > 699)) {
    //     int active;
    //     for(int j=0; j<4; j++) {
    //         active = 0;
    //         for(int i=0; i<MAXPROCPAGES; i++) {
    //             if(q[j].pages[i]) active++;
    //             printf("%ld",q[j].pages[i]);
    //         }
    //         printf("\n");
    //         if(active == 0) {
    //             // printf("\nWe got a new process!!\n");
    //             // break;
    //         }
    //         printf("\nActives: %d\tProcess: %d\n",active,j);
    //     }
    //         // 
    //     printf("\nPC: %ld.\n",q[19].pc);    
    //     raise(SIGINT);
    // }

    /* Local vars */
    int proctmp;
    int pagetmp;
    int remove;
    int currpage;
    int tmptick;
    int per_proc_ct;
    int active_proc;
    
    if(!tick){

        active_proc = MAXPROCESSES;
        /*--let's keep 10 frames in reserve to start--*/
        per_proc_ct = (PHYSICALPAGES -10)/active_proc;
        for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
            /*Should never happen but just in case*/
            if(!q[proctmp].active) {continue;}

            /*--Initialize timekeeper 2D array--*/
            for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){

                timestamps[proctmp][pagetmp] = 0; 
            }

            /*--Let's give them all a couple pages to start--*/
            for(pagetmp=0; pagetmp < per_proc_ct; pagetmp++){
                // printf("\nHERE: %d\n",proctmp);

                int page = (q[proctmp].pc / PAGESIZE) +pagetmp;
                if(pagein(proctmp,page)) {
                    timestamps[proctmp][page] = tick + pagetmp;
                    // printf("Tick + page + proc: %ld\n",tick+pagetmp);
                }
                // else {
                //     printf("Error intializing first pages allocated to process: %d\tpage: %d\n",proctmp,page);
                // }
                // printf("madeit here. proc: %d\tpage: %d\n",proctmp,page);
            }
        }
        /*--So it only runs this once--*/
        // initialized = 1;
    }
    /*---------FINISHED INITIALIZING-----------*/

    /*---------------TODO: Implement LRU Paging----------------*/

    /*------MAIN TO LOOP THROUGH AND DO WORK----*/
    for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++) {
        
        /*--If the process isn't active we can skip it--*/
        if(!q[proctmp].active) {
            continue;
        }

        active_proc = 0;
        for(int j=0; j < MAXPROCESSES; j++) {
            if(q[j].active) active_proc++;
        }
        // printf("Active processes: %d\n",active_proc);
        if(active_proc) {
            per_proc_ct = (PHYSICALPAGES -10)/active_proc;
            if(per_proc_ct > MAXPROCPAGES) {
                per_proc_ct = MAXPROCPAGES;
            }
        }

        currpage = q[proctmp].pc / PAGESIZE;
        /*---Checking if current page already allocated---*/
        if(q[proctmp].pages[currpage]) {
            continue;
        }
        
        timestamps[proctmp][currpage] = tick;

        /*---If not allocated let's get it in there, we need it---*/
        if(pagein(proctmp,currpage)) {
            pagein(proctmp,currpage+1%MAXPROCPAGES);
            // printf("\nHERE: %ld,\tProcess: %d\n",q[proctmp].pages[0],proctmp);
            // continue;
        }
        // else {printf("Unable to pagein.\n");}

        /*--See how many pages are active for this process--*/
        int active_pages = 0;
        for(int j=0; j<MAXPROCPAGES; j++) {
            if(q[proctmp].pages[j]) { active_pages++;}
        }

        /*----Let's see if we need to EJECT some pages----*/
        /*--Plus one here to make sure we can EJECT something--*/
        tmptick = tick+1;

        /*--Checking some global and local variables with augments to use the 10 frame buffer--*/
        if(per_proc_ct < active_pages+2) {
            for(int i=0; i < MAXPROCPAGES; i++){
                /*--Not allocated (skip it)--*/
                if(!q[proctmp].pages[i]) {
                    continue;
                }
                /*--Save oldest (earliest tick) to EJECT--*/
                if(timestamps[proctmp][i] < tmptick) {
                    tmptick = timestamps[proctmp][i];
                    remove = i;
                }
            }

            /*--EJECT--*/
            pageout(proctmp,remove);
            
        }
        
    
    }
    /*--ADD another clock tick--*/
    /* advance time for next pageit iteration */
    tick++;
}