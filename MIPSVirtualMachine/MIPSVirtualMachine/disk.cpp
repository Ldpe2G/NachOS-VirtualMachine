// disk.cc 
//	Routines to simulate a physical disk device; reading and writing
//	to the disk is simulated as reading and writing to a UNIX file.
//	See disk.h for details about the behavior of disks (and
//	therefore about the behavior of this simulation).
//
//	Disk operations are asynchronous, so we have to invoke an interrupt
//	handler when the simulated operation completes.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "disk.h"
//#include "debug.h"
//#include "sysdep.h"
#include "main.h"
using namespace std;
// We put a magic number at the front of the UNIX file representing the
// disk, to make it less likely we will accidentally treat a useful file 
// as a disk (which would probably trash the file's contents).

const int MagicNumber = 0x456789ab;
const int MagicSize = sizeof(int);
const int DiskSize = (MagicSize + (NumSectors * SectorSize));


//----------------------------------------------------------------------
// Disk::Disk()
// 	Initialize a simulated disk.  Open the UNIX file (creating it
//	if it doesn't exist), and check the magic number to make sure it's 
// 	ok to treat it as Nachos disk storage.
//
//	"toCall" -- object to call when disk read/write request completes
//----------------------------------------------------------------------

Disk::Disk(CallBackObj *toCall)
{
    int magicNum;
    int tmp = 0;

    DEBUG(dbgDisk, "Initializing the disk.");
    callWhenDone = toCall;
    lastSector = 0;
    bufferInit = 0;
    
    sprintf(diskname,"DISK_%d",hostName);
	fileno = (* fp_OpenForReadWrite)(diskname, FALSE);	/* Liang DePeng + */
    //fileno = OpenForReadWrite(diskname, FALSE);	/* Liang DePeng - */
    if (fileno >= 0) {		 	// file exists, check magic number 
		(* fp_Read)(fileno, (char *) &magicNum, MagicSize);	/* Liang DePeng + */
		//Read(fileno, (char *) &magicNum, MagicSize);	/* Liang DePeng - */
		ASSERT(magicNum == MagicNumber);
    } else {				// file doesn't exist, create it
		fileno = (* fp_OpenForWrite)(diskname);	/* Liang DePeng + */
        //fileno = OpenForWrite(diskname);	/* Liang DePeng - */
		magicNum = MagicNumber;  
		(* fp_WriteFkFile)(fileno, (char *) &magicNum, MagicSize); // write magic number /* Liang DePeng + */
		//WriteFkFile(fileno, (char *) &magicNum, MagicSize); // write magic number /* Liang DePeng - */

		// need to write at end of file, so that reads will not return EOF
		(* fp_Lseek)(fileno, DiskSize - sizeof(int), 0);	/* Liang DePeng + */
        //Lseek(fileno, DiskSize - sizeof(int), 0);		/* Liang DePeng - */
		(* fp_WriteFkFile)(fileno, (char *)&tmp, sizeof(int));		/* Liang DePeng + */  
		//WriteFkFile(fileno, (char *)&tmp, sizeof(int));		/* Liang DePeng - */  
    }
    active = FALSE;
}

//----------------------------------------------------------------------
// Disk::~Disk()
// 	Clean up disk simulation, by closing the UNIX file representing the
//	disk.
//----------------------------------------------------------------------

Disk::~Disk()
{
	(* fp_Close)(fileno);	/* Liang DePeng + */
    //Close(fileno);	/* Liang DePeng - */
}

//----------------------------------------------------------------------
// Disk::PrintSector()
// 	Dump the data in a disk read/write request, for debugging.
//----------------------------------------------------------------------

static void
PrintSector (bool writing, int sector, char *data)
{
    int *p = (int *) data;

    if (writing)
        cout << "Writing sector: " << sector << "\n"; 
    else
        cout << "Reading sector: " << sector << "\n"; 
    for (unsigned int i = 0; i < (SectorSize/sizeof(int)); i++) {
		cout << p[i] << " ";
    }
    cout << "\n"; 
}

//----------------------------------------------------------------------
// Disk::ReadRequest/WriteRequest
// 	Simulate a request to read/write a single disk sector
//	   Do the read/write immediately to the UNIX file
//	   Set up an interrupt handler to be called later,
//	      that will notify the caller when the simulator says
//	      the operation has completed.
//
//	Note that a disk only allows an entire sector to be read/written,
//	not part of a sector.
//
//	"sectorNumber" -- the disk sector to read/write
//	"data" -- the bytes to be written, the buffer to hold the incoming bytes
//----------------------------------------------------------------------

void
Disk::ReadRequest(int sectorNumber, char* data)
{
    int ticks = ComputeLatency(sectorNumber, FALSE);

    ASSERT(!active);				// only one request at a time
    ASSERT((sectorNumber >= 0) && (sectorNumber < NumSectors));
    
    DEBUG(dbgDisk, "Reading from sector " << sectorNumber);

	(* fp_Lseek)(fileno, SectorSize * sectorNumber + MagicSize, 0);	/* Liang DePeng + */
    (* fp_Read)(fileno, data, SectorSize);								/* Liang DePeng + */
    //Lseek(fileno, SectorSize * sectorNumber + MagicSize, 0);	/* Liang DePeng - */
    //Read(fileno, data, SectorSize);	/* Liang DePeng - */
    if (debug->IsEnabled('d'))
		PrintSector(FALSE, sectorNumber, data);
    
    active = TRUE;
    UpdateLast(sectorNumber);
    //kernel->stats->numDiskReads++;						/* Liang DePeng - */                
    //kernel->interrupt->Schedule(this, ticks, DiskInt);	/* Liang DePeng - */
	stats->numDiskReads++;									/* Liang DePeng + */  
	interrupt->Schedule(this, ticks, DiskInt);				/* Liang DePeng + */
}

void Disk::WriteRequest(int sectorNumber, char* data){

    int ticks = ComputeLatency(sectorNumber, TRUE);

    ASSERT(!active);
    ASSERT((sectorNumber >= 0) && (sectorNumber < NumSectors));
    
    DEBUG(dbgDisk, "Writing to sector " << sectorNumber);

	(* fp_Lseek)(fileno, SectorSize * sectorNumber + MagicSize, 0);	/* Liang DePeng + */ 
    (* fp_WriteFkFile)(fileno, data, SectorSize);						/* Liang DePeng + */

    //Lseek(fileno, SectorSize * sectorNumber + MagicSize, 0);	/* Liang DePeng - */ 
    //WriteFkFile(fileno, data, SectorSize);					/* Liang DePeng - */ 
    if (debug->IsEnabled('d'))
		PrintSector(TRUE, sectorNumber, data);
    
    active = TRUE;
    UpdateLast(sectorNumber);
    //kernel->stats->numDiskWrites++;						/* Liang DePeng - */ 
    //kernel->interrupt->Schedule(this, ticks, DiskInt);	/* Liang DePeng - */ 
	stats->numDiskWrites++;									/* Liang DePeng + */  
	interrupt->Schedule(this, ticks, DiskInt);				/* Liang DePeng + */
}

//----------------------------------------------------------------------
// Disk::CallBack()
// 	Called by the machine simulation when the disk interrupt occurs.
//----------------------------------------------------------------------

void
Disk::CallBack ()
{ 
    active = FALSE;
    callWhenDone->CallBack();
}

//----------------------------------------------------------------------
// Disk::TimeToSeek()
//	Returns how long it will take to position the disk head over the correct
//	track on the disk.  Since when we finish seeking, we are likely
//	to be in the middle of a sector that is rotating past the head,
//	we also return how long until the head is at the next sector boundary.
//	
//   	Disk seeks at one track per SeekTime ticks (cf. stats.h)
//   	and rotates at one sector per RotationTime ticks
//----------------------------------------------------------------------

int
Disk::TimeToSeek(int newSector, int *rotation) 
{
    int newTrack = newSector / SectorsPerTrack;
    int oldTrack = lastSector / SectorsPerTrack;
    int seek = abs(newTrack - oldTrack) * SeekTime;
				// how long will seek take?
    //int over = (kernel->stats->totalTicks + seek) % RotationTime;	/* Liang DePeng - */
	int over = (stats->totalTicks + seek) % RotationTime;			/* Liang DePeng + */
				// will we be in the middle of a sector when
				// we finish the seek?

    *rotation = 0;
    if (over > 0)	 	// if so, need to round up to next full sector
		*rotation = RotationTime - over;
    return seek;
}

//----------------------------------------------------------------------
// Disk::ModuloDiff()
// 	Return number of sectors of rotational delay between target sector
//	"to" and current sector position "from"
//----------------------------------------------------------------------

int 
Disk::ModuloDiff(int to, int from)
{
    int toOffset = to % SectorsPerTrack;
    int fromOffset = from % SectorsPerTrack;

    return ((toOffset - fromOffset) + SectorsPerTrack) % SectorsPerTrack;
}

//----------------------------------------------------------------------
// Disk::ComputeLatency()
// 	Return how long will it take to read/write a disk sector, from
//	the current position of the disk head.
//
//   	Latency = seek time + rotational latency + transfer time
//   	Disk seeks at one track per SeekTime ticks (cf. stats.h)
//   	and rotates at one sector per RotationTime ticks
//
//   	To find the rotational latency, we first must figure out where the 
//   	disk head will be after the seek (if any).  We then figure out
//   	how long it will take to rotate completely past newSector after 
//	that point.
//
//   	The disk also has a "track buffer"; the disk continuously reads
//   	the contents of the current disk track into the buffer.  This allows 
//   	read requests to the current track to be satisfied more quickly.
//   	The contents of the track buffer are discarded after every seek to 
//   	a new track.
//----------------------------------------------------------------------

int
Disk::ComputeLatency(int newSector, bool writing)
{
    int rotation;
    int seek = TimeToSeek(newSector, &rotation);
    //int timeAfter = kernel->stats->totalTicks + seek + rotation; /* Liang DePeng - */
	int timeAfter = stats->totalTicks + seek + rotation;           /* Liang DePeng + */

#ifndef NOTRACKBUF	// turn this on if you don't want the track buffer stuff
    // check if track buffer applies
    if ((writing == FALSE) && (seek == 0) 
		&& (((timeAfter - bufferInit) / RotationTime) 
	     		> ModuloDiff(newSector, bufferInit / RotationTime))) {
        DEBUG(dbgDisk, "Request latency = " << RotationTime);
		return RotationTime; // time to transfer sector from the track buffer
    }
#endif

    rotation += ModuloDiff(newSector, timeAfter / RotationTime) * RotationTime;

    DEBUG(dbgDisk, "Request latency = " << (seek + rotation + RotationTime));
    return(seek + rotation + RotationTime);
}

//----------------------------------------------------------------------
// Disk::UpdateLast
//   	Keep track of the most recently requested sector.  So we can know
//	what is in the track buffer.
//----------------------------------------------------------------------

void
Disk::UpdateLast(int newSector)
{
    int rotate;
    int seek = TimeToSeek(newSector, &rotate);
    
    if (seek != 0)
		bufferInit = stats->totalTicks + seek + rotate;					/* Liang DePeng + */
		//bufferInit = kernel->stats->totalTicks + seek + rotate;		/* Liang DePeng - */
    lastSector = newSector;
    DEBUG(dbgDisk, "Updating last sector = " << lastSector << " , " << bufferInit);
}
