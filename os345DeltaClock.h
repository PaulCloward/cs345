#ifndef __os345delta_clock_h__
#define __os345delta_clock_h__

#include "os345.h"

void deltaUpdate(int, Semaphore*);
void deltaTickTock();

typedef struct Delta {
    int tics;
    Semaphore* sem;
    struct Delta* nextPos;
} Delta;

typedef struct testDelta {
  struct Delta* delta;
} testDelta;


#endif
