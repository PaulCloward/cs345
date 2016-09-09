#include <stdio.h>
#include <stdlib.h>

#include "os345.h"
#include "os345priority_queue.h"
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

Prio readyQ[];

void initPQ(Prio pq[]) {
  pq[0].element = 0;
}

int enQ(Prio pq[], int priority, int tid) {

int size = pq[0].element;

  if(size >= MAX_TASKS){
    printf("Priority Queue has reached its limit");
    return -1;
  }

  for(int i = (size+1); i > 0; i--) {

    if(i != size + 1) {
      pq[i+1].element = pq[i].element;
    }

    if(i == 1 || (pq[i - 1].entry.priority < priority)) {

      pq[i].entry.priority = (uint16_t)priority;
      pq[i].entry.tid = (uint16_t)tid;
      pq[0].element++;
      return 1;
    }
  }
}

int deQ(Prio pq[], int tid) {
  int size = pq[0].element;
  if(size <= 0) {
    return -1;
  }

  int foundTid = -1;

  if(tid == -1){
    foundTid = pq[size].entry.tid;
    pq[size].element = 0;
    pq[0].element--;
    return foundTid;
  }

  int j;
  for(j = 1; j <= size; j++){
    if(pq[j].entry.tid == tid) {
      foundTid = pq[j].entry.tid;
      break;
    }
  }

  if(foundTid != -1){
    size--;
    for(j; j <= size; j++) {
      pq[j].element = pq[j+1].element;
    }
    pq[size+1].element = 0;
    pq[0].element = size;
  }
  return foundTid;
}


int queueSize(Prio pq[]){
  return pq[0].element;
}
//  int main(int argc, char* argv[])
// {
//   enQ(readyQ,1, 1);
//   printQueue();
//
//   enQ(readyQ,5,2);
//   printQueue();
//
//   enQ(readyQ,10,3);
//   printQueue();
//
//   enQ(readyQ,5,4);
//   printQueue();
//
//   enQ(readyQ,1,5);
//   printQueue();
//
//   enQ(readyQ,10,6);
//   printQueue();
//
// //  println("\n\nDeque's turn\n");
//   deQ(readyQ,4);
//   printQueue();
//
//   deQ(readyQ,-1);
//   printQueue();
//
//   deQ(readyQ,1);
//   printQueue();
//
//   deQ(readyQ,4);
//   printQueue();
//
//   deQ(readyQ,6);
//   printQueue();
//
//   deQ(readyQ,-1);
//   printQueue();
//
//   deQ(readyQ,5);
//   printQueue();
//
//   deQ(readyQ,-1);
//   printQueue();
// }

void printQueue(Prio pq[]) {
  int size = pq[0].element;
  for(int i = 1; i < (size + 1); i++){
    printf("element %d:   priority: %d\t tid: %d\n", i, pq[i].entry.priority, pq[i].entry.tid);
  }
  printf("\n");
}
