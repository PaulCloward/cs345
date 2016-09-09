
#ifndef __os345priority_queue_h__
#define __os345priority_queue_h__

#include <stdint.h>

//priority queue functions

typedef struct
{
	union
	{
			uint32_t element;
			struct
			{
				uint16_t priority; //priority    Prio.entry.priority;
				uint16_t tid;      //task Id    Prio.entry.tid
			} entry;
	}

} Prio;

int enQ(Prio pq[], int priority, int tid);//puts tid in queue....
int deQ(Prio pq[], int tid);//returns tid of highest priority task if tid = -1, else deletes entry with tid.
int queueSize(Prio pq[]);


#endif
