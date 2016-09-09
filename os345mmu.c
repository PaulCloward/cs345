// os345mmu.c - LC-3 Memory Management Unit	03/12/2015
//
//		03/12/2015	added PAGE_GET_SIZE to accessPage()
//
// **************************************************************************
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>
#include "os345.h"
#include "os345lc3.h"

// ***********************************************************************
// mmu variables

// LC-3 memory
unsigned short int memory[LC3_MAX_MEMORY];

// statistics
int memAccess;						// memory accesses
int memHits;						// memory hits
int memPageFaults;					// memory faults

int getFrame(int);
int getAvailableFrame(void);
extern TCB tcb[];					// task control block
extern int curTask;					// current task #

int RPageTable = 0x2400;
int mem3000 = 0x3000;

int getFrame(int notme)
{
	int frame;
	frame = getAvailableFrame();
	if (frame >=0) return frame;

	// run clock
	int UPTable, rpta, upta;
	int i, j, frameToChangeOut;
	bool emptyBool;
	i = RPageTable;
	while(1){//iterates root page table

		if(DEFINED(memory[i])){
			upta = FRAME(memory[i])<<6;


			if(DEFINED(memory[i])){
				emptyBool = TRUE;
			}


			UPTable = 0;
			while(UPTable < 64){

				int temp = upta+UPTable;
				if(DEFINED(memory[temp])){
					emptyBool = FALSE;
				}
				UPTable = UPTable + 2;
			}

			if(FRAME(memory[i]) != notme && emptyBool == TRUE && DEFINED(memory[i])){

				frameToChangeOut = FRAME(memory[i]);
				memory[i] = CLEAR_REF(CLEAR_DEFINED(memory[i]));
				RPageTable = i+2;
				if(!PAGED(memory[i + 1]) && emptyBool == TRUE){


					memory[i+1] = accessPage(0, FRAME(memory[i]), PAGE_NEW_WRITE);
				} else if(PAGED(memory[i+1])){
					if(DIRTY(memory[i])){

						accessPage(SWAPPAGE(memory[i+1]), FRAME(memory[i]), PAGE_OLD_WRITE);
					}
				}
				memory[i] = SET_DIRTY(memory[i]);
				memory[i+1] = SET_PAGED(memory[i+1]);

				return frameToChangeOut;
			}

			for(int k = 0; k < 64; k++){
				UPTable++;
			}



			UPTable = 0;
			while(UPTable < 64){

				j = UPTable + upta;
				if(DEFINED(memory[j])){
					if(REFERENCED(memory[j]) && DEFINED(memory[j])){
						UPTable+=2;
						memory[j] = CLEAR_REF(memory[j]);
						continue;
					}
					RPageTable = i;
					frameToChangeOut = FRAME(memory[j]);
					memory[j] = CLEAR_REF(CLEAR_DEFINED(memory[j]));

					if(PAGED(memory[j+1])){
						memory[j] = CLEAR_REF(CLEAR_DEFINED(memory[j]));
						if(DIRTY(memory[j])){
							accessPage(SWAPPAGE(memory[j+1]), FRAME(memory[j]), PAGE_OLD_WRITE);
						}
					}else{
						memory[j+1] = accessPage(0, FRAME(memory[j]), PAGE_NEW_WRITE);
					}
					memory[j+1] = SET_PAGED(memory[j+1]);
					memory[j] = SET_DIRTY(memory[j]);
					return frameToChangeOut;
				}
				UPTable+=2;
			}
		}

		i+=2;

		int j = i;

		setIncrement(i, j);
	}
	return frame;
}

void setIncrement(int i, int j){
	if(i >= mem3000 && j >= mem3000) {
		i = RPageTable;
	}
}



// **************************************************************************
// **************************************************************************
// LC3 Memory Management Unit
// Virtual Memory Process
// **************************************************************************
//           ___________________________________Frame defined
//          / __________________________________Dirty frame
//         / / _________________________________Referenced frame
//        / / / ________________________________Pinned in memory
//       / / / /     ___________________________
//      / / / /     /                 __________frame # (0-1023) (2^10)
//     / / / /     /                 / _________page defined
//    / / / /     /                 / /       __page # (0-4096) (2^12)
//   / / / /     /                 / /       /
//  / / / /     / 	             / /       /
// F D R P - - f f|f f f f f f f f|S - - - p p p p|p p p p p p p p

#define MMU_ENABLE	1

unsigned short int *getMemAdr(int va, int rwFlg)
{
	unsigned short int pa;
	int rpta, rpte1, rpte2;
	int upta, upte1, upte2;
	int rptFrame, uptFrame;

	// turn off virtual addressing for system RAM
	if (va < 0x3000) return &memory[va];
#if MMU_ENABLE
	rpta = tcb[curTask].RPT + RPTI(va);		// root page table address
	rpte1 = memory[rpta];					// FDRP__ffffffffff
	rpte2 = memory[rpta+1];					// S___pppppppppppp
	if (DEFINED(rpte1))	{memHits++;}					// rpte defined
		else	{
			memPageFaults++;
			rpte1 = SET_DEFINED(getFrame(-1));
			if(PAGED(rpte2)){
				accessPage(SWAPPAGE(rpte2), FRAME(rpte1), PAGE_READ);
			} else{
				rpte1 = SET_DIRTY(rpte1);
				rpte2 = 0;
				memset(&memory[FRAME(rpte1) << 6], 0, 64 * sizeof(memory[0]));
			}
		}					// rpte undefined
	memory[rpta] = rpte1 = SET_REF(SET_PINNED(rpte1));
	memory[rpta+1] = rpte2;			// set rpt frame access bit

	upta = (FRAME(rpte1)<<6) + UPTI(va);	// user page table address
	upte1 = memory[upta]; 					// FDRP__ffffffffff
	upte2 = memory[upta+1]; 				// S___pppppppppppp
	memAccess++;

	if (DEFINED(upte1))	{ memHits++;}					// upte defined
		else			{
			memPageFaults++;
			upte1 = SET_DEFINED(getFrame(FRAME(rpte1)));
			if(PAGED(upte2)){
				accessPage(SWAPPAGE(upte2), FRAME(upte1), PAGE_READ);

			} else {
				upte2 = 0;
				upte1 = SET_DIRTY(upte1);

			}
		 }

	memory[upta] = upte1 = SET_REF(upte1);
	memory[upta+1] = upte2;
	if(rwFlg != 0){
		memory[upta] = SET_DIRTY(upte1);
		memory[rpta] = SET_DIRTY(rpte1);
	}			// set upt frame access bit
	return &memory[(FRAME(upte1)<<6) + FRAMEOFFSET(va)];
#else
	return &memory[va];
#endif
} // end getMemAdr


// **************************************************************************
// **************************************************************************
// set frames available from sf to ef
//    flg = 0 -> clear all others
//        = 1 -> just add bits
//
void setFrameTableBits(int flg, int sf, int ef)
{	int i, data;
	int adr = LC3_FBT-1;             // index to frame bit table
	int fmask = 0x0001;              // bit mask

	// 1024 frames in LC-3 memory
	for (i=0; i<LC3_FRAMES; i++)
	{	if (fmask & 0x0001)
		{  fmask = 0x8000;
			adr++;
			data = (flg)?MEMWORD(adr):0;
		}
		else fmask = fmask >> 1;
		// allocate frame if in range
		if ( (i >= sf) && (i < ef)) data = data | fmask;
		MEMWORD(adr) = data;
	}
	return;
} // end setFrameTableBits


// **************************************************************************
// get frame from frame bit table (else return -1)
int getAvailableFrame()
{
	int i, data;
	int adr = LC3_FBT - 1;				// index to frame bit table
	int fmask = 0x0001;					// bit mask

	for (i=0; i<LC3_FRAMES; i++)		// look thru all frames
	{	if (fmask & 0x0001)
		{  fmask = 0x8000;				// move to next work
			adr++;
			data = MEMWORD(adr);
		}
		else fmask = fmask >> 1;		// next frame
		// deallocate frame and return frame #
		if (data & fmask)
		{  MEMWORD(adr) = data & ~fmask;
			return i;
		}
	}
	return -1;
} // end getAvailableFrame



// **************************************************************************
// read/write to swap space
int accessPage(int pnum, int frame, int rwnFlg)
{
	static int nextPage;						// swap page size
	static int pageReads;						// page reads
	static int pageWrites;						// page writes
	static unsigned short int swapMemory[LC3_MAX_SWAP_MEMORY];

	if ((nextPage >= LC3_MAX_PAGE) || (pnum >= LC3_MAX_PAGE))
	{
		printf("\nVirtual Memory Space Exceeded!  (%d)", LC3_MAX_PAGE);
		exit(-4);
	}
	switch(rwnFlg)
	{
		case PAGE_INIT:                    		// init paging
			memAccess = 0;						// memory accesses
			memHits = 0;						// memory hits
			memPageFaults = 0;					// memory faults
			nextPage = 0;						// disk swap space size
			pageReads = 0;						// disk page reads
			pageWrites = 0;						// disk page writes
			return 0;

		case PAGE_GET_SIZE:                    	// return swap size
			return nextPage;

		case PAGE_GET_READS:                   	// return swap reads
			return pageReads;

		case PAGE_GET_WRITES:                    // return swap writes
			return pageWrites;

		case PAGE_GET_ADR:                    	// return page address
			return (int)(&swapMemory[pnum<<6]);

		case PAGE_NEW_WRITE:                   // new write (Drops thru to write old)
			pnum = nextPage++;

		case PAGE_OLD_WRITE:                   // write
			//printf("\n    (%d) Write frame %d (memory[%04x]) to page %d", p.PID, frame, frame<<6, pnum);
			memcpy(&swapMemory[pnum<<6], &memory[frame<<6], 1<<7);
			pageWrites++;
			return pnum;

		case PAGE_READ:                    	// read
			//printf("\n    (%d) Read page %d into frame %d (memory[%04x])", p.PID, pnum, frame, frame<<6);
			memcpy(&memory[frame<<6], &swapMemory[pnum<<6], 1<<7);
			pageReads++;
			return pnum;

		case PAGE_FREE:                   // free page
			printf("\nPAGE_FREE not implemented");
			break;
   }
   return pnum;
} // end accessPage
