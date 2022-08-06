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

#include "simulator.h"

/*
 * 1.So I have a linear program should be fine with pg after pg
    --so keep 2 pages at min (curr and next-coming in & curr->prev out)
 * 2.Run all like linear till we get a fault
    --set a struct variable to distinguish it
 * 3.Keep track of min 3 vars for loops and branches
    --prev,*curr(pc),next
 * 4.Assuming avg distribution between the 5 program types
    --linear should be around 4
    --amt of pages min then is 8 = 4x2
 * 5.Asumming the same on the others
    --the others should need around 45 = 3x15
 * 6.On avg this would cause problems
    -- 8 + 45 = 53 only 50 available
 * 7.After so many ticks and analyzing those
    --can probably min the pages to 2 for non-linear
    --apply some probabilities to which in history it will go
    --check for backward branch--def need 3
 * 8.If statement looking for pc=0 after so many ticks
    --looking for new program start
 */

typedef _Bool bool;

typedef struct {
   int faulted;               // how many times faulted
   int faultpage[3][2];       // fault page, save previous & next (2)
   int currpage;
   int prev_page;             // previous page
   double prob_back;          // probabiility for backward branch
   double prob_branch;        // probabiility for inner branch
   long time[MAXPROCPAGES];   // timestamps per page
   // int per_proc_ct;           // how many pages per process
   bool looper;               // has it looped?
   bool dbl_loop;             // multiple faults and pages looped
   bool backward;             // is it backward?
   bool branch;               // is it inner branch?
} Predict;

/* Static var */
static Predict p[MAXPROCESSES];    //struct to hold all

void p_init() {

   /* Init complex static vars here */
   for(int k=0; k < MAXPROCESSES; k++){
      p[k].faulted = 0;
      for(int i=0; i<3; i++) {
         for(int j=0; j<2; j++) {
            p[k].faultpage[i][j] = -1;
         }
      }
      p[k].prob_back = 0.5;
      p[k].prob_branch = 0.4;
      p[k].currpage = 0;
      p[k].prev_page = 0;

      for(int i=0; i< MAXPROCPAGES; i++) {
         p[k].time[i] = 0;
      }

      p[k].looper = 0;
      p[k].dbl_loop = 0;
      p[k].backward = 0;
      p[k].branch = 0;
   }

   // p[0].faulted++;
   // // p[1].faulted;
   // printf("bool true %d\t false: %d\n.",p[0].faulted,p[1].faulted);

   /* TODO: Implement Predictive Paging */
   // fprintf(stderr, "pager-predict not yet implemented. Exiting...\n");
   // exit(EXIT_FAILURE);
}

void proc_reset(int k) {
   p[k].faulted = 0;
   for(int i=0; i<3; i++) {
      for(int j=0; j<2; j++) {
         p[k].faultpage[i][j] = -1;
      }
   }

   p[k].currpage = 0;
   p[k].prev_page = 0;
   for(int i=0; i< MAXPROCPAGES; i++) {
      p[k].time[i] = 0;
   }
   p[k].looper = 0;
   p[k].dbl_loop = 0;
   p[k].backward = 0;
   p[k].branch = 0;
}

void save_stamp(int proc, int page, int tick) {
   p[proc].time[page] = tick;
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

int pageOut_local(int proc, int tick, Pentry q[MAXPROCESSES]) {
   int remove;
   long tmptick = tick +1;
   int pg_ct = 0;
   for(int i=0; i<MAXPROCPAGES; i++) {
      if(!q[proc].pages[i]) continue;
      pg_ct++;

      if(p[proc].time[i] < tmptick) {
         tmptick = p[proc].time[i];
         remove = i;
      }
   }
   if(calc_pgs(q) < pg_ct){
      pageout(proc,remove);
      return 0;
   }
   return 1;
}

void pageOut_global(int tick, Pentry q[MAXPROCESSES]) {
   int remove;
   long tmptick = tick +1;
   int pg_ct = 0;
   for(int j=0; j<MAXPROCESSES; j++) {
      if(!q[j].active) continue;
      int currpage = q[j].pc / PAGESIZE;
      // if(q[j].pc == 1166 && tick%1000) printf("\nHello World!\tProcess: %d\n",j);

      for(int i=0; i<MAXPROCPAGES; i++) {
         if(!q[j].pages[i]) continue;
         pg_ct++;

         if(p[j].time[i] < tmptick) {
            tmptick = p[j].time[i];
            remove = i;
         }
      }

      if(calc_pgs(q) < pg_ct){
         if(q[j].pages[remove]){
            /*-If past current page we can try to steal one-*/
            if(currpage > (q[remove].pc/PAGESIZE)) {
               pageout(j,remove);
               break;
            }
         } 
      }
   } 
}

void age_page(int proc, int currpage) {
   for(int i=0; i<MAXPROCPAGES; i++) {
      if(p[proc].time[i] != currpage)
         p[proc].time[i]--;
   }

}

void pageit(Pentry q[MAXPROCESSES]) { 

   /* initialize static vars on first run */
   static ulong tick = 0;
   static int per_proc_ct = 0;
   static int tmp_curr_page;
   

   if(!tick) {
      p_init();
      per_proc_ct = calc_pgs(q);
      for(int j=0; j<MAXPROCESSES; j++) {
         for(int i=0; i<per_proc_ct; i++) {
            tmp_curr_page = q[j].pc/PAGESIZE +i;
            if(pagein(j,i)) p[j].time[i] = tick + i;
         }
      }
   }

   for(int proctmp=0; proctmp<MAXPROCESSES; proctmp++) {
      if(!q[proctmp].active) continue;
            
      tmp_curr_page = q[proctmp].pc/PAGESIZE;

      per_proc_ct = calc_pgs(q);

      // if(q[proctmp].pages[tmp_curr_page]) continue;

      /*-keeping track of pages-*/
      if((tmp_curr_page) && (tmp_curr_page != p[proctmp].prev_page)) {
         p[proctmp].prev_page = p[proctmp].currpage;
         p[proctmp].currpage = tmp_curr_page;
      }

      // if((tmp_curr_page == 0) && (tick > 500)) proc_reset(proctmp);

      save_stamp(proctmp,tmp_curr_page,tick);

      if(pagein(proctmp,tmp_curr_page)) {
         if(tick+1%101) {
            pageOut_local(proctmp,tick,q);//) pageOut_global(tick,q);
         }
         if(tmp_curr_page +1 < 11)  {
            if(!q[proctmp].pages[tmp_curr_page+1%MAXPROCPAGES]) {
               pagein(proctmp,((tmp_curr_page+1)%MAXPROCPAGES));
            } 
         }
         // continue;
      }

      // int active_pages = 0;
      // for(int j=0; j<MAXPROCPAGES; j++) {
      //    if(q[proctmp].pages[j]) { active_pages++;}
      // }

      // // age_page(proctmp,tmp_curr_page);

      // if(tick+1 % 100) {
      //    if(per_proc_ct < active_pages+1) {
      //       // if local doesn't work, look globally
      //       if(pageOut_local(proctmp,tick,q)) pageOut_global(tick,q);
      //    }
      // }
      


   }
   
   /* advance time for next pageit iteration */
   tick++;
} 
