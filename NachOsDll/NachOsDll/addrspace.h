// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.





#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "translate.h"
#include "table.h"	/*zhong_expan + */
#include "openfile.h"		/*zhong_expan + */

//class OpenNaFile;	/*zhong_expan + */
class SynchBitmap;	/*zhong_expan + */
//class TranslationEntry;	/*zhong_expan + */

#define UserStackSize		1024 	// increase this as necessary!

//  this is the maximum number of open files per process
//  file identifiers are guaranteed to be greater than one (because
//  we reserve fileIDs 0 and 1 for the console) and less than
//  MaxOpenFiles+2

#define MaxOpenFiles            32      // maximum number of open files		/*zhong_expan + */
                                        // per address space

// Maximum length of string for UserStringToKernel

#define  MaxStringArgLength 256		/*zhong_expan + */

enum TranslationStatus {		/*zhong_expan + */
  OK,
  PageFault,
  ProtectionError,
  BusError,          // invalid physical address
  AddressError       // invalid virtual address
};



//class AddrSpace {		/* Liang DePeng - */
class __declspec(dllexport) AddrSpace{		/* Liang DePeng + */
public:
    //AddrSpace();			// Create an address space.		/*zhong_expan - */
    // Create an address space, initializing it with the program
    // stored in the file "executable"
    // "map" is a bitmap that can be used to find free memory frames
    // "myid" is the unique identifier for the new address space
    //AddrSpace(OpenNaFile *executable, SynchBitmap *map,int myid);		/*zhong_expan + */
	AddrSpace(OpenNaFile *executable, SynchBitmap *map,int myid, int pageSize);	/* Liang DePeng + */

	virtual ~AddrSpace() = 0{};		// De-allocate an address space

    //bool Load(char *fileName);		// Load a program into addr space from		/*zhong_expan - */
                                        // a file
					// return false if not found

    //void Execute();             	// Run a program		/*zhong_expan - */
					// assumes the program has already
                                        // been loaded

    virtual void SaveState() = 0;			// Save/restore address space-specific
    virtual void RestoreState() = 0;		// info on a context switch 

    virtual void InitRegisters() = 0;		// Initialize user-level CPU registers,	/*zhong_expan + */
					// before jumping to user code

    // Translate virtual address _vaddr_
    // to physical address _paddr_. _mode_
    // is 0 for Read, 1 for Write.
	//ExceptionType Translate(unsigned int vaddr, unsigned int *paddr, int mode);		/*zhong_expan - */
	
	virtual TranslationStatus ReadMem(int addr, int size, int *value) = 0;  // read from virtual address	/*zhong_expan + */
    virtual TranslationStatus WriteMem(int addr, int size, int value) = 0; // write to virtual address	/*zhong_expan + */

    virtual int AddOpenFile(OpenNaFile *file) = 0;   // Add a new open file to the		/*zhong_expan + */
                                        //  open file list.  Returns an
                                        //  integer file identifier
    virtual OpenNaFile* DeleteOpenFile(int fId) = 0;  // Remove file with the specified		/*zhong_expan + */
                                        //  identifier from the open file list
                                        // and return the open file pointer
    virtual OpenNaFile* GetOpenFile(int fId) = 0;   // Return the open file with the		/*zhong_expan + */
                                        //  specified identifier

    // utility functions for copying data between address space
    // and kernel data structures
    virtual int UserStringToKernel(int vaddr, char* buf) = 0;	/*zhong_expan + */
    virtual int UserBufToKernel(int vaddr, int length, char* buf) = 0;	/*zhong_expan + */
    virtual int KernelToUserBuf(int vaddr, int length, char* buf) = 0;	/*zhong_expan + */

    int  id;                        // a unique identifier for		/*zhong_expan + */
                                        // this address space.
                                        // a negative value indicates
                                        //  an invalid address space that
                                        // could not be properly initialized
                                        // because of insufficient main
                                        // memory

	TranslationEntry** pageTable;	/* Liang DePeng + */
	int numPages;					/* Liang DePeng + */

protected:

	TranslationStatus Translate(int vaddr,int *paddr, bool write);		/*zhong_expan + */

	 // Assume linear page table translation for now!	
    //TranslationEntry* pageTable;	/* Liang DePeng - */				
	
    //unsigned int numPages;		// Number of pages in the virtual 		/*zhong_expan - */
	//int numPages;		// Number of pages in the virtual 		/*zhong_expan + */
					// address space
	Table* openFiles;                   // An table of pointers		/*zhong_expan + */
                                        // to open files associated
                                        //  with this address space
    SynchBitmap*  memMap;              // pointer to memory map		/*zhong_expan + */

    //void InitRegisters();		// Initialize user-level CPU registers,	/*zhong_expan - */
				//	// before jumping to user code

	int PageSize;	/* Liang DePeng + */
};

/**** add
 * Liang DePeng + */
typedef AddrSpace* (*GetAddrSpace)(OpenNaFile *executable, SynchBitmap *map,int myid, int pageSize);  
typedef void (*DeleteAddrSpace)(AddrSpace*);

extern "C" {
	__declspec(dllexport) AddrSpace* GetAddrSpaceF(OpenNaFile *executable, SynchBitmap *map,int myid, int pageSize);
	__declspec(dllexport) void DeleteAddrSpaceF(AddrSpace*);
}

extern GetAddrSpace getAddrSpace;
extern DeleteAddrSpace deleteAddrSpace;	

/* Liang DePeng +
 ****/


#endif // ADDRSPACE_H
