#ifndef __os345delta_clock_h__
#define __os345delta_clock_h__

#include "os345.h"

typedef struct Delta
{
    Semaphore* s;
    int time;
    Delta* next;
} Delta;

#endif
