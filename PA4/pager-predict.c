/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>
#include <signal.h>

#include "simulator.h"

typedef _Bool bool;

typedef struct {
   int faulted;               // how many times faulted
   int currpage;
   int prev_page;             // previous page
   bool looper;               // has it looped?
   bool dbl_loop;             // multiple faults and pages looped
   bool backward;             // is it backward?
   bool branch;               // is it inner branch?
} Predict;

/* Static var */
Predict p[MAXPROCESSES];    //struct to hold all

void p_init() {

   /* Init complex static vars here */
   for(int k=0; k < MAXPROCESSES; k++){
      p[k].faulted = 0;

      p[k].currpage = 0;
      p[k].prev_page = 0;

      p[k].looper = 0;
      p[k].dbl_loop = 0;
      p[k].backward = 0;
      p[k].branch = 0;
   }

}

void proc_reset(int k) {
   p[k].faulted = 0;

   p[k].currpage = 0;
   p[k].prev_page = 0;

   p[k].looper = 0;
   p[k].dbl_loop = 0;
   p[k].backward = 0;
   p[k].branch = 0;
}

   
int calc_pgs(Pentry q[MAXPROCESSES]) {
   int hold = 0;
   int ct = MAXPROCPAGES +1;
   for(int i=0; i<MAXPROCESSES; i++) {
      if(q[i].active) hold++;
   }
   if(hold) 
      ct = (PHYSICALPAGES -10) / hold;

   if(ct < MAXPROCPAGES)
      return ct;
      
   return MAXPROCPAGES;
}

void pageOut_global(Pentry q[MAXPROCESSES]) {

   for(int j=0; j<MAXPROCESSES; j++) {
      if(!q[j].active) continue;
      if(p[j].faulted) continue;
      
      int currpage = q[j].pc / PAGESIZE;

      for(int i=0; i<MAXPROCPAGES; i++) {
         if(!q[j].pages[i]) continue;
         if((i != currpage) && (i != (currpage+1)%MAXPROCPAGES)) {
            pageout(j,i);
            continue;
         }
      }
   } 
}

void handle_back(int proc,Pentry q[MAXPROCESSES]) {
   int currpage = q[proc].pc/PAGESIZE;

   if(!q[proc].pages[currpage]) { pagein(proc,currpage); }

   if(currpage > 1) { proc_reset(proc); return; }

   if(!q[proc].pages[0]) { pagein(proc,0); }
   if(!q[proc].pages[1]) { pagein(proc,1); }

   for(int i=2; i<MAXPROCPAGES; i++) {
      if(!q[proc].pages[i]) continue;
         pageout(proc,i);
   }
}

void handle_looper(int proc, Pentry q[MAXPROCESSES]) {
   int currpage = q[proc].pc/PAGESIZE;

   if(!q[proc].pages[currpage]) { pagein(proc,currpage); }
   
   if(currpage > 4) { proc_reset(proc); return; }
   
   if(currpage == 4) {
      if(q[proc].pc > 1000) {
         if(!q[proc].pages[0]) { pagein(proc,0); }
      }

      if(q[proc].pages[3]) { pageout(proc,3); }
   }
   else { if(!q[proc].pages[currpage+1] && currpage+1 < 3) { pagein(proc,currpage+1); } }

   for(int i=0; i<MAXPROCPAGES; i++) {
      if(!q[proc].pages[i]) continue;
      if(i != currpage && currpage != 4 && i != currpage+1){
         pageout(proc,i);
      }
   }
}

void handle_dbl_loop(int proc, Pentry q[MAXPROCESSES]) {
   int currpage = q[proc].pc/PAGESIZE;

   if(currpage > 7) { proc_reset(proc); return; }

   if(!q[proc].pages[currpage]) { pagein(proc,currpage); }
   
   if(currpage == 6) {
      if(q[proc].pc > 1550) {
         if(!q[proc].pages[4]) { pagein(proc,4); }
         if(!q[proc].pages[0]) { pagein(proc,0); }
      }

      if(q[proc].pages[5]) { pageout(proc,5); }
      
   }
   else { if(!q[proc].pages[currpage+1] && currpage+1 <7) { pagein(proc,currpage+1); } }
   
   for(int i=0; i<MAXPROCPAGES; i++) {
      if(!q[proc].pages[i]) continue;
      if(i != currpage && currpage != 6 && i != currpage+1){
         pageout(proc,i);
      }
   }
}

void handle_branch(int proc, Pentry q[MAXPROCESSES]) {
   int currpage = q[proc].pc/PAGESIZE;

   if(currpage > 5) { proc_reset(proc); return; }

   if(!q[proc].pages[currpage]) { pagein(proc,currpage); }

   if(currpage == 5) {
      if(q[proc].pc > 1500) {
         if(q[proc].pages[4]) { pageout(proc,4); }
         if(q[proc].pages[1]) { pageout(proc,1); }
         if(q[proc].pages[2]) { pageout(proc,2); }
      }
      if(!q[proc].pages[0]) { pagein(proc,0); }
      
   }
   else if(currpage == 1) {
      if(q[proc].pc > 395) {
         if(!q[proc].pages[5]) { pagein(proc,5); }
         if(!q[proc].pages[2]) { pagein(proc,2); }
      }  
      if(q[proc].pages[0]) { pageout(proc,0); }
      
   }
   else { if(!q[proc].pages[currpage+1] && currpage+1 < 5) { pagein(proc,currpage+1); } }

   
   for(int i=0; i<MAXPROCPAGES; i++) {
      if(!q[proc].pages[i]) continue;

      if(i != currpage && currpage != 5 && currpage != 1 && i != currpage+1){
         pageout(proc,i);
      }
   }
}

void pageit(Pentry q[MAXPROCESSES]) { 

   /* initialize static vars on first run */
   static ulong tick = 0;
   static bool initialized = 0;
   static int per_proc_ct = 0;
   static int tmp_curr_page;
   
   // if((q[0].active) && (tick > 10208)) {
      // int active;
      // for(int j=0; j<4; j++) {
      //    active = 0;
      //    for(int i=0; i<MAXPROCPAGES; i++) {
      //          if(q[j].pages[i]) active++;
      //          printf("%ld",q[j].pages[i]);
      //    }
      //    printf("\n");
      //    if(active == 0) {
      //          printf("\nWe got a new process!!\n");
      //          // break;
      //    }
      //    printf("\nActives: %d\tProcess: %d\n",active,j);
      // }
      //    // 
      // printf("\nPC: %ld.\n",q[19].pc);    
   //    raise(SIGINT);
   // }

   if(!initialized) {
      p_init();
      per_proc_ct = calc_pgs(q);
      for(int j=0; j<MAXPROCESSES; j++) {
         for(int i=0; i<per_proc_ct; i++) {
            pagein(j,i);
         }
      }
      
      initialized = 1;
   }

   for(int proctmp=0; proctmp<MAXPROCESSES; proctmp++) {
      if(!q[proctmp].active) continue;

      int active_pages = 0;
      for(int j=0; j<MAXPROCPAGES; j++) {
         if(q[proctmp].pages[j]) { active_pages++; }
      }
      
      if(!active_pages && (q[proctmp].pc == 0)) {
         proc_reset(proctmp);
      }
            
      tmp_curr_page = (q[proctmp].pc)/PAGESIZE;

      /*-keeping track of pages-*/
      if(tmp_curr_page != p[proctmp].prev_page) {
         p[proctmp].prev_page = p[proctmp].currpage;
         p[proctmp].currpage = tmp_curr_page;
      }


      if(p[proctmp].backward) { handle_back(proctmp,q); continue;}

      if(p[proctmp].looper) { handle_looper(proctmp,q); continue;}

      if(p[proctmp].branch) { handle_branch(proctmp,q); continue;}

      if(p[proctmp].dbl_loop) { handle_dbl_loop(proctmp,q); continue;}

      if(p[proctmp].faulted == 0) {
         
         if( tmp_curr_page == 0 && p[proctmp].prev_page == 1) {
            pagein(proctmp,tmp_curr_page);
            p[proctmp].backward++;
            p[proctmp].faulted = 1;
            // printf("\nFOUND A FAULT!!\tProcess: %d\tBACKWARD\tFault Count: %d\tBackward: %d\n",proctmp,p[proctmp].faulted,p[proctmp].backward);
            continue;
         }         

         if((tmp_curr_page == 0 && p[proctmp].prev_page == 5) || (tmp_curr_page == 5 && p[proctmp].prev_page == 1)) {
            pagein(proctmp,tmp_curr_page);
            pageout(proctmp,p[proctmp].prev_page);
            p[proctmp].branch++;
            p[proctmp].faulted = 1;
            // printf("\nFOUND A FAULT!!\tProcess: %d\tBRANCH\tFault Count: %d\tBranch: %d\n",proctmp,p[proctmp].faulted,p[proctmp].branch);
            continue;
         }

         if(tmp_curr_page == 0 && p[proctmp].prev_page == 4) {
            pagein(proctmp,tmp_curr_page);
            pageout(proctmp,p[proctmp].prev_page);
            p[proctmp].looper++;
            p[proctmp].faulted = 1;
            // printf("\nFOUND A FAULT!!\tProcess: %d\tLOOPER\tFault Count: %d\tLoop: %d\n",proctmp,p[proctmp].faulted,p[proctmp].looper);
            continue;
         }

         if(tmp_curr_page == 4 && p[proctmp].prev_page == 6) {
            pagein(proctmp,tmp_curr_page);
            pageout(proctmp,p[proctmp].prev_page);
            p[proctmp].dbl_loop++;
            p[proctmp].faulted = 1;
            // printf("\nFOUND A FAULT!!\tProcess: %d\tDBL LOOPER\tFault Count: %d\tLoop: %d\n",proctmp,p[proctmp].faulted,p[proctmp].dbl_loop);
            continue;
         }
      }

      /*--Just in case--*/
      if(p[proctmp].faulted) { continue; }

      if(pagein(proctmp,tmp_curr_page)) {
         if(q[proctmp].pc%30 == 0) {
            pagein(proctmp,(tmp_curr_page+1)%MAXPROCPAGES);
         }
         if(tmp_curr_page > 0) {
            if(q[proctmp].pages[tmp_curr_page-1]) { 
               pageout(proctmp,(tmp_curr_page-1)%MAXPROCPAGES);
            }
            if(q[proctmp].pages[0]) { 
               pageout(proctmp,0); 
            }
         }
      }
      // pageOut_local(proctmp,q,tmp_curr_page);
   }
   pageOut_global(q);
   
   
   /* advance time for next pageit iteration */
   tick++;
} 
