// os345p3.c - Jurassic Park
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include "os345.h"
#include "os345park.h"
#include "os345DeltaClock.h"

// ***********************************************************************
// project 3 variables

// Jurassic Park
extern JPARK myPark;
extern Semaphore* parkMutex;
extern Semaphore* fillSeat[NUM_CARS];
extern Semaphore* seatFilled[NUM_CARS];
extern Semaphore* rideOver[NUM_CARS];
extern Semaphore* tics10thsec;

Semaphore* theTickets;
Semaphore* passengerRide;
Semaphore* passengerJumpIntoRide;
Semaphore* ready2GoDriver;
Semaphore* noDriverYet;
Semaphore* dinosaur;
Semaphore* driverFinished[NUM_DRIVERS];
Semaphore* parkEntry;
Semaphore* sellDriverTick;
Semaphore* helloDriverWakeUp;
Semaphore* ticketSold;
Semaphore* buyTicketMutex;
Semaphore* rideFinished;
Semaphore* driverCar;
Semaphore* visitMuseum;
Semaphore* visitGiftShop;
Semaphore* buyGiftMutex;
Semaphore* driverSellGift;
Semaphore* giftSold;
Semaphore* finishingUp;

int currDriver;
int currCar;

// ***********************************************************************
// project 3 functions and tasks
void CL3_project3(int, char**);
void CL3_dc(int, char**);

int carTask(int argc, char* argv[]);
int passengerTask(int argc, char* argv[]);
int driverTask(int argc, char* argv[]);

// ***********************************************************************
// ***********************************************************************
// project3 command
int P3_project3(int argc, char* argv[])
{
	char buffer1[32];
	char buffer2[32];
	char* newArgv[2];

	sprintf(buffer1, "jurassicPark");
	newArgv[0] = buffer1;
	createTask(buffer1, jurassicTask, MED_PRIORITY, 1, newArgv);

	while (!parkMutex) SWAP;
	printf("\nStart Jurassic Park...");

	createSemMethod(buffer1);
	createTaskMethod(buffer1, buffer2, newArgv);

	return 0;
}

void createTaskMethod(char buffer1[], char buffer2[], char* newArgv[]){
	int i = 0;

	while(i < NUM_CARS){
		sprintf(buffer1, "CarTask%d", i);
		newArgv[0] = buffer1;
		sprintf(buffer2, "%d", i);
		newArgv[1] = buffer2;
		createTask(buffer1, carTask, MED_PRIORITY, 2, newArgv);
			i++;
	}

	int j = 0;
	int temp45 = 45;
	while(j < temp45){
		sprintf(buffer1, "PassengerTask%d", j);
		newArgv[0] = buffer1;
		sprintf(buffer2, "%d", j);
		newArgv[1] = buffer2;
		createTask(buffer1, passengerTask, MED_PRIORITY,	2, newArgv);
			j++;
	}

	int k = 0;
	while(k < NUM_DRIVERS){
			sprintf(buffer1, "DriverTask%d", k);
			newArgv[0] = buffer1;
			sprintf(buffer2, "%d", k);
			newArgv[1] = buffer2;
			createTask(buffer1, driverTask, MED_PRIORITY, 2, newArgv);
				k++;
	}
}

void createSemMethod(char buffer1[]){

	theTickets = createSemaphore("theTickets", COUNTING, MAX_TICKETS); SWAP;
	passengerRide = createSemaphore("passengerRide", BINARY, 0);
	sellDriverTick = createSemaphore("sellDriverTick", BINARY, 0);
	helloDriverWakeUp = createSemaphore("helloDriverWakeUp", BINARY, 0);
	passengerJumpIntoRide = createSemaphore("passengerJumpIntoRide", BINARY, 0);
	ready2GoDriver = createSemaphore("ready2GoDriver", BINARY, 0);
	driverCar = createSemaphore("driverCar", BINARY, 0);
	noDriverYet = createSemaphore("noDriverYet", BINARY, 0);
	parkEntry = createSemaphore("parkEntry ", COUNTING, 20);
	ticketSold = createSemaphore("ticketSold", BINARY, 0);
	buyTicketMutex = createSemaphore("buyTicketMutex", BINARY, 1);
	rideFinished = createSemaphore("rideFinished", BINARY, 0);
	dinosaur = createSemaphore("dinosaur", BINARY, 1);
	visitMuseum = createSemaphore("visitMuseum", COUNTING, MAX_IN_MUSEUM);
	visitGiftShop = createSemaphore("visitGiftShop", COUNTING, MAX_IN_GIFTSHOP);
	buyGiftMutex = createSemaphore("buyGiftMutex", BINARY, 1);
	driverSellGift = createSemaphore("driverSellGift", BINARY, 0);
	giftSold = createSemaphore("gift sold by driver", BINARY, 0);
	finishingUp = createSemaphore("finishing up", BINARY, 0);
	int i = 0;
	while(i < NUM_DRIVERS){
		sprintf(buffer1, "driver done driving %d", i);
		driverFinished[i] = createSemaphore(buffer1, BINARY, 0);
		i++;
	}
}

int carTask(int argc, char* argv[])
{
	int carIndex = atoi(argv[1]);
	int driverIndex;

	while(1){

		int i = 0;
		while(i < 3){
			SEM_WAIT(fillSeat[carIndex]);			SWAP;
			SEM_SIGNAL(passengerRide);			SWAP;//save passenger rideFinished[] semaphore
			SEM_WAIT(passengerJumpIntoRide); 	SWAP

			SEM_WAIT(parkMutex);	SWAP
			myPark.numInCarLine--;	SWAP
			myPark.numInCars++;	SWAP
			myPark.numTicketsAvailable++; 	SWAP
			SEM_SIGNAL(parkMutex);	SWAP
			SEM_SIGNAL(theTickets);	SWAP

			if(i > 1){

				currCar = carIndex+1; SWAP
				SEM_SIGNAL(noDriverYet);	SWAP
			 	SEM_SIGNAL(helloDriverWakeUp);	SWAP
				SEM_WAIT(ready2GoDriver);	SWAP
				driverIndex = currDriver; SWAP

			}
			SEM_SIGNAL(seatFilled[carIndex]);			SWAP;
			i++;
		}


		SEM_WAIT(rideOver[carIndex]);			SWAP;

		int j = 0;
		while(j < 3){
			SEM_WAIT(parkMutex);	SWAP
			myPark.numInCars--;	SWAP
			myPark.numInGiftLine++;	SWAP
			SEM_SIGNAL(parkMutex);	SWAP
			SEM_SIGNAL(rideFinished);	SWAP
			j++;
		}

		SEM_SIGNAL(driverFinished[driverIndex]);	SWAP
	}
	return 0;
}

int passengerTask(int argc, char* argv[]){
	char buffer1[32];
	int passID = atoi(argv[1]);

	sprintf(buffer1, "passengerDeltaClockSemaphore%d", passID);	SWAP
	Semaphore* passDeltaClockSemaphore = createSemaphore(buffer1, BINARY, 0);	SWAP
	deltaUpdate(rand()%100, passDeltaClockSemaphore);	SWAP
	SEM_WAIT(passDeltaClockSemaphore);	SWAP

	SEM_WAIT(parkMutex);	SWAP
	myPark.numOutsidePark++;	SWAP
	SEM_SIGNAL(parkMutex);	SWAP

	SEM_WAIT(parkEntry);	SWAP

	SEM_WAIT(parkMutex);	SWAP
	myPark.numOutsidePark--;	SWAP
	myPark.numInPark++;	SWAP
	myPark.numInTicketLine++;	SWAP
	SEM_SIGNAL(parkMutex);	SWAP

	passDeltaClockSemaphore->state = 0;	SWAP
	deltaUpdate(rand()%30, passDeltaClockSemaphore); 	SWAP
	SEM_WAIT(passDeltaClockSemaphore);	SWAP


	SEM_WAIT(buyTicketMutex); 	SWAP
	{

		SEM_SIGNAL(sellDriverTick);	SWAP
		SEM_SIGNAL(helloDriverWakeUp);	SWAP
		SEM_WAIT(ticketSold);	SWAP

		SEM_WAIT(parkMutex);	SWAP
		myPark.numTicketsAvailable--;	SWAP
		myPark.numInTicketLine--;	SWAP
		myPark.numInMuseumLine++;	SWAP
		SEM_SIGNAL(parkMutex);	SWAP

	}
	SEM_SIGNAL(buyTicketMutex); 	SWAP

	passDeltaClockSemaphore->state = 0;	SWAP
	deltaUpdate(rand()%30, passDeltaClockSemaphore); 	SWAP
	SEM_WAIT(passDeltaClockSemaphore);	SWAP


	SEM_WAIT(visitMuseum); 	SWAP

	SEM_WAIT(parkMutex);	SWAP
	myPark.numInMuseumLine--;	SWAP
	myPark.numInMuseum++;	SWAP
	SEM_SIGNAL(parkMutex);	SWAP

	passDeltaClockSemaphore->state = 0;	SWAP
	deltaUpdate(rand()%30, passDeltaClockSemaphore); 	SWAP
	SEM_WAIT(passDeltaClockSemaphore);	SWAP

	SEM_WAIT(parkMutex);	SWAP
	myPark.numInMuseum--;	SWAP
	myPark.numInCarLine++;	SWAP
	SEM_SIGNAL(parkMutex);	SWAP

	SEM_SIGNAL(visitMuseum);	SWAP

	passDeltaClockSemaphore->state = 0;	SWAP
	deltaUpdate(rand()%30, passDeltaClockSemaphore); 	SWAP
	SEM_WAIT(passDeltaClockSemaphore);	SWAP


	SEM_WAIT(passengerRide); 	SWAP
	SEM_SIGNAL(passengerJumpIntoRide);  	SWAP
	SEM_WAIT(rideFinished); 	SWAP

	passDeltaClockSemaphore->state = 0;	SWAP
	deltaUpdate(rand()%30, passDeltaClockSemaphore); 	SWAP
	SEM_WAIT(passDeltaClockSemaphore);	SWAP

	SEM_WAIT(visitGiftShop);
	SEM_WAIT(parkMutex);	SWAP
	myPark.numInGiftLine--;	SWAP
	myPark.numInGiftShop++;	SWAP
	SEM_SIGNAL(parkMutex);	SWAP

	passDeltaClockSemaphore->state = 0;	SWAP
	deltaUpdate(rand()%30, passDeltaClockSemaphore); 	SWAP
	SEM_WAIT(passDeltaClockSemaphore);	SWAP


	SEM_WAIT(parkMutex);	SWAP
	myPark.numInGiftShop--;	SWAP
	myPark.numInPark--;	SWAP
	myPark.numExitedPark++;	SWAP
	SEM_SIGNAL(parkMutex);	SWAP

	SEM_SIGNAL(visitGiftShop);	SWAP
	SEM_SIGNAL(parkEntry);	SWAP

	deleteSemaphore(passDeltaClockSemaphore);	SWAP
	return 0;
}

int driverTask(int argc, char* argv[])
{
	char buffer1[32];

	int myID = atoi(argv[1]);	SWAP

	while(1){

		bool test = FALSE;

		SEM_WAIT(helloDriverWakeUp);	SWAP
		if(SEM_TRYLOCK(noDriverYet)){

			SEM_WAIT(parkMutex);	SWAP
			myPark.drivers[myID] = currCar;	SWAP
			SEM_SIGNAL(parkMutex);	SWAP

			currDriver = myID; 	SWAP
			SEM_SIGNAL(ready2GoDriver); 	SWAP
			SEM_WAIT(driverFinished[myID]); 	SWAP
			test = TRUE;
		}
		if(SEM_TRYLOCK(sellDriverTick)){

			SEM_WAIT(parkMutex);	SWAP
			myPark.drivers[myID] = -1;	SWAP
			SEM_SIGNAL(parkMutex);	SWAP

			SEM_WAIT(theTickets);	SWAP
			SEM_SIGNAL(ticketSold);	SWAP
			test = TRUE;
		}
		if(SEM_TRYLOCK(driverSellGift)){
			SEM_WAIT(parkMutex);	SWAP
			myPark.drivers[myID] = -1;	SWAP
			SEM_SIGNAL(parkMutex);	SWAP
			printf("\n DRIVER %d is selling gift !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!", myID+1);
			SEM_SIGNAL(giftSold);	SWAP
			test = TRUE;
		}

		if(test == FALSE){
			break;
		}

		SEM_WAIT(parkMutex);	SWAP
		myPark.drivers[myID] = 0;	SWAP
		SEM_SIGNAL(parkMutex);	SWAP
	}
	return 0;
}

// ***********************************************************************
// ***********************************************************************
// delta clock command
int P3_dc(int argc, char* argv[])
{
	printf("\nDelta Clock");
	printDeltaClock();
	return 0;
} // end CL3_dc
