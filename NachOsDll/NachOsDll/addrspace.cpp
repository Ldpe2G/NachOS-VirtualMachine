// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -n -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you are using the "stub" file system, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
//#include "main.h"			/* Liang Depeng - */
#include "addrspace.h"
//#include "machine.h"		/* Liang Depeng - */
#include "noff.h"

//#include "translate.h"		/*zhong_expan + */
#include "synchbitmap.h"		/*zhong_expan + */
#include "debug.h"
#include "kernel.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void  SwapHeader (NoffHeader *noffH){

    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
#ifdef RDATA
    noffH->readonlyData.size = WordToHost(noffH->readonlyData.size);
    noffH->readonlyData.virtualAddr = 
           WordToHost(noffH->readonlyData.virtualAddr);
    noffH->readonlyData.inFileAddr = 
           WordToHost(noffH->readonlyData.inFileAddr);
#endif 
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);

#ifdef RDATA
    DEBUG(dbgAddr, "code = " << noffH->code.size <<  
                   " readonly = " << noffH->readonlyData.size <<
                   " init = " << noffH->initData.size <<
                   " uninit = " << noffH->uninitData.size << "\n");
#endif
}



/**** add
 * Liang DePeng + */
class __declspec(dllexport) MyAddrSpace: public AddrSpace {	/* Liang DePeng + */
public:
    //AddrSpace();			// Create an address space.		/*zhong_expan - */
    // Create an address space, initializing it with the program
    // stored in the file "executable"
    // "map" is a bitmap that can be used to find free memory frames
    // "myid" is the unique identifier for the new address space
    //AddrSpace(OpenNaFile *executable, SynchBitmap *map,int myid);		/*zhong_expan + */
	
	MyAddrSpace(OpenNaFile *executable, 
		SynchBitmap *map,
		int myid, 
		int pageSize):AddrSpace(executable, map, myid, pageSize){}	/* Liang DePeng + */

	~MyAddrSpace();		// De-allocate an address space

    //bool Load(char *fileName);		// Load a program into addr space from		/*zhong_expan - */
                                        // a file
					// return false if not found

    //void Execute();             	// Run a program		/*zhong_expan - */
					// assumes the program has already
                                        // been loaded

     void SaveState() ;			// Save/restore address space-specific
     void RestoreState() ;		// info on a context switch 

     void InitRegisters() ;		// Initialize user-level CPU registers,	/*zhong_expan + */
					// before jumping to user code

    // Translate virtual address _vaddr_
    // to physical address _paddr_. _mode_
    // is 0 for Read, 1 for Write.
	//ExceptionType Translate(unsigned int vaddr, unsigned int *paddr, int mode);		/*zhong_expan - */
	 //TranslationStatus Translate(int vaddr,int *paddr, bool write) ;		/*zhong_expan + */

	 TranslationStatus ReadMem(int addr, int size, int *value) ;  // read from virtual address	/*zhong_expan + */
     TranslationStatus WriteMem(int addr, int size, int value) ; // write to virtual address	/*zhong_expan + */

     int AddOpenFile(OpenNaFile *file) ;   // Add a new open file to the		/*zhong_expan + */
                                        //  open file list.  Returns an
                                        //  integer file identifier
     OpenNaFile* DeleteOpenFile(int fId) ;  // Remove file with the specified		/*zhong_expan + */
                                        //  identifier from the open file list
                                        // and return the open file pointer
     OpenNaFile* GetOpenFile(int fId) ;   // Return the open file with the		/*zhong_expan + */
                                        //  specified identifier

    // utility functions for copying data between address space
    // and kernel data structures
     int UserStringToKernel(int vaddr, char* buf) ;	/*zhong_expan + */
     int UserBufToKernel(int vaddr, int length, char* buf) ;	/*zhong_expan + */
     int KernelToUserBuf(int vaddr, int length, char* buf) ;	/*zhong_expan + */
};

extern "C" {
	__declspec(dllexport) AddrSpace* GetAddrSpaceF(OpenNaFile *executable, SynchBitmap *map,int myid, int pageSize){
		return new MyAddrSpace(executable, map, myid, pageSize);
	}

	__declspec(dllexport) void DeleteAddrSpaceF(AddrSpace* as){
		delete as;
	}
}
/* Liang DePeng +
 ****/

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//----------------------------------------------------------------------
//
//AddrSpace::AddrSpace()		/*zhong_expan - */
//{
//    pageTable = new TranslationEntry[NumPhysPages];
//    for (int i = 0; i < NumPhysPages; i++) {
//	pageTable[i].virtualPage = i;	// for now, virt page # = phys page #
//	pageTable[i].physicalPage = i;
//	pageTable[i].valid = TRUE;
//	pageTable[i].use = FALSE;
//	pageTable[i].dirty = FALSE;
//	pageTable[i].readOnly = FALSE;  
//    }
//    
//    // zero out the entire address space
//    bzero(kernel->machine->mainMemory, MemorySize);
//}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      "executable" is the file containing the object code to load into memory
//      "map" is a bitmap that can be used to determine which frames
//         are available
//      "myid" is the unique identifier for this address space
//
//      If the address space cannot be initialized because of lack
//      of space in primary memory, the constructor sets the id
//      of the address space to -1.
//----------------------------------------------------------------------
//AddrSpace::AddrSpace(OpenNaFile *executable,SynchBitmap* map,int myid)				/*zhong_expan + */
AddrSpace::AddrSpace(OpenNaFile *executable,SynchBitmap* map,int myid, int pageSize){	/* Liang DePeng + */
	
	
	NoffHeader noffH;
	int i;
	int paddr;
	int pagenum;
	int numbytes;

	// initialize object
	PageSize = pageSize;	/* Liang DePeng + */
	memMap = map;
	id = myid;

	//pageTable = (TranslationEntry *)0;	/* Liang DePeng - */
	pageTable = (TranslationEntry **)0;		/* Liang DePeng + */
	numPages = 0;
	openFiles = (Table *)0;

	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);

	if (noffH.noffMagic != NOFFMAGIC)
	{
		// doesn't look like executable is a noff file
		id = -1;
		return;
	}

	// how big is the address space?
	// the following assumes that each segment is page-aligned
	numPages = divRoundUp(noffH.code.size, PageSize) + 
		divRoundUp(noffH.initData.size,PageSize) + 
		divRoundUp(noffH.uninitData.size,PageSize) +
		divRoundUp(UserStackSize,PageSize); 
#ifdef RDATA
	numPages += divRoundUp(noffH.readonlyData.size,PageSize);
#endif

	DEBUG(dbgAddr, "Initializing address space: " << numPages << " pages."); 
#ifdef RDATA
	DEBUG(dbgAddr, "code = " << noffH.code.size <<  
		" readonly = " << noffH.readonlyData.size <<
		" init = " << noffH.initData.size <<
		" uninit = " << noffH.uninitData.size);
#else
	DEBUG(dbgAddr, "code = " << noffH.code.size <<  
		" init = " << noffH.initData.size <<
		" uninit = " << noffH.uninitData.size);
#endif

	// Set up a translation table for this process
	//pageTable = new TranslationEntry[numPages];	/* Liang DePeng - */
	pageTable = new TranslationEntry*[numPages];	/* Liang DePeng + */
	for(i=0; i < numPages; i++)						/* Liang DePeng + */
		pageTable[i] = GetTranslationEntryF();		/* Liang DePeng + */

	ASSERT(pageTable != (TranslationEntry**)0);
	// mark all entries as invalid initially
	for (i = 0; i < numPages; i++){
		//pageTable[i].valid = FALSE;	/* Liang DePeng - */
		pageTable[i]->valid = FALSE;	/* Liang DePeng + */
	}

	/**** add
	 * Liang DePeng + */
	// initialize the page table
	for (i = 0; i < numPages; i++){
		pageTable[i]->virtualPage = i;
		pageTable[i]->physicalPage = memMap->FindAndSet();
		if (pageTable[i]->physicalPage < 0){
			// we ran out of free frames
			// until we have paging, this means we cannot run the process
			id = -1;  // mark this as a bad address space
			return;
		}
		pageTable[i]->valid = TRUE;
		pageTable[i]->use = FALSE;
		pageTable[i]->dirty = FALSE;
		// this will be changed later for read-only pages
		pageTable[i]->readOnly = FALSE; 
	}
	/* Liang DePeng +
	 ****/


	/**** delete
	 * Liang DePeng - */
	/*for (i = 0; i < numPages; i++){
		pageTable[i].virtualPage = i;
		pageTable[i].physicalPage = memMap->FindAndSet();
		if (pageTable[i].physicalPage < 0){
			// we ran out of free frames
			// until we have paging, this means we cannot run the process
			id = -1;  // mark this as a bad address space
			return;
		}
		pageTable[i].valid = TRUE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		// this will be changed later for read-only pages
		pageTable[i].readOnly = FALSE; 
	}*/
	/* Liang DePeng -
	 ****/


	// zero out the entire address space, to zero the unitialized data segment 
	// and the stack segment and any partially-filled pages
	// do this one page at a time
	for(i=0;i<numPages;i++){
		bzero(&(kernel->mainMemory[pageTable[i]->physicalPage*PageSize]), PageSize);	/* Liang DePeng + */
		//bzero(&(kernel->machine->mainMemory[pageTable[i].physicalPage*PageSize]),PageSize);	/* Liang DePeng - */
	}

	// create an open file table
	openFiles = new Table(MaxOpenFiles);
	ASSERT(openFiles != (Table*)0);

	// initialize the code segment
	while(noffH.code.size > 0){
		if (PageSize < noffH.code.size){
			numbytes = PageSize;
		}
		else{
			numbytes = noffH.code.size;
		}
		pagenum = noffH.code.virtualAddr / PageSize;
		ASSERT(pagenum >= 0);
		ASSERT(pagenum < numPages);
		if (Translate(noffH.code.virtualAddr,&paddr,FALSE) != OK){
			ASSERT(FALSE);
		}
		ASSERT(paddr >= 0);
		// load the code from the file
		executable->ReadAt(&(kernel->mainMemory[paddr]),
			numbytes, noffH.code.inFileAddr);
		DEBUG(dbgAddr, "Loaded code segment page. VA: " << noffH.code.virtualAddr << " bytes: " << numbytes); 
		pageTable[pagenum]->use = FALSE; // undo side-effect of Translate
		pageTable[pagenum]->readOnly = TRUE;
		noffH.code.size -= numbytes;
		noffH.code.inFileAddr += numbytes;
		noffH.code.virtualAddr += numbytes;
	}
	// initialize the read only data segment
#ifdef RDATA
	while(noffH.readonlyData.size > 0){
		if (PageSize < noffH.readonlyData.size){
			numbytes = PageSize;
		} else {
			numbytes = noffH.readonlyData.size;
		}
		pagenum = noffH.readonlyData.virtualAddr / PageSize;
		ASSERT(pagenum >= 0);
		ASSERT(pagenum < numPages);
		if (Translate(noffH.readonlyData.virtualAddr,&paddr,FALSE) != OK){
			ASSERT(FALSE);
		}
		ASSERT(paddr >= 0);
		// load the read-only data from the file
		executable->ReadAt(&(kernel->mainMemory[paddr]), numbytes, noffH.readonlyData.inFileAddr);
		DEBUG(dbgAddr, "Loaded read-only data segement page. VA: " << noffH.readonlyData.virtualAddr << " bytes: " << numbytes); 
		pageTable[pagenum]->use = FALSE;  // undo side-effect of Translate
		pageTable[pagenum]->readOnly = TRUE;
		noffH.readonlyData.size -= numbytes;
		noffH.readonlyData.inFileAddr += numbytes;
		noffH.readonlyData.virtualAddr += numbytes;
	}
#endif
	// initialize the initialized data segment
	while(noffH.initData.size > 0){
		if (PageSize < noffH.initData.size){
			numbytes = PageSize;
		}
		else{
			numbytes = noffH.initData.size;
		}
		pagenum = noffH.initData.virtualAddr / PageSize;
		ASSERT(pagenum >= 0);
		ASSERT(pagenum < numPages);
		if (Translate(noffH.initData.virtualAddr,&paddr,FALSE) != OK){
			ASSERT(FALSE);
		}
		ASSERT(paddr >= 0);
		// zero out the page if it will only be partially filled
		if (numbytes < PageSize){
			bzero(&(kernel->mainMemory[paddr]),PageSize);
		}
		// initialized the data from the file
		executable->ReadAt(&(kernel->mainMemory[paddr]),
			numbytes, noffH.initData.inFileAddr);
		DEBUG(dbgAddr, "Loaded initialized data segment page. VA: "<< noffH.initData.virtualAddr << " bytes: " << numbytes); 
		pageTable[pagenum]->use = FALSE;  // undo side-effect of Translate
		noffH.initData.size -= numbytes;
		noffH.initData.inFileAddr += numbytes;
		noffH.initData.virtualAddr += numbytes;
	}
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------

//AddrSpace::~AddrSpace()		/*zhong_expan - */
//{
//   delete pageTable;
//}

MyAddrSpace::~MyAddrSpace(){		
	int i;
	OpenNaFile* f;

	if (pageTable != (TranslationEntry**)0){
		// free up the frames we are using
		for(i=0;i<numPages;i++){
			if (pageTable[i]->valid){
				memMap->Clear(pageTable[i]->physicalPage);
			}
			// delete the page table
			DeleteTranslationEntryF(pageTable[i]);
		}
	}

	if (openFiles != (Table*)0){
		// close any open files associated with this address space
		for(i=0;i<MaxOpenFiles;i++)
		{
			f = (OpenNaFile*)(openFiles->Fetch(i));
			if (f != (OpenNaFile*)0) delete f;
		}
		// delete the openFiles table itself
		delete openFiles;
	}
}

//----------------------------------------------------------------------
// AddrSpace::Load
// 	Load a user program into memory from a file.
//
//	Assumes that the page table has been initialized, and that
//	the object code file is in NOFF format.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------
//
//bool 
//AddrSpace::Load(char *fileName) 		/*zhong_expan - */
//{
//    OpenFile *executable = kernel->fileSystem->Open(fileName);
//    NoffHeader noffH;
//    unsigned int size;
//
//    if (executable == NULL) {
//	cerr << "Unable to open file " << fileName << "\n";
//	return FALSE;
//    }
//
//    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
//    if ((noffH.noffMagic != NOFFMAGIC) && 
//		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
//    	SwapHeader(&noffH);
//    ASSERT(noffH.noffMagic == NOFFMAGIC);
//
//#ifdef RDATA
//// how big is address space?
//    size = noffH.code.size + noffH.readonlyData.size + noffH.initData.size +
//           noffH.uninitData.size + UserStackSize;	
//                                                // we need to increase the size
//						// to leave room for the stack
//#else
//// how big is address space?
//    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
//			+ UserStackSize;	// we need to increase the size
//						// to leave room for the stack
//#endif
//    numPages = divRoundUp(size, PageSize);
//    size = numPages * PageSize;
//
//    ASSERT(numPages <= NumPhysPages);		// check we're not trying
//						// to run anything too big --
//						// at least until we have
//						// virtual memory
//
//    DEBUG(dbgAddr, "Initializing address space: " << numPages << ", " << size);
//
//// then, copy in the code and data segments into memory
//// Note: this code assumes that virtual address = physical address
//    if (noffH.code.size > 0) {
//        DEBUG(dbgAddr, "Initializing code segment.");
//	DEBUG(dbgAddr, noffH.code.virtualAddr << ", " << noffH.code.size);
//        executable->ReadAt(
//		&(kernel->machine->mainMemory[noffH.code.virtualAddr]), 
//			noffH.code.size, noffH.code.inFileAddr);
//    }
//    if (noffH.initData.size > 0) {
//        DEBUG(dbgAddr, "Initializing data segment.");
//	DEBUG(dbgAddr, noffH.initData.virtualAddr << ", " << noffH.initData.size);
//        executable->ReadAt(
//		&(kernel->machine->mainMemory[noffH.initData.virtualAddr]),
//			noffH.initData.size, noffH.initData.inFileAddr);
//    }
//
//#ifdef RDATA
//    if (noffH.readonlyData.size > 0) {
//        DEBUG(dbgAddr, "Initializing read only data segment.");
//	DEBUG(dbgAddr, noffH.readonlyData.virtualAddr << ", " << noffH.readonlyData.size);
//        executable->ReadAt(
//		&(kernel->machine->mainMemory[noffH.readonlyData.virtualAddr]),
//			noffH.readonlyData.size, noffH.readonlyData.inFileAddr);
//    }
//#endif
//
//    delete executable;			// close file
//    return TRUE;			// success
//}

//----------------------------------------------------------------------
// AddrSpace::Execute
// 	Run a user program using the current thread
//
//      The program is assumed to have already been loaded into
//      the address space
//
//----------------------------------------------------------------------

//void 
//AddrSpace::Execute() 		/*zhong_expan - */
//{
//
//    kernel->currentThread->space = this;
//
//    this->InitRegisters();		// set the initial register values
//    this->RestoreState();		// load page table register
//
//    kernel->machine->Run();		// jump to the user progam
//
//    ASSERTNOTREACHED();			// machine->Run never returns;
//					// the address space exits
//					// by doing the syscall "exit"
//}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

//void AddrSpace::InitRegisters(){	/* Liang DePeng - */
void MyAddrSpace::InitRegisters(){	/* Liang DePeng + */

    //Machine *machine = kernel->machine;	/* Liang DePeng - */
    int i;

    for (i = 0; i < kernel->numTotalRegs; i++)
		(* kernel->writeRegister)(i, 0);	/* Liang DePeng + */
		//machine->WriteRegister(i, 0);		/* Liang DePeng - */

    // Initial program counter -- must be location of "Start", which
    //  is assumed to be virtual address zero
	(* kernel->writeRegister)(pCReg, 0);	/* Liang DePeng + */
    //machine->WriteRegister(PCReg, 0);		/* Liang DePeng - */

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    // Since instructions occupy four bytes each, the next instruction
    // after start will be at virtual address four.
	(* kernel->writeRegister)(nextPCReg, 4);	/* Liang DePeng + */
    //machine->WriteRegister(NextPCReg, 4);		/* Liang DePeng - */

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
	(* kernel->writeRegister)(stackReg, numPages * PageSize - 16);	/* Liang DePeng + */
    //machine->WriteRegister(StackReg, numPages * PageSize - 16);	/* Liang DePeng - */
    DEBUG(dbgAddr, "Initializing stack pointer: " << numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, don't need to save anything!
//----------------------------------------------------------------------

void MyAddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

//void AddrSpace::RestoreState(){	/* Liang DePeng - */
void MyAddrSpace::RestoreState(){	/* Liang DePeng + */
	(* kernel->restoreState)(pageTable, numPages);	/* Liang DePeng + */
    //kernel->machine->pageTable = pageTable;		/* Liang DePeng - */
    //kernel->machine->pageTableSize = numPages;	/* Liang DePeng - */
}


//----------------------------------------------------------------------
// AddrSpace::Translate
//  Translate the virtual address in _vaddr_ to a physical address
//  and store the physical address in _paddr_.
//  The flag _isReadWrite_ is false (0) for read-only access; true (1)
//  for read-write access.
//  Return any exceptions caused by the address translation.
//----------------------------------------------------------------------
//ExceptionType
//AddrSpace::Translate(unsigned int vaddr, unsigned int *paddr, int isReadWrite)		/*zhong_expan - */
//{
//    TranslationEntry *pte;
//    int               pfn;
//    unsigned int      vpn    = vaddr / PageSize;
//    unsigned int      offset = vaddr % PageSize;
//
//    if(vpn >= numPages) {
//        return AddressErrorException;
//    }
//
//    pte = &pageTable[vpn];
//
//    if(isReadWrite && pte->readOnly) {
//        return ReadOnlyException;
//    }
//
//    pfn = pte->physicalPage;
//
//    // if the pageFrame is too big, there is something really wrong!
//    // An invalid translation was loaded into the page table or TLB.
//    if (pfn >= NumPhysPages) {
//        DEBUG(dbgAddr, "Illegal physical page " << pfn);
//        return BusErrorException;
//    }
//
//    pte->use = TRUE;          // set the use, dirty bits
//
//    if(isReadWrite)
//        pte->dirty = TRUE;
//
//    *paddr = pfn*PageSize + offset;
//
//    ASSERT((*paddr < MemorySize));
//
//    //cerr << " -- AddrSpace::Translate(): vaddr: " << vaddr <<
//    //  ", paddr: " << *paddr << "\n";
//
//    return NoException;
//}

//-------------------------------------------------------------------
// AddrSpace::Translate
//      map a virtual address to a physical one
//
//   "vaddr" is the virtual address to translate.
//   "*paddr" is the resulting physical address
//   "write" should be true if the translation is for an update,
//            false otherwise
//
//   return value is a TranslationStatus indicating whether or
//     not the translation was successful.
//   a physical address is written into *paddr only if the
//    return value is "OK"
//
//-------------------------------------------------------------------

//TranslationStatus AddrSpace::Translate(int vaddr,int *paddr, bool write)	/*zhong_expan + */	/* Liang DePeng - */
TranslationStatus AddrSpace::Translate(int vaddr,int *paddr, bool write)	/* Liang DePeng + */
{
	int pagenum;
	int offset;
	int framenum;

	ASSERT(vaddr >= 0);
	ASSERT(paddr != (int *)0);
	pagenum = vaddr/PageSize;
	if (pagenum >= numPages)
	{
		return(AddressError);
	}
	if (!pageTable[pagenum]->valid)
	{
		return(PageFault);
	}
	if (write && pageTable[pagenum]->readOnly)
	{
		return(ProtectionError);
	}
	framenum = pageTable[pagenum]->physicalPage;
	if ((framenum >= kernel->numPhysPages) || (framenum < 0))
	{
		return(BusError);
	}
	pageTable[pagenum]->use = TRUE;
	if (write)
	{
		pageTable[pagenum]->dirty = TRUE;
	}
	offset = vaddr%PageSize;
	*paddr = framenum*PageSize + offset;
	ASSERT(*paddr < kernel->memorySize);
	ASSERT(*paddr >= 0);
	return(OK);
}

//---------------------------------------------------------------------
// AddrSpace::UserStringToKernel
//    copy a string from an address space into
//    a kernel character buffer, which must be provided.
//
//    copying stops when a string terminator (\0) is found
//
//    At most MaxStringArgLength bytes will be copied
//
//   "vaddr" is the virtual address of the start of the string
//
//   "buf" points to a character buffer which will hold the string.
//
//   returns the length of the copied string if a string
//   terminator is found, -1 otherwise
//
//----------------------------------------------------------------------
//int AddrSpace::UserStringToKernel(int vaddr, char* buf)		/*zhong_expan + */	/* Liang DePeng - */
int MyAddrSpace::UserStringToKernel(int vaddr, char* buf)		/* Liang DePeng + */
{
	int length = 0;
	int dat;
	TranslationStatus status;

	DEBUG(dbgAddr, "Reading user string to kernel, starting virtual address: " << vaddr); 
	while(length < MaxStringArgLength)
	{
		status = ReadMem(vaddr,1,&dat);
		// n.b. this will fail if user string's page(s) are not in
		// memory.
		if (status != OK)
		{
			return(-1);
		}
		*buf = (char)dat;
		vaddr++;
		length++;
		if (*buf == '\0')
			break;
		buf++;
	}
	if (length >= MaxStringArgLength)
	{
		return(-1);
	}
	else
	{
		return(length);
	}
}

//---------------------------------------------------------------------
// AddrSpace::UserBufToKernel
//    copy a buffer of known length from an address space into
//    a kernel character buffer, which must be provided.
//
//   "vaddr" is the virtual address of the start of the buffer
//   "length" is its length
//
//   "buf" points to the target buffer
//
//   returns "length" on success, else -1
//----------------------------------------------------------------------
//int AddrSpace::UserBufToKernel(int vaddr, int length, char* buf)	{	/* Liang DePeng - */
int MyAddrSpace::UserBufToKernel(int vaddr, int length, char* buf)	{	/* Liang DePeng + */
	int knt;
	TranslationStatus status;
	int dat;

	DEBUG(dbgAddr, "Reading user buffer to kernel, starting virtual address: " << vaddr << " ,length " << length << " bytes."); 
	for(knt=0;knt<length;knt++)
	{
		status = ReadMem(vaddr,1,&dat);
		// n.b. this will fail if user buf's page(s) are not in
		// memory.
		if (status != OK) 
		{
			return(-1);
		}
		*buf = (char)dat;
		vaddr++;
		buf++;
	}
	return(length);
}

//---------------------------------------------------------------------
// AddrSpace::KernelToUserBuf
//    copy a string of known length from a kernel into
//    an address space
//
//   "vaddr" is the virtual address of the start of the target buffer
//
//   "length" is the length of the buffer.
//
//   "buf" points to a kernel source buffer
//
//   returns "length" on success, else -1
//----------------------------------------------------------------------
//int AddrSpace::KernelToUserBuf(int vaddr, int length, char* buf){	/* Liang DePeng - */
int MyAddrSpace::KernelToUserBuf(int vaddr, int length, char* buf){	/* Liang DePeng + */
	TranslationStatus status;
	int knt;

	DEBUG(dbgAddr, "Writing user buffer from kernel, starting virtual address: " << vaddr << " ,length " << length << " bytes."); 
	for(knt=0;knt < length;knt++)
	{
		status = WriteMem(vaddr,1,(int)(*buf));
		// n.b. this will fail if user buf's page(s) are not in
		// memory.
		if (status != OK)
		{
			return(-1);
		}
		vaddr++;
		buf++;
	}
	return(length);
}

//----------------------------------------------------------------------
// AddrSpace::ReadMem
//      Read "size" (1, 2, or 4) bytes of virtual memory at virtual
//      address "addr" into *value
//
//  Returns a TranslationStatus to indicate whether the required
//    virtual to physical translation was successful.
//  Places data into *value only if the return value is "OK"
//----------------------------------------------------------------------
//TranslationStatus AddrSpace::ReadMem(int addr, int size, int *value)	/*zhong_expan + */{	/* Liang DePeng - */
TranslationStatus MyAddrSpace::ReadMem(int addr, int size, int *value){	/* Liang DePeng + */

	int physicalAddress;
	TranslationStatus status;

	ASSERT(value != (int *)0);
	ASSERT(addr >= 0);
	status = Translate(addr, &physicalAddress, FALSE);
	if (status != OK)
	{
		return(status);
	}
	switch (size){
	case 1:
		*value = kernel->mainMemory[physicalAddress];
		break;

	case 2:
		if (addr & 0x1){
			// alignment error
			return(AddressError);
		}
		*value = ShortToHost(*(unsigned short *) &kernel->mainMemory[physicalAddress]);
		break;

	case 4:
		if (addr & 0x3){
			// alignment error
			return(AddressError);
		}
		*value = WordToHost(*(unsigned int *) &kernel->mainMemory[physicalAddress]);
		break;

	default: ASSERT(FALSE);
	}
	return(OK);
}

//------------------------------------------------------------------
// AddrSpace::AddOpenFile
//     add a new open file to the list of open files for this
//     address space, and assign an unused file identifier to it
//
//     "file" is a pointer to an already-created OpenFile object
//
//     returns an integer, which is the file identifier
//     a negative return value indicates that a new file identifier
//     cannot be created because there are too many in use
//
//     Note that no two files open concurrently
//     in the same address space will
//     have the same file identifier.  However, file identifiers
//      are reused after files are closed.
//     Note also that two opens on the same file will result
//      in two distinct file identifiers (like UNIX).
//------------------------------------------------------------------
//int AddrSpace::AddOpenFile(OpenNaFile* file)	/*zhong_expan + */{	/* Liang DePeng - */

int MyAddrSpace::AddOpenFile(OpenNaFile* file){	/* Liang DePeng + */

	int fileId;

	fileId = openFiles->Insert((void*)file);
	if (fileId < 0)
	{
		// open file table is full
		DEBUG(dbgAddr, "Attempting to add open file: file table full."); 
		return(fileId);
	}
	else
	{
		ASSERT(fileId < MaxOpenFiles);
		DEBUG(dbgAddr, "Adding open file with fileId " << fileId+2); 
		return(fileId+2);
	}
}

//------------------------------------------------------------------
// AddrSpace::DeleteOpenFile
//     if a file with the specified identifier is on the list then
//      return it and delete it from the list, else return a null
//      pointer
//
//     "fId" is the identifier of the desired file
//------------------------------------------------------------------
//OpenNaFile* AddrSpace::DeleteOpenFile(int fId){	/* Liang DePeng - */

OpenNaFile* MyAddrSpace::DeleteOpenFile(int fId){		/* Liang DePeng + */
	OpenNaFile* temp;

	if ((fId < 2) || (fId >= MaxOpenFiles+2)){
		return((OpenNaFile *)0);
	}
	DEBUG(dbgAddr, "Deleting open file with fileId " << fId); 
	temp = (OpenNaFile*)(openFiles->Fetch(fId-2));
	openFiles->Remove(fId-2);
	return(temp);
}

//------------------------------------------------------------------
// AddrSpace::GetOpenFile
//     if a file with the specified identifier is on the list then
//      return it, else return a null pointer
//     This is like AddrSpace::DeleteOpenFile, except the open file
//     is not deleted.
//
//     "fId" is the identifier of the desired file
//------------------------------------------------------------------
//OpenNaFile* AddrSpace::GetOpenFile(int fId){	/* Liang DePeng - */
OpenNaFile* MyAddrSpace::GetOpenFile(int fId){	/* Liang DePeng + */
	if ((fId < 2) || (fId >= MaxOpenFiles+2)){
		return((OpenNaFile *)0);
	}
	return((OpenNaFile*)(openFiles->Fetch(fId-2)));
}

//----------------------------------------------------------------------
// AddrSpace::WriteMem
//      Write "value", which is "size" bytes, to virtual address "addr"
//  Returns a TranslationStatus to indicate whether the required
//    virtual to physical translation was successful.
//  Writes data to addr only if the return value is "OK"
//----------------------------------------------------------------------
//TranslationStatus AddrSpace::WriteMem(int addr, int size, int value){	/* Liang DePeng - */
TranslationStatus MyAddrSpace::WriteMem(int addr, int size, int value){	/* Liang DePeng + */

	int physicalAddress;
	TranslationStatus status;

	ASSERT(addr >= 0);
	status = Translate(addr, &physicalAddress,TRUE);
	if (status != OK){
		return(status);
	}
	switch (size){
	case 1:
		kernel->mainMemory[physicalAddress] =
			(unsigned char)(value & 0xff);
		break;

	case 2:
		if (addr & 0x1){
			// alignment error
			return(AddressError);
		}
		*(unsigned short *) &(kernel->mainMemory[physicalAddress]) = 
			ShortToMachine((unsigned short)(value & 0xffff));
		break;

	case 4:
		if (addr & 0x3){
			// alignment error
			return(AddressError);
		}
		*(unsigned int *) &(kernel->mainMemory[physicalAddress]) = 
			WordToMachine((unsigned int)value);
		break;

	default: ASSERT(FALSE);
	}
	return(OK);
}




