// os345fat.c - file management system
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
//		11/19/2011	moved getNextDirEntry to P6
//
// ***********************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <assert.h>
#include "os345.h"
#include "os345fat.h"

// ***********************************************************************
// ***********************************************************************
//	functions to implement in Project 6
//
int fmsCloseFile(int);
int fmsDefineFile(char*, int);
int fmsDeleteFile(char*);
int fmsOpenFile(char*, int);
int fmsReadFile(int, char*, int);
int fmsSeekFile(int, int);
int fmsWriteFile(int, char*, int);

// ***********************************************************************
// ***********************************************************************
//	Support functions available in os345p6.c
//
extern int fmsGetDirEntry(char* fileName, DirEntry* dirEntry);
extern int fmsGetNextDirEntry(int *dirNum, char* mask, DirEntry* dirEntry, int dir);

extern int fmsMount(char* fileName, void* ramDisk);

extern void setFatEntry(int FATindex, unsigned short FAT12ClusEntryVal, unsigned char* FAT);
extern unsigned short getFatEntry(int FATindex, unsigned char* FATtable);

extern int fmsMask(char* mask, char* name, char* ext);
extern void setDirTimeDate(DirEntry* dir);
extern int isValidFileName(char* fileName);
extern void printDirectoryEntry(DirEntry*);
extern void fmsError(int);

extern int fmsReadSector(void* buffer, int sectorNumber);
extern int fmsWriteSector(void* buffer, int sectorNumber);

// ***********************************************************************
// ***********************************************************************
// fms variables
//
// RAM disk
unsigned char RAMDisk[SECTORS_PER_DISK * BYTES_PER_SECTOR];

// File Allocation Tables (FAT1 & FAT2)
unsigned char FAT1[NUM_FAT_SECTORS * BYTES_PER_SECTOR];
unsigned char FAT2[NUM_FAT_SECTORS * BYTES_PER_SECTOR];

char dirPath[128];							// current directory path
FDEntry OFTable[NFILES];					// open file table

extern bool diskMounted;					// disk has been mounted
extern TCB tcb[];							// task control block
extern int curTask;							// current task #


// ***********************************************************************
// ***********************************************************************
// This function closes the open file specified by fileDescriptor.
// The fileDescriptor was returned by fmsOpenFile and is an index into the open file table.
//	Return 0 for success, otherwise, return the error number.
//
int fmsCloseFile(int fileDescriptor)
{
	if(OFTable[fileDescriptor].name[0] == 0 ){
		return ERR63;
	}
	OFTable[fileDescriptor].startCluster = 0;
	OFTable[fileDescriptor].name[0] = 0;
	return 0;
} // end fmsCloseFile



// ***********************************************************************
// ***********************************************************************
// If attribute=DIRECTORY, this function creates a new directory
// file directoryName in the current directory.
// The directory entries "." and ".." are also defined.
// It is an error to try and create a directory that already exists.
//
// else, this function creates a new file fileName in the current directory.
// It is an error to try and create a file that already exists.
// The start cluster field should be initialized to cluster 0.  In FAT-12,
// files of size 0 should point to cluster 0 (otherwise chkdsk should report an error).
// Remember to change the start cluster field from 0 to a free cluster when writing to the
// file.
//
// Return 0 for success, otherwise, return the error number.
//
int fmsDefineFile(char* fileName, int attribute)
{
		return ERR61;
} // end fmsDefineFile



// ***********************************************************************
// ***********************************************************************
// This function deletes the file fileName from the current director.
// The file name should be marked with an "E5" as the first character and the chained
// clusters in FAT 1 reallocated (cleared to 0).
// Return 0 for success; otherwise, return the error number.
//
int fmsDeleteFile(char* fileName)
{
	return ERR61;
} // end fmsDeleteFile


// ***********************************************************************
// ***********************************************************************
// This function opens the file fileName for access as specified by rwMode.
// It is an error to try to open a file that does not exist.
// The open mode rwMode is defined as follows:
//    0 - Read access only.
//       The file pointer is initialized to the beginning of the file.
//       Writing to this file is not allowed.
//    1 - Write access only.
//       The file pointer is initialized to the beginning of the file.
//       Reading from this file is not allowed.
//    2 - Append access.
//       The file pointer is moved to the end of the file.
//       Reading from thisis initialized to the beginning of the file.
//       Both read and writing to the file is allowed.
// A maximum of 32 files may be open at any one time.
// If successful, return a file descriptor that is used in calling subsequent file
// handling functions; otherwise, return the error number.
//
int fmsOpenFile(char* fileName, int rwMode)
{

	FDEntry* fdEntry;
	int error, theTempCluster,fileDescriptor;
	fileDescriptor = -1;
	DirEntry* dirEntry = malloc(sizeof(DirEntry));

	if((error = fmsGetDirEntry(fileName, dirEntry))) return error;

	int i = 0;
	while(i < NFILES){
		if(OFTable[i].name[0] == 0 && fileDescriptor == -1){
			fileDescriptor = i;
		}
		i++;
	}

	if(fileDescriptor == -1){
		 return ERR70;
	 }

	fdEntry = &OFTable[fileDescriptor];
	memcpy(fdEntry->extension, dirEntry->extension,1);
	memcpy(fdEntry->name, dirEntry->name, 8);

	fdEntry->currentCluster = 0;
	fdEntry->directoryCluster = CDIR;
	fdEntry->startCluster = dirEntry->startCluster;
	fdEntry->attributes = dirEntry->attributes;
	fdEntry->fileSize = dirEntry->fileSize;
	fdEntry->flags = 0;
	fdEntry->fileIndex = 0;
	fdEntry->pid = curTask;
	fdEntry->mode = rwMode;

	if(rwMode == OPEN_APPEND && fileDescriptor != -1){

		int theTempCluster, presentCluster;
		theTempCluster = presentCluster = fdEntry->startCluster;
		fdEntry->fileIndex = fdEntry->fileSize;
		while(theTempCluster != FAT_EOC){

			theTempCluster = getFatEntry(theTempCluster, FAT1);
			if(theTempCluster != FAT_EOC){

				presentCluster = theTempCluster;
				theTempCluster = getFatEntry(theTempCluster, FAT1);
			}
		}
	}
	free(dirEntry);
	fdEntry->currentCluster = 0;
	fdEntry->flags = 0;
	fdEntry->fileIndex = 0;
	return fileDescriptor;
}



// ***********************************************************************
// ***********************************************************************
// This function reads nBytes bytes from the open file specified by fileDescriptor into
// memory pointed to by buffer.
// The fileDescriptor was returned by fmsOpenFile and is an index into the open file table.
// After each read, the file pointer is advanced.
// Return the number of bytes successfully read (if > 0) or return an error number.
// (If you are already at the end of the file, return EOF error.  ie. you should never
// return a 0.)
//
int fmsReadFile(int fileDescriptor, char* buffer, int nBytes)
{
	int error, nextCluster;
	FDEntry* fdEntry;
	int numBytesRead = 0;
	unsigned int bytesLeft, bufferIndex;
	fdEntry = &OFTable[fileDescriptor];
	if (fdEntry->name[0] == 0) return ERR63;//file is not open
	if((fdEntry->mode == 1) || (fdEntry->mode == 2)) return ERR85; //1-OPEN_WRITE ; 2- OPEN_APPEND
	while(nBytes > 0){//Decrement through the given number of bytes

			if(fdEntry->fileSize == fdEntry->fileIndex)//end of file
							return (numBytesRead ? numBytesRead : ERR66);//return numBytesRead if greater than zero or return end of file error
			bufferIndex = fdEntry->fileIndex % BYTES_PER_SECTOR;//bufferIndex within a sector
			if((bufferIndex == 0) && (fdEntry->fileIndex || !fdEntry->currentCluster))//if at the beginning of sector and (file index is greater than zero or current cluster == 0)
			{
				if(fdEntry->currentCluster== 0){
					if(fdEntry->startCluster == 0) return ERR66; //endOfFile
					nextCluster = fdEntry->startCluster;//set next cluster to be non-zero
					fdEntry->fileIndex = 0;
				}
				else{
					nextCluster = getFatEntry(fdEntry->currentCluster, FAT1);//get the next fat entry
					if(nextCluster == FAT_EOC) return numBytesRead;//end of cluster
				}
				if(fdEntry->flags & BUFFER_ALTERED){

					if((error = fmsWriteSector(fdEntry->buffer,
															C_2_S(fdEntry->currentCluster)))) return error;
					fdEntry->flags &= ~BUFFER_ALTERED;
				}
				if(error = fmsReadSector(fdEntry->buffer, C_2_S(nextCluster))) return error;

				fdEntry->currentCluster = nextCluster;
			}
		bytesLeft = BYTES_PER_SECTOR - bufferIndex;
		if(bytesLeft > nBytes) bytesLeft = nBytes;
		if(bytesLeft > (fdEntry->fileSize - fdEntry->fileIndex)){
			bytesLeft = fdEntry->fileSize - fdEntry->fileIndex;
		}
		memcpy(buffer, &fdEntry->buffer[bufferIndex], bytesLeft);
		fdEntry->fileIndex += bytesLeft;
		numBytesRead += bytesLeft;
		buffer+=bytesLeft;
		nBytes -=bytesLeft;
	}
	return numBytesRead;
} // end fmsReadFile



// ***********************************************************************
// ***********************************************************************
// This function changes the current file pointer of the open file specified by
// fileDescriptor to the new file position specified by index.
// The fileDescriptor was returned by fmsOpenFile and is an index into the open file table.
// The file position may not be positioned beyond the end of the file.
// Return the new position in the file if successful; otherwise, return the error number.
//
int fmsSeekFile(int fileDescriptor, int index)
{

	return 0;
} // end fmsSeekFile



// ***********************************************************************
// ***********************************************************************
// This function writes nBytes bytes to the open file specified by fileDescriptor from
// memory pointed to by buffer.
// The fileDescriptor was returned by fmsOpenFile and is an index into the open file table.
// Writing is always "overwriting" not "inserting" in the file and always writes forward
// from the current file pointer position.
// Return the number of bytes successfully written; otherwise, return the error number.
//
int fmsWriteFile(int fileDescriptor, char* buffer, int nBytes)
{
	return ERR63;
} // end fmsWriteFile
