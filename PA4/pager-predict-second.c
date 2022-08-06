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
    -- so after next page in pc eject others
 * 7.After so many ticks and analyzing those
    --can probably min the pages to 2 for non-linear
    --check for backward branch--def need 3 (at that time)
 * 8.If statement looking for pc=0 after so many ticks
    --looking for new program start
 */

typedef _Bool bool;

typedef struct {
   int faulted;               // how many times faulted
   // int faultpage[1][2];       // fault page, save currentF & options (2)
   int currpage;
   int prev_page;             // previous page
   // int rand;
   // long curr_pc;
   // long prev_pc;
   // long time[MAXPROCPAGES];   // timestamps per page
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
      // for(int i=0; i<3; i++) {
      //    for(int j=0; j<2; j++) {
      //       p[k].faultpage[i][j] = 0;
      //    }
      // }

      p[k].currpage = 0;
      p[k].prev_page = 0;
      // p[k].rand = lrand48()%21 +10;

      // for(int i=0; i< MAXPROCPAGES; i++) {
      //    p[k].time[i] = 0;
      // }

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
   // for(int i=0; i<3; i++) {
   //    for(int j=0; j<2; j++) {
   //       p[k].faultpage[i][j] = 0;
   //    }
   // }

   p[k].currpage = 0;
   p[k].prev_page = 0;
   // for(int i=0; i< MAXPROCPAGES; i++) {
   //    p[k].time[i] = 0;
   // }
   p[k].looper = 0;
   p[k].dbl_loop = 0;
   p[k].backward = 0;
   p[k].branch = 0;
}

// void save_stamp(int proc, int page, int tick) {
//    p[proc].time[page] = tick;
// }
   
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

// int pageOut_local(int proc, Pentry q[MAXPROCESSES],int curr) {
//    // int remove;
//    // long tmptick = tick +1;
//    // int pg_ct = 0;
//    for(int i=0; i<MAXPROCPAGES; i++) {
//       if(!q[proc].pages[i]) continue;
//       if((i != curr) && (i != (curr+1)%MAXPROCPAGES)) {
//          pageout(proc,i);
//       }
//       // pg_ct++;
//    }
//    // int calc = calc_pgs(q);
//    // if(calc < pg_ct){
//    //    for(int i=0; i< pg_ct; i++) {
//    //       for(int j=0; j<MAXPROCPAGES; j++) {
//    //          if(curr != j) {
//    //             pageout(proc,j);
//    //             printf("\nMade it here.\t%d\n",j);
//    //          }
            
//    //       }
         
//    //    }
         
//    //    return 0;
//    // }
//    return 1;
// }

void pageOut_global(Pentry q[MAXPROCESSES]) {
   // int remove;
   // long tmptick = tick +1;
   // int pg_ct = 0;
   for(int j=0; j<MAXPROCESSES; j++) {
      if(!q[j].active) continue;
      int currpage = q[j].pc / PAGESIZE;
      // if(q[j].pc == 1166 && tick%1000) printf("\nHello World!\tProcess: %d\n",j);

      for(int i=0; i<MAXPROCPAGES; i++) {
         if(!q[j].pages[i]) continue;
         if((i != currpage) && (i != (currpage+1)%MAXPROCPAGES)) {
            if(!(p[j].faulted)) {
               pageout(j,i);
               continue;
            }
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
   else { if(!q[proc].pages[currpage+1] && currpage+1 < 4) { pagein(proc,currpage+1); } }

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
      // if(q[proc].pages[3]) { pageout(proc,3); }
      
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
      if(q[proc].pc > 1400) {
         if(q[proc].pages[4]) { pageout(proc,4); }
      }
      if(!q[proc].pages[0]) { pagein(proc,0); }
      
   }
   else if(currpage == 1) {
      if(q[proc].pc > 350) {
         if(!q[proc].pages[5]) { pagein(proc,5); }
         // if(drand48() > 0.6){
         if(!q[proc].pages[2]) { pagein(proc,2); }
         // }
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
   // static int check_page;
   // static int check_pc;
   
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
      // raise(SIGINT);
   // }

   if(!initialized) {
      p_init();
      per_proc_ct = calc_pgs(q);
      for(int j=0; j<MAXPROCESSES; j++) {
         for(int i=0; i<per_proc_ct; i++) {
            pagein(j,i);//) p[j].time[i] = tick + i;
         }
      }
      // printf("\nONCE?\n");
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
      // check_page = p[proctmp].prev_page;
      // check_pc = q[proctmp].pc;

      /*-keeping track of pages-*/
      if(tmp_curr_page != p[proctmp].prev_page) {
         p[proctmp].prev_page = p[proctmp].currpage;
         p[proctmp].currpage = tmp_curr_page;
      }

      // if(check_pc != p[proctmp].prev_pc) {
      //    p[proctmp].prev_pc = p[proctmp].curr_pc;
      //    p[proctmp].curr_pc = check_pc;
      // }

      // if(proctmp == 19) { p[proctmp].faulted = 1; }

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

      if(tmp_curr_page == 6) {
         // if(q[proctmp].pc > 1610) {
         //    if(q[proctmp].pages[5]) { pageout(proctmp,5); }
         // }
         if(!q[proctmp].pages[0]) { pagein(proctmp,0); }
      }

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
         // else { if(q[proctmp].pages[tmp_curr_page] && q[proctmp].pc > 260) { pageout(proctmp,p[proctmp].prev_page); } }
      }
      // pageOut_local(proctmp,q,tmp_curr_page);
   }
   pageOut_global(q);
   
   
   /* advance time for next pageit iteration */
   tick++;
} 
