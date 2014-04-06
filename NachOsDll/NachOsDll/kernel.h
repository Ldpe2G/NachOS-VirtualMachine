// kernel.h
//	Global variables for the Nachos kernel.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.



#ifndef KERNEL_H
#define KERNEL_H


#include "copyright.h"
#include "debug.h"
#include "utility.h"

#include "scheduler.h"
//#include "interrupt.h"	/* Liang DePeng - */
//#include "stats.h"		/* Liang DePeng - */
#include "alarm.h"
#include "filesys.h"
//#include "machine.h"		/* Liang DePeng - */
#include "synch.h"          /* Liang DePeng + */
#include "synchconsole.h"

//class PostOfficeInput;	/* Liang DePeng - */
//class PostOfficeOutput;	/* Liang DePeng - */
class SynchConsoleInput;
class SynchConsoleOutput;
class SynchBitmap;		/*zhong_expan + */
class ProcTable;		/*zhong_expan + */
class SynchDisk;


#define stackReg	29	// User's stack pointer
#define retAddrReg	31	// Holds return address for procedure calls
#define numGPRegs	32	// 32 general purpose registers on MIPS
#define hiReg		32	// Double register to hold multiply result
#define loReg		33
#define pCReg		34	// Current program counter
#define nextPCReg	35	// Next program counter (for branch delay) 
#define prevPCReg	36	// Previous program counter (for debugging)
#define loadReg		37	// The register target of a delayed load.
#define loadValueReg 	38	// The value to be loaded by a delayed load.
#define badVAddrReg	39	// The failing virtual address on an exception

typedef int (* SetInterruptLevel)(int); /* Liang DePeng + */
typedef int (* GetInterruptLevel)();			/* Liang DePeng + */
typedef void (* InterruptIdle)();					/* Liang DePeng + */
typedef int (* ReadVMRegister)(int/*num*/);			/* Liang DePeng + */
typedef void (* WriteVMRegister)(int/* num*/, int/* value*/);	/* Liang DePeng + */
typedef void (* RestoreMachineState)(TranslationEntry** /* pageTable*/, int/* numPages*/);	/* Liang DePeng + */
typedef void (* InterruptHalt)();	 /* Liang DePeng + */
typedef void (* MachineRun)();		/* Liang DePeng + */

typedef void (* InitializeDisk)(CallBackObj *toCall);		/* Liang DePeng + */
typedef void (* DiskReadRequest)(int sectorNumber, char* data);		/* Liang DePeng + */
typedef void (* DiskWriteRequest)(int sectorNumber, char* data);	/* Liang DePeng + */

//class Kernel {	/* Liang DePeng - */
class __declspec(dllexport) Kernel {	/* Liang DePeng + */
public:
    Kernel(int argc, char **argv,
			char *mainMemoryy,
			int numTotalRegss,
			int memorySizee,
			int diskNumSectorss,
			int diskSectorSizee,
			DiskReadRequest diskReadRequestt,		
			DiskWriteRequest diskWriteRequestt,
			InitializeDisk initDiskk,
			MachineRun machineRunn,
			InterruptHalt interruptHaltt,
			RestoreMachineState restoreStatee,
			SetInterruptLevel setLevell, 
			GetInterruptLevel getLevell, InterruptIdle idlee,
			ReadVMRegister readRegisterr, WriteVMRegister writeRegisterr);
    				// Interpret command line arguments
	virtual ~Kernel() = 0{};		        // deallocate the kernel
    
	//void Initialize(int numPhysPagess);	/* Liang DePeng - */
    virtual void Initialize(int numPhysPagess,  
							AlarmCallBack alarmCallBackk,
							void (* initConsoleInput)(char *inputFile, CallBackObj *callTo),
							void (* initConsoleOutput)(char *outputFile, CallBackObj *callTo),
							void (* initTimer)(bool doRandom, CallBackObj *toCall),
							ConGetChar getCharr,
							ConPutChar putCharr) = 0 /* Liang DePeng + */; 		
				// initialize the kernel -- separated
				// from constructor because 
				// refers to "kernel" as a global
	
	//void ThreadSelfTest();	/* Liang DePeng - */
    virtual void ThreadSelfTest() = 0;	/* Liang DePeng + */ // self test of threads and synchronization

	//void ConsoleTest();	/* Liang DePeng - */
    virtual void ConsoleTest() = 0;   /* Liang DePeng + */      // interactive console self test

	//分离到了虚拟机部分
    //void NetworkTest();         // interactive 2-machine network test
    
// These are public for notational convenience; really, 
// they're global variables used everywhere.

    Thread *currentThread;	// the thread holding the CPU
    Scheduler *scheduler;	// the ready list
    
	//分离到了虚拟机	
	//Interrupt *interrupt;	// interrupt status			/* Liang DePeng - */
    //Statistics *stats;		// performance metrics	/* Liang DePeng - */
    //Machine *machine;           // the simulated CPU	/* Liang DePeng - */
	//PostOfficeInput *postOfficeIn;					/* Liang DePeng - */
	//PostOfficeOutput *postOfficeOut;					/* Liang DePeng - */
    //int hostName;               // machine identifier	/* Liang DePeng - */
	//分离到了虚拟机

	SetInterruptLevel setLevel;		/* Liang DePeng + */
	GetInterruptLevel getLevel;		/* Liang DePeng + */
	InterruptIdle idle;				/* Liang DePeng + */
	ReadVMRegister readRegister;	/* Liang DePeng + */
	WriteVMRegister writeRegister;	/* Liang DePeng + */
	RestoreMachineState restoreState;	/* Liang DePeng + */
	InterruptHalt interruptHalt;	/* Liang DePeng + */
	MachineRun machineRun;			/* Liang DePeng + */
	InitializeDisk initDisk;		/* Liang DePeng + */
	DiskReadRequest diskReadRequest;		/* Liang DePeng + */
	DiskWriteRequest diskWriteRequest;		/* Liang DePeng + */


	Alarm *alarm;		// the software alarm clock    
    SynchConsoleInput *synchConsoleIn;
    SynchConsoleOutput *synchConsoleOut;
    SynchBitmap *frameMap;    // bitmap to track memory frames		/*zhong_expan + */
    ProcTable* procTable;    // table of all processes				/*zhong_expan + */
    SynchDisk *synchDisk;
    FileSystem *fileSystem;  

	char *mainMemory;	//machine MainMemory		/* Liang DePeng + */
	int numTotalRegs;	//machine registers num		/* Liang DePeng + */
	int numPhysPages;	//machine physical pages	/* Liang DePeng + */
	int memorySize;	//machine memory size (numPhysPages * phyPageSize)		/* Liang DePeng + */
	int diskNumSectors; // total # of sectors per disk
	int diskSectorSize; // number of bytes per disk sector

protected:
    bool randomSlice;		// enable pseudo-random time slicing
    bool debugUserProg;         // single step user program
    //double reliability;         // likelihood messages are dropped	/* Liang DePeng - */
    char *consoleIn;            // file to read console input from
    char *consoleOut;           // file to send console output to
#ifndef FILESYS_STUB
    bool formatFlag;          // format the disk if this is true
#endif
};

/**** add
 * Liang DePeng + */
//宏定义函数指针
typedef Kernel* (*GetKernel)(int argc, char **argv,
							char *mainMemoryy,
							int numTotalRegss,
							int memorySizee,
							int diskNumSectorss,
							int diskSectorSizee,
							DiskReadRequest diskReadRequestt,		
							DiskWriteRequest diskWriteRequestt,
							InitializeDisk initDiskk,
							MachineRun machineRunn,
							InterruptHalt interruptHaltt,
							RestoreMachineState restoreStatee,
							SetInterruptLevel setLevell, 
							GetInterruptLevel getLevell, InterruptIdle idlee,
							ReadVMRegister readRegisterr, WriteVMRegister writeRegisterr);  
typedef void (*DeleteKernel)(Kernel*);

extern "C" {
	__declspec(dllexport) Kernel* GetKernelF(int argc, char **argv,
											char *mainMemoryy,
											int numTotalRegss,
											int memorySizee,
											int diskNumSectorss,
											int diskSectorSizee,
											DiskReadRequest diskReadRequestt,		
											DiskWriteRequest diskWriteRequestt,
											InitializeDisk initDiskk,
											MachineRun machineRunn,
											InterruptHalt interruptHaltt,
											RestoreMachineState restoreStatee,
											SetInterruptLevel setLevell, 
											GetInterruptLevel getLevell, InterruptIdle idlee,
											ReadVMRegister readRegisterr, WriteVMRegister writeRegisterr);
	__declspec(dllexport) void DeleteKernelF(Kernel*);
}

extern GetKernel getKernel;
extern DeleteKernel deleteKernel;	
/* Liang DePeng +
 ****/

extern Kernel *kernel; /* Liang DePeng + */


#endif // KERNEL_H


