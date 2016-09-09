#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "os345.h"
#include "os345DeltaClock.h"

int i;

Delta* theDELTAClock = 0;
void deltaUpdate(int tics, Semaphore* sem){

  Delta* delta = (Delta*)malloc(sizeof(Delta));
  delta->tics = tics;
  delta->sem = sem;
  delta->nextPos = 0;

  Delta* iter = theDELTAClock;

  bool test = FALSE;

  if(theDELTAClock != 0 && theDELTAClock->tics > delta->tics){
        delta->nextPos = theDELTAClock;
        test = TRUE;
        theDELTAClock = delta;
  } else if(theDELTAClock == 0){
      theDELTAClock = delta;
      test = TRUE;
  }

  
  if(test == FALSE && theDELTAClock != 0){

    while(iter != 0){//Will keep looping until null
      delta->tics -= iter->tics;//minus tics from previous entry.
      if(iter->nextPos == 0 || (iter->nextPos->tics) > (delta->tics)){
        delta->nextPos = iter->nextPos;
        iter->nextPos = delta;
        iter = delta->nextPos;
        break;
      }
      iter = iter->nextPos;
    }
  }

  if(iter != 0){
  //  iter->tics -= delta->tics;
    iter->tics = subtractTics(iter->tics, delta->tics);
  }
}

int subtractTics(int numOne, int numTwo){
    return (numOne - numTwo);
}

void deltaTickTock(){

  if(theDELTAClock == 0){
    return;
  }
  Delta* iter = theDELTAClock;
  iter->tics--;

  while(1){
    if(iter->sem != 0){
      semSignal(iter->sem);
    }
    Delta* deltaFree = iter;
    iter = iter->nextPos;
    free(deltaFree);

    if(iter == 0){
      break;
    }else if(iter->tics>= 0){
      break;
    }
  }

  if(theDELTAClock == 0){
    iter = theDELTAClock;
  }

  theDELTAClock = iter;
}

void printDeltaClock(){

    Delta* iter = theDELTAClock;
    while(iter){
      printf("\n%d", iter->tics);
      iter = iter->nextPos;
    }
}
