// synchdisk.cc 
//	Routines to synchronously access the disk.  The physical disk 
//	is an asynchronous device (disk requests return immediately, and
//	an interrupt happens later on).  This is a layer on top of
//	the disk providing a synchronous interface (requests wait until
//	the request completes).
//
//	Use a semaphore to synchronize the interrupt handlers with the
//	pending requests.  And, because the physical disk can only
//	handle one operation at a time, use a lock to enforce mutual
//	exclusion.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synchdisk.h"
#include "kernel.h"

//----------------------------------------------------------------------
// SynchDisk::SynchDisk
// 	Initialize the synchronous interface to the physical disk, in turn
//	initializing the physical disk.
//
//----------------------------------------------------------------------

SynchDisk::SynchDisk()
{
    //semaphore = new Semaphore("synch disk", 0);	/* Liang DePeng - */
    //lock = new Lock("synch disk lock");	/* Liang DePeng - */
    //disk = new Disk(this);	/* Liang DePeng - */

	semaphore = GetSemaphoreF("synch disk", 0);	/* Liang DePeng + */
    lock = GetLockF("synch disk lock");	/* Liang DePeng + */
}

//----------------------------------------------------------------------
// SynchDisk::~SynchDisk
// 	De-allocate data structures needed for the synchronous disk
//	abstraction.
//----------------------------------------------------------------------

SynchDisk::~SynchDisk(){
    //delete disk;	/* Liang DePeng - */
    //delete lock;	/* Liang DePeng - */
    //delete semaphore;	/* Liang DePeng - */
	DeleteLockF(lock);	/* Liang DePeng + */
    DeleteSemaphoreF(semaphore);	/* Liang DePeng + */
}

//----------------------------------------------------------------------
// SynchDisk::ReadSector
// 	Read the contents of a disk sector into a buffer.  Return only
//	after the data has been read.
//
//	"sectorNumber" -- the disk sector to read
//	"data" -- the buffer to hold the contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::ReadSector(int sectorNumber, char* data)
{
    lock->Acquire();			// only one disk I/O at a time

    //disk->ReadRequest(sectorNumber, data);		/* Liang DePeng - */
	(* kernel->diskReadRequest)(sectorNumber, data);	/* Liang DePeng + */

    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::WriteSector
// 	Write the contents of a buffer into a disk sector.  Return only
//	after the data has been written.
//
//	"sectorNumber" -- the disk sector to be written
//	"data" -- the new contents of the disk sector
//----------------------------------------------------------------------

void
SynchDisk::WriteSector(int sectorNumber, char* data)
{
    lock->Acquire();			// only one disk I/O at a time

    //disk->WriteRequest(sectorNumber, data);	/* Liang DePeng - */
	(* kernel->diskWriteRequest)(sectorNumber, data);	/* Liang DePeng + */

    semaphore->P();			// wait for interrupt
    lock->Release();
}

//----------------------------------------------------------------------
// SynchDisk::CallBack
// 	Disk interrupt handler.  Wake up any thread waiting for the disk
//	request to finish.
//----------------------------------------------------------------------

void SynchDisk::CallBack(){ 
    semaphore->V();
}
