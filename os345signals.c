// os345signal.c - signals
// ***********************************************************************
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// **                                                                   **
// ** The code given here is the basis for the CS345 projects.          **
// ** It comes "as is" and "unwarranted."  As such, when you use part   **
// ** or all of the code, it becomes "yours" and you are responsible to **
// ** understand any algorithm or method presented.  Likewise, any      **
// ** errors or problems become your responsibility to fix.             **
// **                                                                   **
// ** NOTES:                                                            **
// ** -Comments beginning with "// ??" may require some implementation. **
// ** -Tab stops are set at every 3 spaces.                             **
// ** -The function API's in "OS345.h" should not be altered.           **
// **                                                                   **
// **   DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER ** DISCLAMER   **
// ***********************************************************************
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>
#include "os345.h"
#include "os345signals.h"

extern TCB tcb[];							// task control block
extern int curTask;							// current task #

// ***********************************************************************
// ***********************************************************************
//	Call all pending task signal handlers
//
//	return 1 if task is NOT to be scheduled.
//
int signals(void)
{
	if (tcb[curTask].signal)
	{
		if (tcb[curTask].signal & mySIGCONT)
		{
			tcb[curTask].signal &= ~mySIGCONT;
			(*tcb[curTask].SigCONTHandler)();
		}
		if (tcb[curTask].signal & mySIGTSTP)
		{
			tcb[curTask].signal &= ~mySIGTSTP;
			(*tcb[curTask].SigTSTPHandler)();
		}
		if (tcb[curTask].signal & mySIGINT)
		{
			tcb[curTask].signal &= ~mySIGINT;
			(*tcb[curTask].SigINTHandler)();
		}
		if (tcb[curTask].signal & mySIGTERM)
		{
			tcb[curTask].signal &= ~mySIGTERM;
			(*tcb[curTask].SigTERMHandler)();
		}
	}
	return 0;
}


// **********************************************************************
// **********************************************************************
//	Register task signal handlers
//
int sigAction(void (*sigHandler)(void), int sig)
{
	switch (sig)
	{
		case mySIGTSTP:
		{
			tcb[curTask].SigTSTPHandler = sigHandler;		// mySIGINT handler
			return 0;
		}
		case mySIGCONT:
		{
			tcb[curTask].SigCONTHandler = sigHandler;		// mySIGINT handler
			return 0;
		}
		case mySIGINT:
		{
			tcb[curTask].SigINTHandler = sigHandler;		// mySIGINT handler
			return 0;
		}
		case mySIGTERM:
		{
			tcb[curTask].SigTERMHandler = sigHandler;		// mySIGINT handler
			return 0;
		}
	}
	return 1;
}


// **********************************************************************
//	sigSignal - send signal to task(s)
//
//	taskId = task (-1 = all tasks)
//	sig = signal
//
int sigSignal(int taskId, int sig)
{
	// check for task
	if ((taskId >= 0) && tcb[taskId].name)
	{
		tcb[taskId].signal |= sig;
		return 0;
	}
	else if (taskId == -1)
	{
		for (taskId=0; taskId<MAX_TASKS; taskId++)
		{
			sigSignal(taskId, sig);
		}
		return 0;
	}
	// error
	return 1;
}


// **********************************************************************
// **********************************************************************
//	Default signal handlers
//
// void defaultSigINTHandler(void)			// task mySIGINT handler
// {
// 	printf("\ndefaultSigINTHandler");
// 	return;
// }

void defaultSigCONTHandler()
{
	printf("\ndefaulySigCONTHandler");
	return;
}
void defaultSigINTHandler()
{
	printf("Hellomynameisinigomontoyayoukilledmyfatherpreparetodie");
	return;
}
void defaultSigTERMHandler()
{
	printf("\ndefaulySigTERMHandler");
	return;
}

void defaultSigTSTPHandler()
{
	printf("\ndefaulySigTSTPHandler");
	return;
}

void defaultSigSTOPHandler()
{
	printf("\ndefaulySigSTOPHandler");
	return;
}


void createTaskSigHandlers(int tid)
{
	tcb[tid].signal = 0;
	if (tid)
	{
		// inherit parent signal handlers
		tcb[tid].SigCONTHandler = tcb[curTask].SigCONTHandler;
		tcb[tid].SigINTHandler = tcb[curTask].SigINTHandler;			// mySIGINT handler
		tcb[tid].SigTERMHandler = tcb[curTask].SigTERMHandler;
		tcb[tid].SigTSTPHandler = tcb[curTask].SigTSTPHandler;
	//	tcb[tid].SigSTOPHandler = tcb[curTask].SigSTOPHandler;
	}
	else
	{
		// otherwise use defaults
		tcb[tid].SigCONTHandler = defaultSigCONTHandler;
		tcb[tid].SigINTHandler = defaultSigINTHandler;			// task mySIGINT handler
		tcb[tid].SigTERMHandler = defaultSigTERMHandler;
		tcb[tid].SigTSTPHandler = defaultSigTSTPHandler;
	//	tcb[tid].SigSTOPHandler = defaultSigSTOPHandler;
	}
	}
